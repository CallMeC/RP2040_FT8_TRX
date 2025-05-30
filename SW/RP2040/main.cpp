#include "main.h"


int main()
{
    static c_IO_Manager IOManager;
    c_IO_Manager::SetInstance(&IOManager);
    USBConsole console;


    // Initialize chosen serial port
    stdio_init_all();

    IOManager.GPIO_Init();

    // Loop forever
    while (true)
    {
        IOManager.IO_OPERATION = UPDATE_CLK;
        Si5351.pool();
        Si5351.frequency = IOManager.vfo_frequency;
        //IOManager.USB_CONSOLE_TASK = console.poll();
        sleep_ms(10);
    }
}
