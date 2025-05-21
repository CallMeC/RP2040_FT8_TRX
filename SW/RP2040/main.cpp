#include "main.h"


int main()
{

    c_IO_Manager IOManager;
    c_IO_Manager::SetInstance(&IOManager);
    USBConsole console;


    // Initialize chosen serial port
    stdio_init_all();
    IOManager.GPIO_Init();
    

    // Loop forever
    while (true)
    {
        IOManager.IO_OPERATION = UPDATE_CLK;
        IOManager.USB_CONSOLE_TASK = console.poll();
        //IOManager.setFrequency(0, console.getFrequency());
        sleep_ms(10);
    }
}
