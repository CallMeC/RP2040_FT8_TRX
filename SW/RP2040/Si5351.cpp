#include "Si5351.h"

c_I2C I2C0;

// ---------------- I2C Helpers ----------------
void c_Si5351::write_register(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
	I2C0.i2c_put_data(SI_ADDRESS, buf, 2, false);
}

void c_Si5351::write_registers(uint8_t start_reg, uint8_t *data, size_t len)
{
    uint8_t buf[len + 1];
    buf[0] = start_reg;
    memcpy(&buf[1], data, len);
    I2C0.i2c_put_data(SI_ADDRESS, buf, len + 1, false);
}

void c_Si5351::read_register(uint8_t reg, uint8_t *value)
{
    // Write register address (no stop), then read 1 byte
    I2C0.i2c_put_data(SI_ADDRESS, &reg, 1, true);
    I2C0.i2c_get_data(SI_ADDRESS, value, 1, false);
}

// ---------------- PLL Parameter Calculator ----------------
void c_Si5351::calc_params(double ratio, uint32_t *P1, uint32_t *P2, uint32_t *P3)
{
    uint32_t A = (uint32_t)floor(ratio);
    uint32_t B = (uint32_t)((ratio - A) * SI_PLL_C);
    *P3 = SI_PLL_C;
    uint32_t temp = (128 * B) / SI_PLL_C;
    *P1 = 128 * A + temp - 512;
    *P2 = 128 * B - SI_PLL_C * temp;
}

// ---------------- SI5351 Control ----------------
void c_Si5351::reset_pll(uint8_t pll) {
    write_register(SI_PLL_RESET_REG, (pll == 0) ? 0xA0 : 0x60);
}

void c_Si5351::set_output_enable(uint8_t clk, uint8_t enable) {
    uint8_t reg;
    read_register(SI_CLK_ENABLE_REG, &reg);
    if (enable) reg &= ~(1 << clk);
    else reg |= (1 << clk);
    write_register(SI_CLK_ENABLE_REG, reg);
}

// ---------------- MS Configuration ----------------
void c_Si5351::set_pll(uint8_t pll, double ratio)
{
    uint8_t base_reg = (pll == 0) ? SI_MSNA_REG : (SI_MSNA_REG + 8);
    uint32_t P1, P2, P3;
    calc_params(ratio, &P1, &P2, &P3);

    uint8_t data[8] = {
        (P3 >> 8) & 0xFF,
        P3 & 0xFF,
        (P1 >> 16) & 0x03,
        (P1 >> 8) & 0xFF,
        P1 & 0xFF,
        ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F),
        (P2 >> 8) & 0xFF,
        P2 & 0xFF
    };
    write_registers(base_reg, data, 8);
}

void c_Si5351::set_multisynth(uint8_t clk, double ratio, uint8_t Rdiv)
{
    uint32_t P1, P2, P3;
    calc_params(ratio, &P1, &P2, &P3);

    uint8_t base = 42 + 8 * clk;
    uint8_t data[8] = {
        (P3 >> 8) & 0xFF,
        P3 & 0xFF,
        (P1 >> 16) & 0x03,
        (P1 >> 8) & 0xFF,
        P1 & 0xFF,
        ((Rdiv & 0x07) << 4) | ((P3 >> 12) & 0x0F) | ((P2 >> 16) & 0x0F),
        (P2 >> 8) & 0xFF,
        P2 & 0xFF
    };
    write_registers(base, data, 8);

    write_register(SI_CLK0_CTRL + clk, 0x4F | ((clk == 0) ? 0x00 : 0x20));
}

void c_Si5351::set_phase_offset(uint8_t clk, uint8_t offset_steps)
{
    if (clk > 2 || offset_steps > 7) return; // offset max 7 (3 bits)
    uint8_t base_reg = 165 + clk * 8; // 165, 173, 181 pour CLK0,1,2

    uint8_t reg_val = 0;
    read_register(base_reg, &reg_val);

    reg_val = (reg_val & 0xF8) | (offset_steps & 0x07);
    write_register(base_reg, reg_val);
}


// ---------------- Initialization ----------------
void c_Si5351::init(void)
{
	frequency = 14105000; //Default Frequency
	I2C0.Init();
    // Désactiver toutes les sorties
    write_register(SI_CLK_ENABLE_REG, 0xFF);

    // Éteindre tous les clocks
    for (uint8_t i = 0; i < 8; i++) {
        write_register(SI_CLK0_CTRL + i, 0x80); // Power down
    }

    // Définir PLLA à 620.664 MHz
    double pll_ratio = 24.0 + 2583.0 / 3125.0;
    set_pll(0, pll_ratio); // PLLA = 0

    // Configurer CLK0, CLK1, CLK2 à 14.106 MHz
    for (uint8_t clk = 0; clk <= 2; clk++) {
        set_multisynth(clk, 44.0, 0);  // MS = 44, R = 1
        set_output_enable(clk, 1);
    }

    // Configurer les sorties
    write_register(SI_CLK0_CTRL, 0b00011111); // Inversé
    write_register(SI_CLK1_CTRL, 0b00001111); // Normal
    write_register(SI_CLK2_CTRL, 0b00011111); // Inversé

    // Reset PLL pour appliquer les changements
    reset_pll(0);
    sleep_ms(1); // <--- Important pour la stabilité
}


void c_Si5351::set_frequency_all(uint32_t freq_hz)
{
    // Désactiver les sorties pendant la config
	write_register(SI_CLK_ENABLE_REG, 0xFF);
    // Calcul PLL
    double f_out = (double)freq_hz;
    double pll_freq = f_out * 44.0;
    double pll_ratio = pll_freq / (double)SI_XTAL_FREQ;
    // Config PLLA
    set_pll(0, pll_ratio);
    // Config MS (Multisynths)
    for (uint8_t clk = 0; clk <= 2; clk++) {
        set_multisynth(clk, 44.0, 0); // R = 0 (pas de division)
    }
    // CTRL: CLK0 et CLK1 normaux, CLK2 inversé
    write_register(SI_CLK0_CTRL, 0b00001111);  // Normal
    write_register(SI_CLK1_CTRL, 0b00001111);  // Normal
    write_register(SI_CLK2_CTRL, 0b00011111);  // Inversé
    // Activer les sorties
    write_register(SI_CLK_ENABLE_REG, 0b11111000); // 0-2 ON, 3-7 OFF
    // Reset PLLA pour appliquer
    reset_pll(0);
}

bool c_Si5351::check_connection()
{
    uint8_t reg = SI_CLK0_CTRL;
    uint8_t value = 0x00;

    // Écrire l'adresse du registre à lire
    int ret = I2C0.i2c_put_data(SI_ADDRESS, &reg, 1, true);
    if (ret < 0)
	{
        return false;
    }
    // Lire le contenu du registre
    ret = I2C0.i2c_get_data(SI_ADDRESS, &value, 1, false);
    if (ret < 0)
	{
        return false;
    }

    // Vérification simple : on évite de considérer 0xFF (souvent "pull-up") ou 0x00 (souvent "open-drain") comme valides
    if (value == 0xFF || value == 0x00)
	{
        return false;
    }
	printf("Device OK\n");
    return true;
}

void c_Si5351::pool()
{
	switch(TASK)
	{
		case 1:
			check_connection();
			TASK = 0;
			break;
		case 2:
			set_frequency_all(frequency);
			TASK = 0;
			break;
		
		case 3:
			Alouette();
			TASK = 0;

		case 0:
			break;

		default:
			break;
	}

}

void c_Si5351::Alouette()
{
    write_register(SI_CLK_ENABLE_REG, 0xFF);

    double pll_ratio = 24.768;
    set_pll(0, pll_ratio); // PLLA = 0

    // Configurer CLK0, CLK1, CLK2 à 14.106 MHz
    for (uint8_t clk = 0; clk <= 2; clk++) {
        set_multisynth(clk, 6.0, 0);  // MS = 44, R = 1
        set_output_enable(clk, 1);
    }

    // Configurer les sorties
    write_register(SI_CLK0_CTRL, 0b00001111); // Normal
    write_register(SI_CLK1_CTRL, 0b00001111); // Normal
    write_register(SI_CLK2_CTRL, 0b00001111); // Normal
	set_phase_offset(2, 2);

    // Reset PLL pour appliquer les changements
    reset_pll(0);
}
