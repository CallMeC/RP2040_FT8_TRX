#include "usb_console.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

USBConsole::USBConsole()
{
    stdio_usb_init();
    current_freq = DEFAULT_FREQ; // fréquence par défaut
}

uint8_t USBConsole::poll()
{
    static char buf[64];
    static int idx = 0;

    while (true)
    {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT)
            break;

        // Display chars
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ' || c == '\r' || c == '\n')
            printf("%c", c);

        if (c == '\r' || c == '\n')
        {
            if (idx > 0)
            {
                buf[idx] = 0;
                printf("\n>> %s\n", buf);

                // === Analyse de la commande ===
                if (strcmp(buf, "T ON") == 0)
                {
                    //si5351_set_frequency_integer(0, current_freq, nullptr);
                    printf("Clock generation started on CLK0\n");
                    idx = 0;
                    return TASK_TX_ON;
                }
                else if (strcmp(buf, "T OFF") == 0)
                {
                    //si5351_disable_output(0);
                    printf("Clock generation stopped (CLK0 OFF)\n");
                    idx = 0;
                    return TASK_TX_OFF;
                }
                else if (strcmp(buf, "R ON") == 0)
                {
                    //si5351_set_frequency_integer(1, current_freq, nullptr);
                    //si5351_set_frequency_integer(2, current_freq, nullptr);
                    //si5351_set_phase_offset(2, (current_freq > 0) ? (900000000 / current_freq) / 4 : 0); // quadrature
                    printf("Clock generation started (CLK1 & CLK2, quadrature)\n");
                    idx = 0;
                    return TASK_RX_ON;
                }
                else if (strcmp(buf, "R OFF") == 0)
                {
                    //si5351_disable_output(1);
                    //si5351_disable_output(2);
                    printf("Clock generation stopped (CLK1 & CLK2 OFF)\n");
                    idx = 0;
                    return TASK_RX_OFF;
                }
                else if (strcmp(buf, "HELP") == 0)
                {
                    printf("Commands availiable :\n");
                    printf("- T [ON|OFF] : Enables / Disables CLK0 generation\n");
                    printf("- R [ON|OFF] : Enables / Disables CLK1 & CLK2 generation\n");
                    printf("- SETF [freq. (Hz)] : Sets frequency\n");
                    printf("- RESETF : Sets default frequency (%u Hz)\n", DEFAULT_FREQ);
                    idx = 0;
                    return TASK_NOOP;
                }
                else if (strcmp(buf, "RESETF") == 0)
                {
                    printf("Frequency set on : %u Hz\n", DEFAULT_FREQ);
                    idx = 0;
                    return TASK_RESETF;
                }
                else if (strncmp(buf, "SETF ", 5) == 0)
                {
                    uint32_t f = atoi(buf + 5);

                    if (f > 0 && f <= 160000000)
                    {
                        current_freq = f;
                        printf("Frequency set on : %lu Hz\n", current_freq);
                        idx = 0;
                        return TASK_SETF;
                    }
                    else
                    {
                        printf("Invalid frequency (range: 1 Hz - 160 000 000 Hz)\n");
                        idx = 0;
                        return TASK_NOOP;
                    }
                }
                else
                {
                    printf("BUF : %02X\n", buf);
                    printf("Commande inconnue.\n");
                    idx = 0;
                    return TASK_NOOP;
                }                
            }
        }
        else if (idx < (int)(sizeof(buf)) - 1)
        {
        // Ajoute uniquement si lettre ou espace
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ')
        {
            buf[idx++] = (char)c;
        }
}

    }    
}

uint32_t USBConsole::getFrequency()
{
    return current_freq;
}