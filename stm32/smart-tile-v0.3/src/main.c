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
	Init_Check_Status();
	
	int test = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4);

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
	printf("Test: %d\n", test);
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
	Eink_WakeDisplaySleep();
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

void Eink_Display_Charging(){
	ClearBuffer();
	DrawStringAt(10, 0, "Charging", &Font24, 2, 3, 1);
	Eink_WakeDisplaySleep();
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
		Eink_WakeDisplaySleep();
	}
	else {
		Eink_Display_Depth(depth);
	}
}

void underwater(void) {
	// clock for tasks
	int cnt_100ms = 0;

	// sensor data
	float pressure = -1.11, depth = -1.11, battery = 0;

	while (1) {
		// update data
		if(!(cnt_100ms % PERIOD_LED)) Toggle_LED_Green();
		if(!(cnt_100ms % PERIOD_DEPTH)) ms5803_getDepthAndPressure(&depth, &pressure);
		//printf("%f\n", ADC1_ReadBattery());
		battery += ADC1_ReadBattery();
		if(!(cnt_100ms % PERIOD_BATT)){
			battery /= PERIOD_BATT;
			battery = ADC1_ReadBattery();
		}
		// transmit
		if(!(cnt_100ms % PERIOD_PRINT_SENSOR)) printSensorData(pressure, depth, battery);
		// display
		if(cnt_100ms % PERIOD_EINK == PERIOD_EINK - 20) {
			// 2s before display digits
			// clear the screen to prevent from burning
			Eink_Clear();
		}
		if(!(cnt_100ms % PERIOD_EINK)) {
			einkUserLogic(pressure, depth, battery);
		}
		// reset the battery
		if(!(cnt_100ms % PERIOD_BATT)) battery = 0;
		// prevent overflow
		if(!(cnt_100ms % PERIOD_OVERALL)) cnt_100ms = 0;
			cnt_100ms++;
		DelayMs(100);
	}
}

void charging(void) {
	// clock for tasks
	int cnt_100ms = 0;

	// sensor data
	float pressure = -1.11, depth = -1.11, battery = 0;

	while (1) {
		// update data
		if(!(cnt_100ms % PERIOD_LED)) Toggle_LED_Green();
		//printf("%f\n", ADC1_ReadBattery());
		battery += ADC1_ReadBattery();
		if(!(cnt_100ms % PERIOD_BATT)){
			battery /= PERIOD_BATT;
			battery = ADC1_ReadBattery();
		}
		// transmit
		if(!(cnt_100ms % PERIOD_PRINT_SENSOR)){}// printSensorData(pressure, depth, battery);
		// display
		if(cnt_100ms % PERIOD_EINK == PERIOD_EINK - 20) {
			// 2s before display digits
			// clear the screen to prevent from burning
			Eink_Clear();
		}
		if(!(cnt_100ms % PERIOD_EINK)) {
			Eink_Display_Charging();
		}
		// reset the battery
		if(!(cnt_100ms % PERIOD_BATT)) battery = 0;
		// prevent overflow
		if(!(cnt_100ms % PERIOD_OVERALL)) cnt_100ms = 0;
			cnt_100ms++;
		DelayMs(100);
	}
}

int main(void)
{
	// initialization
	deviceSetup();

	while (1) {
		printf("Water State: %d\n", GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5));
		printf("Charging State: %d\n", GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));

		// under water state
		if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)) {
			printf("Under Water State\n");
			underwater();
		}
		// charging state
		else if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)) {
			printf("Charging State\n");
			charging();
		}
		DelayMs(1000);
	}
}

