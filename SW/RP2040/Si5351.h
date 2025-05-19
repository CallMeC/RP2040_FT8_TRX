#ifndef SI5351_H
#define SI5351_H

#include "I2C.h"
#include <stdint.h>

#define SI5151_ADDR 0x60

class SI5351
{
public:
    SI5351(i2c_inst_t* i2c = I2C_PORT, uint8_t addr = SI5151_ADDR);

    bool init();
    bool enableOutput(uint8_t clk);
    bool enableAllOutputs();
    bool disableOutput(uint8_t clk);
    bool setFrequency(uint8_t clk, uint32_t freq_hz);
    bool ping();
    bool setFrequencyInteger(uint8_t clk, uint32_t freq_hz);
    bool setFrequencyInteger(uint8_t clk, uint32_t freq_hz, uint32_t* actual_freq = nullptr);
    void setPhaseOffset(uint8_t clk, uint8_t offset);
    void printStatus();

private:
    c_I2C I2C;
    i2c_inst_t* _i2c;
    uint8_t _addr;

    bool writeRegister(uint8_t reg, uint8_t value);
    bool writeRegisters(uint8_t reg, uint8_t* data, uint8_t len);
    bool readRegister(uint8_t reg, uint8_t& value);

    void configurePLL();
    void configurePLL(uint64_t pll_freq);
    void configureMultisynth(uint8_t clk, uint32_t freq_hz);
};

#endif // SI5351_H
