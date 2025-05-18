#ifndef __IO_MANAGER__H
#define __IO_MANAGER__H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

#include "Si5351.h"

#define DEFAULT_NOP     0
#define CHECK_RTC       5
#define UPDATE_CLK      6
#define GPIO_GET        7

class c_IO_Manager
{
    public:
        void GPIO_Init();
        void GPIO_LED_TOGGLE();
        void GPIO_Callback();
        uint8_t LED_STATE;
        uint8_t IO_OPERATION;
        
        void USR_SW_3_CallBack();
        void USR_SW_4_CallBack();
        void USR_SW_5_CallBack();

        static void SetInstance(c_IO_Manager* inst);
        static c_IO_Manager* GetInstance();

    private:
        static c_IO_Manager* s_instance;

        SI5351 si5351;

        void GPIO_Mainloop();
        void GPIO_Bootfinished();
        bool checkVCO();
        
        uint8_t timer_counter;
        uint8_t USR_SW_3_Counter;
        uint8_t USR_SW_4_Counter;
        uint8_t USR_SW_5_Counter;
        
        uint32_t actual_freq;
};
#endif