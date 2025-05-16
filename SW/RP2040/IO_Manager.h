#ifndef __IO_MANAGER__H
#define __IO_MANAGER__H

#include "pico/stdlib.h"
#include <stdio.h>
#include "Si5351.h"

#define DEFAULT_NOP     0
#define CHECK_RTC       5
#define UPDATE_CLK      6

class c_IO_Manager
{
    public:
        void GPIO_Init();
        void GPIO_LED_TOGGLE();
        void GPIO_Callback();
        uint8_t LED_STATE;
        uint8_t IO_OPERATION;
        

    private:
        SI5351 si5351;
        void GPIO_Mainloop();
        void GPIO_Bootfinished();
};
#endif