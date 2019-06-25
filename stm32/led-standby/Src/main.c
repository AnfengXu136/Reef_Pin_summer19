#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "wkup.h"

void delay(unsigned int nCount);

int main (void)
{	
	DelayInit();
	WKUP_Init();
	
	
	while (1)
	{
		/* Toggle LED on PA13 */
		Toggle_LED_Green();
		DelayMs(500);
	}
}

