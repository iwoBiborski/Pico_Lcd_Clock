/*****************************************************************************
* | File      	:   LCD_2in_test.c
* | Author      :   Waveshare team
* | Function    :   2inch LCD test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-08-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "Pico_Clock.h"
#include "LCD_2in.h"
#include "hardware/uart.h"
#include "math.h"
#include "clock_states.h"

/* Graph origin */
static int x_origin = 10;
static int y_origin = 230;

/* Patient parameter values */
static int mass = 0;
static int height = 0;
static int age = 0;
static int sex = 0;

/* Data digits */
static int ctr_mass[3] = {0};
static int ctr_height[3] = {0};
static int ctr_age[3] = {0};
static int ctr_sex = 0;

/* Control keys pins? */
static int key0 = 15; 
static int key1 = 17; 
static int key2 = 2; 
static int key3 = 3;

static char start;

static int hours[12] = {12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

//static State_T current_state = PATIENT_DATA_INPUT_MASS;

static int data[320] = {0};

/* Set origin */
void Set_Origin(int x_org, int y_org)
{
    x_origin = x_org;
    y_origin = y_org;
}

/* Draws point of function graph relatively to origin */
void Draw_Point_Up_To_Origin(int x, double (*func)(double))
{
        Paint_DrawPoint(x, y_origin - (int)func((double)(x-x_origin)), RED, DOT_PIXEL_3X3, DOT_FILL_AROUND);
}

void Draw_Point_Up_To_Origin_From_Data(int x, int y)
{
        Paint_DrawPoint(x + x_origin, y_origin - y, BLUE, DOT_PIXEL_3X3, DOT_FILL_AROUND);
}

void Draw_Line_Up_To_Origin(int x1, int x2, double (*func)(double))
{
    Paint_DrawLine(x1, y_origin - (int)func((double)(x1-x_origin)), x2, y_origin - (int)func((double)(x2-x_origin)), RED, DOT_PIXEL_1X1, DOT_FILL_AROUND);
}

void Draw_Scale(uint16_t color, int div_X, int div_Y, int unit_X, int unit_Y)
{
    int ct = div_X;
    int numb = unit_X; 
    while(x_origin + ct < 310)
    {
        Paint_DrawLine(x_origin + ct, y_origin - 3, x_origin + ct, y_origin + 3, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(x_origin - ct, y_origin - 3, x_origin - ct, y_origin + 3, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        char nm[3];
        sprintf(nm, "%i", numb);
        Paint_DrawString_EN( x_origin + ct - 5, y_origin + 5 , nm, &Font8, 0x000f, 0xfff0);
        sprintf(nm, "%i", -numb);
        Paint_DrawString_EN( x_origin - ct - 10, y_origin + 5 , nm, &Font8, 0x000f, 0xfff0);
        ct += div_X;
        numb += unit_X;
    }
    ct = div_Y;
    numb = unit_Y;

    while(y_origin + ct < 210 || y_origin - ct > 0)
    {
        Paint_DrawLine(x_origin - 3, y_origin + ct, x_origin + 3, y_origin + ct, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(x_origin - 3, y_origin - ct, x_origin  + 3, y_origin - ct, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        char nm[3];
        sprintf(nm, "%i", -numb);
        Paint_DrawString_EN( x_origin - 20, y_origin + ct - 3, nm, &Font8, 0x000f, 0xfff0);
        sprintf(nm, "%i", numb);
        Paint_DrawString_EN( x_origin - 15, y_origin - ct - 3, nm, &Font8, 0x000f, 0xfff0);
        ct += div_Y;
        numb += unit_Y;
    }
}

void Draw_Cartesian(uint16_t color, int x_org, int y_org)
{
    if(x_org != x_origin || y_org != y_origin)
    {
        Set_Origin(x_org, y_org);
    }

    /*Draw X axis */
    Paint_DrawLine(0, y_origin, 320, y_origin, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(315, y_origin + 5, 320, y_origin, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(315, y_origin - 5, 320, y_origin, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        
    /*Draw Y axis */
    Paint_DrawLine(x_origin, 0, x_origin, 240, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(x_origin + 5, 5, x_origin, 0, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(x_origin - 5, 5, x_origin, 0, color ,DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Draw_Scale(color, 40, 30, 10, 10);
}

bool reserved_addr(uint8_t addr) {
return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void Draw_Digit(int i, int position, uint16_t x_start, uint16_t y_start)
{
    char buff[1];
    itoa(i, buff, 10);
    Paint_DrawString_EN(x_start + 12*position, y_start, buff, &Font20, BLACK,  WHITE);
}

void draw_three_digit_param(const char *param_name, int *ctrs, int x_start, int y_start)
{
    Paint_DrawString_EN(10, y_start, param_name, &Font20, BLACK,  WHITE);
    Draw_Digit(ctrs[0], 0, x_start, y_start);
    Draw_Digit(ctrs[1], 1, x_start, y_start);
    Draw_Digit(ctrs[2], 2, x_start, y_start);
}

void update_clock(Clock_T *clock)
{
    int sec = clock->sec;
    clock->sec = ++sec;
    if(sec == 60)
    {
        clock->min += 1;
        clock->hour +=1;
    }
    clock->sec = (clock->sec % 60);
    clock->min = (clock->min % 60);
    clock->hour = (clock->hour % 720);
}

Pair_T polar(float a, float b, float radius, float angle)
{
    Pair_T coord;
    coord.x = a + radius*cos(angle);
    coord.y = b + radius*sin(angle);
    return coord;
}

void draw_seconds(int seconds, uint16_t color)
{
    int sec = seconds % 60;
    Pair_T sec_p = polar(160.0, 120.0, 70.0, -M_PI_2 + ((float)sec)*(M_PI/30.0));
    Paint_DrawLine(160, 120, (int)sec_p.x, (int)sec_p.y, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

void draw_minutes(int minutes, uint16_t color)
{
    int min = minutes % 60;
    Pair_T min_p = polar(160.0, 120.0, 70.0, -M_PI_2 + ((float)min)*(M_PI/30.0));
    Paint_DrawLine(160, 120, (int)min_p.x, (int)min_p.y, color, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
}

void draw_hours(int hours, uint16_t color)
{
    int sec = hours;
    Pair_T sec_p = polar(160.0, 120.0, 40.0, -M_PI_2 + ((float)sec)*(M_PI/360.0));
    Paint_DrawLine(160, 120, (int)sec_p.x, (int)sec_p.y, color, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
}

void draw_clock()
{
    Paint_DrawCircle(160, 120, 102, GREEN, DOT_PIXEL_4X4, DRAW_FILL_FULL);

    Pair_T ct_12 = polar(160.0, 120.0, 100.0, -M_PI_2);
    //Paint_DrawLine(160, 120, 160, 20, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_12.x - 15, (int)ct_12.y + 5, "12", &Font20, GREEN, BLUE);
    
    Pair_T ct_1 = polar(160.0, 120.0, 100.0, -M_PI/3.0);
    //Paint_DrawLine(160, 120, (int)ct_1.x , (int)ct_1.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_1.x -13, (int)ct_1.y + 5, "1", &Font20, GREEN, BLUE);
    
    Pair_T ct_2 = polar(160.0, 120.0, 100.0, -M_PI/6.0);
    //Paint_DrawLine(160, 120, (int)ct_2.x , (int)ct_2.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_2.x - 20, (int)ct_2.y, "2", &Font20, GREEN, BLUE);
    
    Pair_T ct_3 = polar(160.0, 120.0, 100.0, 0.0);
    //Paint_DrawLine(160, 120, (int)ct_3.x , (int)ct_3.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_3.x - 18, (int)ct_3.y - 8, "3", &Font20, GREEN, BLUE);
    
    Pair_T ct_4 = polar(160.0, 120.0, 100.0, M_PI/6.0);
    //Paint_DrawLine(160, 120, (int)ct_4.x , (int)ct_4.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_4.x - 19, (int)ct_4.y - 12, "4", &Font20, GREEN, BLUE);

    Pair_T ct_5 = polar(160.0, 120.0, 100.0, M_PI/3.0);
    //Paint_DrawLine(160, 120, (int)ct_5.x ,sec_p (int)ct_5.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_5.x - 16, (int)ct_5.y - 18, "5", &Font20, GREEN, BLUE);

    Pair_T ct_6 = polar(160.0, 120.0, 100.0, M_PI/2.0);
    //Paint_DrawLine(160, 120, (int)ct_6.x, (int)ct_6.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_6.x - 6, (int)ct_6.y - 18, "6", &Font20, GREEN, BLUE);

    Pair_T ct_7 = polar(160.0, 120.0, 100.0, M_PI/2.0 + M_PI/6.0);
    //Paint_DrawLine(160, 120, (int)ct_7.x, (int)ct_7.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_7.x + 1, (int)ct_7.y - 16, "7", &Font20, GREEN, BLUE);
    
    Pair_T ct_8 = polar(160.0, 120.0, 100.0, M_PI_2 + M_PI/3.0);
    //Paint_DrawLine(160, 120, (int)ct_8.x , (int)ct_8.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_8.x + 2, (int)ct_8.y - 15, "8", &Font20, GREEN, BLUE);

    Pair_T ct_9 = polar(160.0, 120.0, 100.0, M_PI);
    //Paint_DrawLine(160, 120, (int)ct_9.x , (int)ct_9.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_9.x + 2, (int)ct_9.y - 9, "9", &Font20, GREEN, BLUE);

    Pair_T ct_10 = polar(160.0, 120.0, 100.0, M_PI + M_PI/6.0);
    //Paint_DrawLine(160, 120, (int)ct_10.x, (int)ct_10.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_10.x +2, (int)ct_10.y - 2, "10", &Font20, GREEN, BLUE);

    Pair_T ct_11 = polar(160.0, 120.0, 100.0, M_PI + M_PI/3.0);
    //Paint_DrawLine(160, 120, (int)ct_11.x , (int)ct_11.y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN((int)ct_11.x - 4, (int)ct_11.y + 2, "11", &Font20, GREEN, BLUE);
}

int Pico_Clock(void)
{
    DEV_Delay_ms(100);

    if(DEV_Module_Init()!=0){
        return -1;
    }
    DEV_SET_PWM(50);
    /* LCD Init */
    printf("2inch LCD demo...\r\n");
    LCD_2IN_Init(HORIZONTAL);
    LCD_2IN_Clear(BLACK);
    
    //LCD_SetBacklight(1023);
    UDOUBLE Imagesize = LCD_2IN_HEIGHT*LCD_2IN_WIDTH*2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        //printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)BlackImage,LCD_2IN.WIDTH,LCD_2IN.HEIGHT, 90, BLACK);
    Paint_SetScale(65);
    Paint_Clear(BLACK);
    Paint_SetRotate(ROTATE_270);
   
   SET_Infrared_PIN(key0);    
   SET_Infrared_PIN(key1);
   SET_Infrared_PIN(key2);
   SET_Infrared_PIN(key3);

   Paint_Clear(BLACK);
   LCD_2IN_Display((uint8_t * )BlackImage);

   draw_clock();
   int i = 0;
   int im = 0;
   long int ts1 = 0;
   long int ts2 = 0;
   uint64_t tm1 = 0;
   uint64_t tm2 = 0;
   bool run = false;
   draw_minutes(im, RED);
   draw_seconds(i, RED);
   draw_hours(0, RED);

    Clock_T clock_current = {0};
    Clock_T clock_run = {0};

   while(1)
   {
        if(DEV_Digital_Read(key0 ) == 0)
        {
            run = !run;
        }
        ts2 = time_us_64()/1000;

        if(run)
        {
            if(970 <= (ts2 -ts1) /*10 <= ts2 - ts1*/)
            {
                update_clock(&clock_run);
                ts1 = ts2;
                if(clock_run.sec != clock_current.sec)
                {
                    draw_seconds(clock_current.sec, GREEN);
                    draw_seconds(clock_run.sec, RED);
                    draw_minutes(clock_current.min, RED);
                    draw_hours(clock_current.hour, RED);
                }
                if(clock_run.min != clock_current.min)
                {
                    draw_minutes(clock_current.min, GREEN);
                    draw_minutes(clock_run.min, RED);
                    draw_hours(clock_current.hour, GREEN);
                    draw_hours(clock_run.hour, RED);
                }
                DEV_Delay_us(5250); //DEV_Delay_ms(5) also good
                clock_current = clock_run;
            }
        }

        else
        {
            ts1 = ts2;
        }
        LCD_2IN_Display((uint8_t * )BlackImage);       
   }

    /* Module Exit */
    free(BlackImage);
    BlackImage = NULL;
    
    DEV_Module_Exit();
    return 0;
}
