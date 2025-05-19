#include "Si5351.h"
#include <stdio.h>
#include <math.h>

// Utilise tes fonctions I2C existantes
extern int32_t sys_i2c_rbuf_reg(i2c_inst_t*, uint8_t, uint8_t, uint8_t*, uint32_t);
extern int32_t sys_i2c_wbuf_reg(i2c_inst_t*, uint8_t, uint8_t, uint8_t*, uint32_t);

SI5351::SI5351(i2c_inst_t* i2c, uint8_t addr) : _i2c(i2c), _addr(addr)
{
    I2C.Init(); //0x60 & 0xE0 found
}

bool SI5351::init()
{
    // Désactiver toutes les sorties
    if (!writeRegister(3, 0xFF)) return false;

    // Configurer la capacité de charge à 10 pF (bits [1:0] = 0b10)
    uint8_t val;
    if (!readRegister(183, val)) return false;
    val = (val & 0b11111100) | 0b10; // Conserve les autres bits, force 10pF
    if (!writeRegister(183, val)) return false;

    return true;
}


bool SI5351::writeRegister(uint8_t reg, uint8_t value)
{
    return I2C.sys_i2c_wbuf_reg(_i2c, _addr, reg, &value, 1) >= 0;
}

bool SI5351::writeRegisters(uint8_t reg, uint8_t* data, uint8_t len)
{
    return I2C.sys_i2c_wbuf_reg(_i2c, _addr, reg, data, len) >= 0;
}

bool SI5351::readRegister(uint8_t reg, uint8_t& value)
{
    return I2C.sys_i2c_rbuf_reg(_i2c, _addr, reg, &value, 1) >= 0;
}

bool SI5351::enableOutput(uint8_t clk)
{
    uint8_t val;
    if (!readRegister(3, val)) return false;
    val &= ~(1 << clk); // Active la sortie clk
    return writeRegister(3, val);
}

bool SI5351::enableAllOutputs()
{
    // Registre 3 : un bit à 1 désactive une sortie, donc 0x00 les active toutes
    return writeRegister(3, 0x00);
}

bool SI5351::disableOutput(uint8_t clk)
{
    uint8_t val;
    if (!readRegister(3, val)) return false;
    val |= (1 << clk); // Désactive la sortie clk
    return writeRegister(3, val);
}

bool SI5351::setFrequency(uint8_t clk, uint32_t freq_hz)
{
    // Appelle des fonctions simplifiées ici pour la PLL et MS
    configurePLL();
    configureMultisynth(clk, freq_hz);
    enableOutput(clk);
    return true;
}

void SI5351::configurePLL()
{
    // Configure PLLA à 900 MHz : Mult = 36, num = 0, denom = 1 (25 MHz x 36)
    uint8_t pll_regs[8];
    uint32_t mult = 36;
    uint32_t num = 0, denom = 1;

    uint32_t P1 = 128 * mult + (128 * num) / denom - 512;
    uint32_t P2 = (128 * num) % denom;
    uint32_t P3 = denom;

    pll_regs[0] = (P3 >> 8) & 0xFF;
    pll_regs[1] = P3 & 0xFF;
    pll_regs[2] = (P1 >> 16) & 0x03;
    pll_regs[3] = (P1 >> 8) & 0xFF;
    pll_regs[4] = P1 & 0xFF;
    pll_regs[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F);
    pll_regs[6] = (P2 >> 8) & 0xFF;
    pll_regs[7] = P2 & 0xFF;

    writeRegisters(26, pll_regs, 8); // PLLA = base 0x26
}

void SI5351::configurePLL(uint64_t pll_freq) {
    // PLL = XTAL * (a + b/c)
    // On utilise un multiple entier => b = 0, c = 1
    const uint32_t xtal = 25000000; // 25 MHz typique
    uint32_t mult = pll_freq / xtal;
    uint32_t P1 = 128 * mult - 512;
    uint32_t P2 = 0;
    uint32_t P3 = 1;

    uint8_t pll_regs[8];
    pll_regs[0] = (P3 >> 8) & 0xFF;
    pll_regs[1] = P3 & 0xFF;
    pll_regs[2] = (P1 >> 16) & 0x03;
    pll_regs[3] = (P1 >> 8) & 0xFF;
    pll_regs[4] = P1 & 0xFF;
    pll_regs[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F);
    pll_regs[6] = (P2 >> 8) & 0xFF;
    pll_regs[7] = P2 & 0xFF;

    writeRegisters(26, pll_regs, 8); // PLLA = Reg 26-33
}


void SI5351::configureMultisynth(uint8_t clk, uint32_t freq_hz)
{
    // Utilise PLLA = 900 MHz, simplifié : MSx = PLL / Freq
    uint32_t pll_freq = 900000000;
    uint32_t div = pll_freq / freq_hz;

    if (div < 8) div = 8; // Min selon datasheet
    if (div > 900) div = 900; // Max selon datasheet

    uint32_t P1 = 128 * div - 512;
    uint32_t P2 = 0;
    uint32_t P3 = 1;

    uint8_t ms_regs[8];
    ms_regs[0] = (P3 >> 8) & 0xFF;
    ms_regs[1] = P3 & 0xFF;
    ms_regs[2] = (P1 >> 16) & 0x03;
    ms_regs[3] = (P1 >> 8) & 0xFF;
    ms_regs[4] = P1 & 0xFF;
    ms_regs[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F);
    ms_regs[6] = (P2 >> 8) & 0xFF;
    ms_regs[7] = P2 & 0xFF;

    writeRegisters(42 + 8 * clk, ms_regs, 8); // MS for clk0 = 0x42, clk1 = 0x4A, ...
    writeRegister(16 + clk, 0x4F); // Route CLKx from PLLA
}

bool SI5351::ping()
{
    uint8_t val = 0;
    return readRegister(0, val); // Le registre 0 contient le Device Status Register
}

bool SI5351::setFrequencyInteger(uint8_t clk, uint32_t freq_hz)
{
    configurePLL(); // Configure PLL à 900 MHz

    uint32_t pll_freq = 900000000;
    uint32_t div = pll_freq / freq_hz;
    if (div < 8) div = 8;
    if (div > 900) div = 900;

    uint32_t actual_freq = pll_freq / div;
    printf("Diviseur entier : %lu -> Freq réelle : %lu Hz\n", div, actual_freq);

    // P1, P2, P3 pour diviseur entier
    uint32_t P1 = 128 * div - 512;
    uint32_t P2 = 0;
    uint32_t P3 = 1;

    uint8_t ms_regs[8];
    ms_regs[0] = (P3 >> 8) & 0xFF;
    ms_regs[1] = P3 & 0xFF;
    ms_regs[2] = (P1 >> 16) & 0x03;
    ms_regs[3] = (P1 >> 8) & 0xFF;
    ms_regs[4] = P1 & 0xFF;
    ms_regs[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F);
    ms_regs[6] = (P2 >> 8) & 0xFF;
    ms_regs[7] = P2 & 0xFF;

    writeRegisters(42 + 8 * clk, ms_regs, 8);       // Multisynth
    writeRegister(16 + clk, 0x4F);                  // Source = PLLA, 8 mA
    enableOutput(clk);

    return true;
}

bool SI5351::setFrequencyInteger(uint8_t clk, uint32_t freq_hz, uint32_t* actual_freq)
{
    // Etape 1: Choisir un diviseur raisonnable au départ
    uint32_t div = 64; // point de départ correct
    uint64_t pll_freq = (uint64_t)freq_hz * div;

    // Etape 2: S'assurer que le PLL reste dans les limites
    if (pll_freq < 600000000ULL) pll_freq = 600000000ULL;
    if (pll_freq > 900000000ULL) pll_freq = 900000000ULL;

    // Etape 3: Recalculer le diviseur avec le PLL ajusté
    div = (uint32_t)((pll_freq + freq_hz / 2) / freq_hz); // arrondi
    if (div < 8) div = 8;
    if (div > 900) div = 900;

    pll_freq = (uint64_t)freq_hz * div;

    uint32_t freq_out = pll_freq / div;
    if (actual_freq) *actual_freq = freq_out;

    // Etape 4: Configurer la PLL avec cette fréquence
    configurePLL(pll_freq); // à adapter (voir plus bas)

    // Etape 5: Préparer les registres multisynth (diviseur entier → P2 = 0, P3 = 1)
    uint32_t P1 = 128 * div - 512;
    uint32_t P2 = 0;
    uint32_t P3 = 1;

    uint8_t ms_regs[8];
    ms_regs[0] = (P3 >> 8) & 0xFF;
    ms_regs[1] = P3 & 0xFF;
    ms_regs[2] = (P1 >> 16) & 0x03;
    ms_regs[3] = (P1 >> 8) & 0xFF;
    ms_regs[4] = P1 & 0xFF;
    ms_regs[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F);
    ms_regs[6] = (P2 >> 8) & 0xFF;
    ms_regs[7] = P2 & 0xFF;

    writeRegisters(42 + 8 * clk, ms_regs, 8);
    writeRegister(16 + clk, 0x4F); // PLLA, 8 mA
    enableOutput(clk);

    return true;
}

void SI5351::setPhaseOffset(uint8_t clk, uint8_t offset)
{
    if (clk > 2 || offset > 127) return;
    writeRegister(165 + clk, offset);
}

void SI5351::printStatus()
{
    uint8_t status = 0;
    if (!readRegister(0, status))
    {
        printf("Failed to read status register.\n");
        return;
    }

    printf("Status Register (0x00): 0x%02X\n", status);

    if (status & (1 << 7))
    {
        printf("SYS_INIT: PLLs initializing or reset.\n");
    }
    else
    {
        printf("SYS_INIT: PLLs initialized.\n");
    }

    if (status & (1 << 6))
    {
        printf("LOL_A: PLLA lost lock!\n");
    }
    else
    {
        printf("LOL_A: PLLA locked.\n");
    }

    if (status & (1 << 5))
    {
        printf("LOL_B: PLLB lost lock!\n");
    }
    else
    {
        printf("LOL_B: PLLB locked.\n");
    }

    if (status & (1 << 4))
    {
        printf("LOS: XTAL not detected (loss of signal)!\n");
    }
    else
    {
        printf("LOS: Crystal oscillator detected.\n");
    }
}