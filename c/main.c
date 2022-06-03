#include "Pico_Clock.h"   //Examples
#include <stdio.h>
#include "pico/stdlib.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(3000);  
    printf("Welcome \n");  
    Pico_Clock();
    return 0;
}
