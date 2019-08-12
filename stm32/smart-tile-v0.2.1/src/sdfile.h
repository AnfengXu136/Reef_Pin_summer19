#include "rtc.h"
#include "ff.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rtc.h"
#include <stdbool.h>

FIL file;

int make_integer(float num);
float make_frac(float num, int integer);
void print_rtc_time_to_string(void);
void sd_mkfile(void);
void sd_data(float depth, float pressure);
void sd_closefile(void);
