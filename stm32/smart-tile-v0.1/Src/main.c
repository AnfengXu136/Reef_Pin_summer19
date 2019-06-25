#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "wkup.h"
#include "delay.h"
#include "usart.h"
#include "i2c.h"
#include "ms5803.h"
#include "ds1307.h"
#include "led_display.h"
#include "adc.h"
#include "eink.h"

// PA9:		Tx
// PA10:	Rx
// PB10:	SCL
// PB11:	SDA


void Device_Setup() {
	// wake up if pressed more than 2s
	DelayInit();
	WKUP_Init();
	
	// initialize peripherals
	USART1_Init();
	printf("##################\n");
	printf("USART1 Initialized\n");
	Eink_Init();
	printf("Eink Display Initialized\n");
	ADC1_Init();
	printf("ADC1 Initialized\n");
	ds1307_init();
	printf("RTC Initialized\n");
	ms5803_Init(ADDRESS_HIGH);
	printf("Depth Sensor Initialized\n");
	LED_7_Seg_Init();
	printf("LED Display Initialized\n");
	printf("------------------\n");
	printf("All Peripherals Initialized\n");
	printf("------------------\n");
	
	// register standby functions for peripherals
	Register_Standby_Funcs(LED_7_Seg_Pre_Standby);
	Register_Standby_Funcs(Eink_Standby);
	Register_Standby_Funcs(LED_7_Seg_Standby);
}

static uint8_t welcome_flag = 1;

int main(void)
{
	int cnt_100ms = 0;
	float depth = 0.0;
	char line1[30], line2[30], line3[30];
	
	Device_Setup();

	while (1)
	{
		if(!(cnt_100ms % 5)) Toggle_LED_Green(); // 0.5s
		if(!(cnt_100ms % 20)) ds1307_demo(line1, line2);		// 1s
		if(!(cnt_100ms % 20)) depth = ms5803_getDepth();		// 1s
		if(!(cnt_100ms % 20)) Battery_demo(line3);		// 2s
		if(!(cnt_100ms % 20)) LED_7_Seg_Display_Depth(depth);	// 2s
		if(!(cnt_100ms % 300)) {						// 30s
			if(welcome_flag) {
				welcome_flag = 0;
				Eink_Display_Welcome(line1, line2, line3);
			}
			else Eink_Display_Depth(depth);
		}
		
		if(!(cnt_100ms % 6000)) cnt_100ms = 0;

		cnt_100ms++;
		DelayMs(100);
	}
}

