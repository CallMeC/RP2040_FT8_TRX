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

#define TASK_TX_ON      1
#define TASK_TX_OFF     2
#define TASK_RX_ON      3
#define TASK_RX_OFF     4
#define TASK_SETF       5
#define TASK_RESETF     6
#define TASK_STATUS     7
#define TASK_ALL        8
#define TASK_NOOP       0

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

        // Fréquence (Hz) pour tous les canaux
        void setFrequency(uint32_t freq);
        uint32_t getFrequency() const;
        void set_frequency(uint8_t output, uint32_t freq_hz);
        void set_phase_offset(uint8_t output, uint8_t offset);

        // Contrôle des sorties
        void enableTX();     // CLK0
        void disableTX();
        void enableRX();     // CLK1 & CLK2 en quadrature
        void disableRX();

        uint8_t USB_CONSOLE_TASK;

        void enableVFO(int vfoIndex, bool enable);
        void setFrequency(int vfoIndex, uint32_t frequency);
        void setPhase(int vfoIndex, uint8_t phase);
        vfo_t getVFO(int vfoIndex);

    private:
        static c_IO_Manager* s_instance;

        //SI5351 si5351;
        vfo_t vfo[2];  // VFO0 et VFO1

        void GPIO_Mainloop();
        void GPIO_Bootfinished();
        bool checkVCO();
        
        uint8_t timer_counter;
        uint8_t USR_SW_3_Counter;
        uint8_t USR_SW_4_Counter;
        uint8_t USR_SW_5_Counter;
        
        uint32_t actual_freq;
        uint32_t currentFreq;
};
#endif