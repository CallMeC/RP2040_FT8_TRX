#ifndef USB_CONSOLE_H
#define USB_CONSOLE_H

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_FREQ 1000000

#define TASK_TX_ON      1
#define TASK_TX_OFF     2
#define TASK_RX_ON      3
#define TASK_RX_OFF     4
#define TASK_SETF       5
#define TASK_RESETF     6
#define TASK_NOOP       0

class USBConsole
{
    public:
        USBConsole();
        uint8_t poll(); // à appeler périodiquement depuis main()
        uint32_t getFrequency();
    
        private:
            uint32_t current_freq;
};

#endif // USB_CONSOLE_H
