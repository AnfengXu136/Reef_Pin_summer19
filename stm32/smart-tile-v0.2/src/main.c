#include "main.h"
#include "wkup.h"
#include "delay.h"
#include "usart.h"
#include "i2c.h"
#include "ms5803.h"
#include "adc.h"
#include "eink.h"
#include "fonts.h"
#include "rtc.h"
#include "power.h"

#include <math.h>

// PA9:		Tx
// PA10:	Rx
// PB10:	SCL
// PB11:	SDA
extern int RECV_CNT;
extern int dbg_line_flag;
extern char dbg_line[];


void deviceSetup() {
	// wake up if pressed more than 2s
	DelayInit();
	Init_LED_Green();
	WKUP_Init();
	Turn_On_Sensor_Power();
	
	// initialize peripherals
	USART1_Init();
	printf("##################\n");
	printf("USART1 Initialized\n");
	USART3_Init();
	printf("USART3 Initialized\n");
	RTC_Init();
	printf("RTC Initialized\n");
	Eink_Init();
	printf("Eink Display Initialized\n");
	ADC1_Init();
	printf("ADC1 Initialized\n");
	ms5803_Init(ADDRESS_HIGH);
	printf("Depth Sensor Initialized\n");
	printf("------------------\n");
	printf("All Peripherals Initialized\n");
	
	// register standby functions for peripherals
	// Register_Standby_Funcs(Eink_Standby);
	Register_Standby_Funcs(Turn_Off_Sensor_Power);
}

void printSensorData(float pressure, float depth, float battery) {
	printf("--------\n");
	printf("pressure  %.1f\n", pressure);
	printf("Vbatt     %.4fV\n", battery);
	printf("received  %d\n", RECV_CNT);
	RTC_Get();
	printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,
	calendar.w_date,calendar.hour,calendar.min,calendar.sec);
	if(dbg_line_flag) printf("package  %s\n", dbg_line);
}

void Eink_Display_Depth(float depth) {
	char depth_str[10];
	int dig = (int)depth;
	int frac = (int)((fabsf(depth - dig)) * 100 + .5);
	ClearBuffer();
	sprintf(depth_str, "%2d.%02d", dig, frac);
	DrawStringAt(0, 25, depth_str, &Font24, 3.5, 4, 1);
	Eink_SetAndDisplay();
}

void Eink_Display_Debug(float pressure, float depth, float battery) {
	char line[30];
	ClearBuffer();
	DrawStringAt(0, 0, "S.I.T.  Debug", &Font24, 1, 1.2, 1);
	sprintf(line, "Recv %d", RECV_CNT);
	DrawStringAt(0, 40, line, &Font24, 1, 1, 1);
	sprintf(line, "Pressure  %.1f", pressure);
	DrawStringAt(0, 70, line, &Font24, 1, 1, 1);
	//sprintf(line, "Vbatt %.4fV", battery);
	if(dbg_line_flag)  DrawStringAt(0, 100, dbg_line, &Font8, 1, 1, 1);
	Eink_SetAndDisplay();
}

void einkUserLogic(float pressure, float depth, float battery) {
	static uint8_t welcome_flag = 1;
	if(welcome_flag) {
		// char line[30];
		welcome_flag = 0;
		ClearBuffer();
		DrawStringAt(0, 10, "Smart Integrated Tile", &Font24, 0.8, 1, 1);
		DrawStringAt(0, 50, "Version 0.2", &Font24, 0.8, 0.9, 1);
		DrawStringAt(0, 90, "12/06/2019", &Font24, 0.8, 0.9, 1);

		// DrawStringAt(0, 70, line2, &Font24, 1, 1);
		// sprintf(line, "Vbatt %.4fV", battery);
		//DrawStringAt(0, 100, line, &Font24, 1, 1);
		Eink_SetAndDisplay();
	}
	else {
		Eink_Display_Depth(depth);
	}
}

int main(void)
{
	// clock for tasks
	int cnt_100ms = 0;
	
	// sensor data
	float pressure = -1.11, depth = -1.11, battery = -1.11;
	
	// initialization
	deviceSetup();

	while (1)
	{
		// update data
		if(!(cnt_100ms % PERIOD_LED)) Toggle_LED_Green();
		if(!(cnt_100ms % PERIOD_DEPTH)) ms5803_getDepthAndPressure(&depth, &pressure);
		if(!(cnt_100ms % PERIOD_BATT)) battery = ADC1_ReadBattery();
		// transmit
		if(!(cnt_100ms % PERIOD_PRINT_SENSOR)) printSensorData(pressure, depth, battery);
		// display
		if(cnt_100ms % PERIOD_EINK == PERIOD_EINK - 20) {
			// 2s before display digits
			// clear the screen to prevent from burning
			// Eink_ClearFrameMemory(0xFF);
			// Eink_DisplayFrame();
		}
		if(!(cnt_100ms % PERIOD_EINK)) {
			einkUserLogic(pressure, depth, battery);
		}
		// prevent overflow
		if(!(cnt_100ms % PERIOD_OVERALL)) cnt_100ms = 0;

		cnt_100ms++;
		DelayMs(100);
	}
}

