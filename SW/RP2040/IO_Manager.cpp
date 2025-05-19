#include "IO_Manager.h"
#include "IO_Definition.h"

bool timer_1000ms_callback(repeating_timer_t *rt)
{
    c_IO_Manager* instance = static_cast<c_IO_Manager*>(rt->user_data);
    if (instance)
        instance->GPIO_Callback();
    return true;
}

void gpio_callback(uint gpio, uint32_t events)
{
    c_IO_Manager* instance = c_IO_Manager::GetInstance();

    switch(gpio)
    {
        case USER_SW_3:
            if (instance)
                instance->USR_SW_3_CallBack();
            break;

        case USER_SW_4:
            if (instance)
                instance->USR_SW_4_CallBack();
            break;

        case USER_SW_5:
            if (instance)
                instance->USR_SW_5_CallBack();
            break;
    }
}

//--------------------------------------------

c_IO_Manager* c_IO_Manager::s_instance = nullptr;

void c_IO_Manager::SetInstance(c_IO_Manager* inst) {
    s_instance = inst;
}

c_IO_Manager* c_IO_Manager::GetInstance() {
    return s_instance;
}

void c_IO_Manager::GPIO_Init()
{
    timer_counter = 0; USB_CONSOLE_TASK = 0;
    USR_SW_3_Counter = 0; USR_SW_4_Counter = 0; USR_SW_5_Counter = 0;

    gpio_init(OUTPUT_LED);
    gpio_init(ANTENNA_RELAY);

    gpio_set_dir(OUTPUT_LED, GPIO_OUT);
    gpio_set_dir(ANTENNA_RELAY, GPIO_OUT);

    gpio_put(OUTPUT_LED, false);
    gpio_put(ANTENNA_RELAY, false);

    gpio_set_irq_enabled_with_callback(USER_SW_3, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(USER_SW_4, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(USER_SW_5, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    si5351.init();

    GPIO_Bootfinished();

    IO_OPERATION = DEFAULT_NOP;
}

void c_IO_Manager::GPIO_Mainloop()
{
    if (timer_counter == 10)
        checkVCO();

    switch(IO_OPERATION)
    {
        case DEFAULT_NOP:
            break;
        
        case CHECK_RTC:
            IO_OPERATION = DEFAULT_NOP;
            break;

        case UPDATE_CLK:
            switch(USB_CONSOLE_TASK)
            {
                case TASK_TX_ON:
                    enableTX();

                case TASK_TX_OFF:
                    disableTX();
                    
                case TASK_RX_ON:
                    enableRX();

                case TASK_RX_OFF:
                    disableRX();

                case TASK_SETF:
                    setFrequency(currentFreq);

                case TASK_RESETF:
                    setFrequency(1000000);

                case TASK_STATUS:
                    si5351.printStatus();

                case TASK_ALL:
                    si5351.enableAllOutputs();

                default:
                    ;
            }
            IO_OPERATION = DEFAULT_NOP;
            break;
        
        case GPIO_GET:
            ;
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
    timer_counter++;
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
    if (LED_STATE == 0)
        LED_STATE = 1;
    else
        LED_STATE = 0;

    gpio_put(OUTPUT_LED, LED_STATE);
    return;
}

bool c_IO_Manager::checkVCO()
{
    if (si5351.ping())
    {
        //printf("SI5351 Detected\n");
        //gpio_put(ANTENNA_RELAY, true);
        //si5351.setFrequencyInteger(0, 14000000); // CLK0 à ~14.06 MHz, carré 50%
        //gpio_put(ANTENNA_RELAY, false);
        return true;
    }
    else
    {
        printf("Error : SI5351 Timeout\n");
        return false;
    }
}

void c_IO_Manager::USR_SW_3_CallBack()
{
    printf("USR SW 3\n");
    uint32_t actual;

    if (USR_SW_3_Counter == 0)
    {
        uint32_t freq = 7031250; // ~7.03 MHz (PLL = 900 MHz, div = 128)

        si5351.setFrequencyInteger(1, freq, &actual);
        si5351.setFrequencyInteger(2, freq, &actual);

        // Phase en quadrature : 90° → 1/4 période → offset = div / 4
        si5351.setPhaseOffset(2, 32); // CLK2 en quadrature par rapport à CLK1

        printf("CLK1 & CLK2 @ %lu Hz, quadrature OK\n", actual);
        USR_SW_3_Counter = 1;
    }
    else
    {
        si5351.disableOutput(1);
        si5351.disableOutput(2);
        actual_freq = 0;
        USR_SW_3_Counter = 0;
        printf("CLK1 & CLK2 @ 0 Hz\n", actual);
    }
}

void c_IO_Manager::USR_SW_4_CallBack()
{
    printf("USR SW 4\n");

    uint32_t actual_freq;
    if (USR_SW_4_Counter == 0)
    {
        si5351.setFrequencyInteger(0, 1400, &actual_freq); // CLK0 à ~1.406 kHz, carré 50%
        USR_SW_4_Counter = 1;
    }
    else if (USR_SW_4_Counter == 1)
    {
        si5351.setFrequencyInteger(0, 14000, &actual_freq);
        USR_SW_4_Counter = 2;
        
    }
    else if (USR_SW_4_Counter == 2)
    {
        si5351.setFrequencyInteger(0, 140000, &actual_freq); // CLK0 à ~140.6 kHz, carré 50%
        USR_SW_4_Counter = 3;
    }
    else if (USR_SW_4_Counter == 3)
    {
        si5351.setFrequencyInteger(0, 1400000, &actual_freq); // CLK0 à ~1.406 MHz, carré 50%
        USR_SW_4_Counter = 4;
    }
    else if (USR_SW_4_Counter == 4)
    {
        si5351.setFrequencyInteger(0, 14000000, &actual_freq); // CLK0 à ~14.06 MHz, carré 50%
        USR_SW_4_Counter = 5;
    }
    else
    {
        si5351.disableOutput(0);
        actual_freq = 0;
        USR_SW_4_Counter = 0;
    }
        
    printf("CLK0 Real frequency = %lu Hz\n", actual_freq);
}

void c_IO_Manager::USR_SW_5_CallBack()
{
    printf("USR SW 5\n");

    uint32_t actual_freq;
    if (USR_SW_5_Counter == 0)
    {
        
        USR_SW_5_Counter = 1;
    }
    else
    {
        si5351.disableOutput(0);
        actual_freq = 0;
        USR_SW_5_Counter = 0;
    }

    printf("CLK0 Real frequency = %lu Hz\n", actual_freq);
}

void c_IO_Manager::setFrequency(uint32_t freq)
{
    currentFreq = freq;
    // Ne change pas les sorties directement ici
}

uint32_t c_IO_Manager::getFrequency() const
{
    return currentFreq;
}

void c_IO_Manager::enableTX()
{
    si5351.setFrequencyInteger(0, currentFreq, nullptr);
}

void c_IO_Manager::disableTX()
{
    si5351.disableOutput(0);
}

void c_IO_Manager::enableRX()
{
    si5351.setFrequencyInteger(1, currentFreq, nullptr);
    si5351.setFrequencyInteger(2, currentFreq, nullptr);

    // quadrature : 90° de déphasage
    uint32_t phase = (900000000 / currentFreq) / 4;
    si5351.setPhaseOffset(2, phase);
}

void c_IO_Manager::disableRX()
{
    si5351.disableOutput(1);
    si5351.disableOutput(2);
}