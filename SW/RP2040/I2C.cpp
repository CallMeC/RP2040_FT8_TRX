#include "I2C.h"

c_I2C::c_I2C()
{
}

int c_I2C::i2c_get_data(uint8_t addr, uint8_t *dst, size_t len, bool nostop)
{
    int ret = i2c_read_timeout_us(I2C_PORT, addr, dst, len, nostop, I2C_TIMEOUT);
    if (ret < 0) {
        return ret;
    }
    sleep_us(I2C_LINGER_US);
    return ret;
}

int c_I2C::i2c_put_data(uint8_t addr, const uint8_t *src, size_t len, bool nostop)
{
    int ret = i2c_write_timeout_us(I2C_PORT, addr, src, len, nostop, I2C_TIMEOUT);
    if (ret < 0) {
        return ret;
    }
    sleep_us(I2C_LINGER_US);
    return ret;
}


void c_I2C::Init() //Init of the I2C peripheral
{
    i2c_init(I2C_PORT, I2C_FREQUENCY);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void c_I2C::Scan()
{
    printf("I2C SCAN\n");
    for (uint8_t i = 1; i < 255; i++)
    {
        int32_t ret = i2c_write_timeout_us(I2C_PORT, i, 0x00, 1, true, I2C_TIMEOUT);
        if (ret > 0)
            printf("%02X\n", i);
        else
            printf("-");
    }
}