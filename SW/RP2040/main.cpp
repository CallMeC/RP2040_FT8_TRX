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
        sleep_ms(300);
    }
}

