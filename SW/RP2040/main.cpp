#include "main.h"


int main()
{

    c_IO_Manager IOManager;
    c_IO_Manager::SetInstance(&IOManager);


    // Initialize chosen serial port
    stdio_init_all();
    IOManager.GPIO_Init();
    

    // Loop forever
    while (true)
    {
        IOManager.IO_OPERATION = GPIO_GET;
        sleep_ms(3000);
    }
}

