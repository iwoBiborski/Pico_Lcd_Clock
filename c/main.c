#include "LCD_Test.h"   //Examples
#include <stdio.h>
#include "pico/stdlib.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(3000);  
    printf("Welcome \n");  
    LCD_2in_test();
    return 0;
}
