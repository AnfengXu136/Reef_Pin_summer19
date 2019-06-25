#include "stm32f10x.h"

#include "delay.h"
#include "lcd16x2.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>


int main(void)
{
	float depth = 0.02;
	float x = 123.4567; 
  char buf[100]; 
	sprintf(buf, "%g", depth);
	
	
	DelayInit();
	//lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Initialize USART with receive interrupt
	USART2_Init();
	
	while (1)
	{
		DelayMs(2000);
		USART2_PutString(buf);
	}
}


