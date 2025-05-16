#include "main.h"


int main()
{

    c_IO_Manager IOManager;
    


    // Initialize chosen serial port
    stdio_init_all();
    IOManager.GPIO_Init();

    // Loop forever
    while (true)
    {
        printf("Helloooo!\r\n");
        IOManager.IO_OPERATION = UPDATE_CLK;
        sleep_ms(3000);
    }
}

