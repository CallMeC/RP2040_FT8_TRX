#include "IO_Manager.h"
#include "IO_Definition.h"

bool timer_1000ms_callback(repeating_timer_t *rt)
{
    c_IO_Manager* instance = static_cast<c_IO_Manager*>(rt->user_data);
    if (instance)
        instance->GPIO_Callback();
    return true;
}

void c_IO_Manager::GPIO_Init()
{
    gpio_init(OUTPUT_LED);
    gpio_init(ANTENNA_RELAY);

    gpio_set_dir(OUTPUT_LED, GPIO_OUT);
    gpio_set_dir(ANTENNA_RELAY, GPIO_OUT);

    gpio_put(OUTPUT_LED, false);
    gpio_put(ANTENNA_RELAY, false);

    
    si5351.init();
    //si5351.setFrequency(0, 10000000); // 10 MHz sur CLK0


    GPIO_Bootfinished();

    IO_OPERATION = DEFAULT_NOP;
}

void c_IO_Manager::GPIO_Mainloop()
{
    switch(IO_OPERATION)
    {
        case DEFAULT_NOP:
            break;
        
        case CHECK_RTC:
            IO_OPERATION = DEFAULT_NOP;
            break;

        case UPDATE_CLK:
            if (si5351.ping())
            {
                printf("SI5351 Detected\n");
                gpio_put(ANTENNA_RELAY, true);
                si5351.setFrequencyInteger(0, 14000000); // CLK0 à ~14.06 MHz, carré 50%
                gpio_put(ANTENNA_RELAY, false);
            }
            else
                printf("Error : SI5351 Timeout\n");

            IO_OPERATION = DEFAULT_NOP;
            break;
        
        default:
            IO_OPERATION = DEFAULT_NOP;
            break;
    }
    return;
}

void c_IO_Manager::GPIO_Callback()
{
    GPIO_Mainloop();
    GPIO_LED_TOGGLE();
    return;
}

void c_IO_Manager::GPIO_Bootfinished()
{
    static repeating_timer_t timer_1000ms;
    add_repeating_timer_ms(500, &timer_1000ms_callback, this, &timer_1000ms);
    return;
}

void c_IO_Manager::GPIO_LED_TOGGLE()
{   
    if (LED_STATE == 0) LED_STATE = 1;
    else LED_STATE = 0;
    gpio_put(OUTPUT_LED, LED_STATE);
    return;
}