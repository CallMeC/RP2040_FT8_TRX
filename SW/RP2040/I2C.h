#ifndef __I2C__H
#define __I2C__H

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "IO_Definition.h"

//Param I2C Devices
#define I2C_PORT                i2c0
#define I2C_TIMEOUT             1000      //Timeout (µs)
#define I2C_FREQUENCY           400000     //I2C à 100kHz
#define I2C_LINGER_US           10         //Linger time (µs)

class c_I2C
{
    public:
        c_I2C();
        int i2c_put_data(uint8_t addr, const uint8_t *src, size_t len, bool nostop);
        int i2c_get_data(uint8_t addr, uint8_t *dst, size_t len, bool nostop);
        void Init();
        void Scan();
};



#endif