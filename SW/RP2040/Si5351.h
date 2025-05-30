#ifndef SI5351_H
#define SI5351_H

#include <stdint.h>
#include "I2C.h"

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

// ----------- Configuration Constants -----------
#define SI_ADDRESS      0x60
#define SI_XTAL_FREQ    25000000UL
#define SI_PLL_VCO_MIN  600000000UL
#define SI_PLL_VCO_MAX  900000000UL
#define SI_MS_DIV_MIN   8
#define SI_MS_DIV_MAX   2048
#define SI_PLL_C        1048575UL  // 2^20 - 1

// ----------- Register Addresses -----------
#define SI_CLK_ENABLE_REG  3
#define SI_PLL_RESET_REG   177
#define SI_MSNA_REG        26
#define SI_CLK0_CTRL       16
#define SI_CLK1_CTRL       17
#define SI_CLK2_CTRL       18
#define SI_CLK3_CTRL       19
#define SI_CLK4_CTRL       20
#define SI_CLK5_CTRL       21
#define SI_CLK6_CTRL       22
#define SI_CLK7_CTRL       23

// ----------- Public Functions -----------

extern c_I2C I2C0;

class c_Si5351
{
    public:
        void init(void);
        void pool();
        uint32_t frequency;

        uint8_t TASK;

    private:
        void write_register(uint8_t reg, uint8_t value);
        void write_registers(uint8_t start_reg, uint8_t *data, size_t len);
        void read_register(uint8_t reg, uint8_t *value);
        void calc_params(double ratio, uint32_t *P1, uint32_t *P2, uint32_t *P3);
        void set_pll(uint8_t pll, double ratio);
        void set_multisynth(uint8_t clk, double ratio, uint8_t Rdiv);
        void set_phase_offset(uint8_t clk, uint8_t offset_steps);
        void set_frequency_all(uint32_t freq_hz);
        void reset_pll(uint8_t pll);
        void set_output_enable(uint8_t clk, uint8_t enable);
        bool check_connection();
        void Alouette();

};


#endif // SI5351_H
