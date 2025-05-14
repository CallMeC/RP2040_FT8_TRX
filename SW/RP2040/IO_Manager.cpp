#include "IO_Manager.h"
#include "IO_Definition.h"

bool timer_1000ms_callback(repeating_timer_t *rt)
{
    c_IO_Manager* instance = static_cast<c_IO_Manager*>(rt->user_data);
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

    IO_OPERATION = DEFAULT_NOP;
}

void c_IO_Manager::GPIO_Mainloop()
{
    switch(IO_OPERATION)
    {
        case DEFAULT_NOP:
            ;
            break;
        
        case CHECK_RTC:
            ;
            break;

        case UPDATE_CLK:
            ;
            break;
        
        default:
            ;
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
    add_repeating_timer_ms(500, &timer_1000ms_callback, NULL, &timer_1000ms);
    return;
}

void c_IO_Manager::GPIO_LED_TOGGLE()
{
    if (LED_MICRO_STATE == 0)
        LED_MICRO_STATE = 1;
    else
        LED_MICRO_STATE = 0;

    gpio_put(OUTPUT_LED, LED_MICRO_STATE);

    return;
}