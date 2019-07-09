#include "wkup.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

static uint8_t Sensor_Power_Status = 0;
static uint8_t ZigBee_Power_Status = 0;
static uint8_t Sensor_Power_Initialized = 0;
static uint8_t ZigBee_Power_Initialized = 0;

void Init_Sensor_Power() {
	GPIO_InitTypeDef GPIO_InitStructure;
	// Set GPIO for Sensor Power
	// Enable clock for GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// Configure PB0 as push-pull output
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Turn_Off_Sensor_Power() {
	if(!Sensor_Power_Initialized) {
		Init_Sensor_Power();
		Sensor_Power_Initialized = 1;
        DelayMs(10);
	}
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	Sensor_Power_Status = 1;
}

void Turn_On_Sensor_Power() {
	if(!Sensor_Power_Initialized) {
		Init_Sensor_Power();
		Sensor_Power_Initialized = 1;
        DelayMs(10);
	}
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	Sensor_Power_Status = 0;
}

void Init_ZigBee_Power() {
	GPIO_InitTypeDef GPIO_InitStructure;
	// Set GPIO for Sensor Power
	// Enable clock for GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// Configure PB3 as push-pull output
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			// temporarily using 8 instead of 3 because PB3 does not work for some reason
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Turn_Off_ZigBee_Power() {
	if(!ZigBee_Power_Initialized) {
		Init_ZigBee_Power();
		ZigBee_Power_Initialized = 1;
        DelayMs(10);
	}
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	ZigBee_Power_Status = 1;
}

void Turn_On_ZigBee_Power() {
	if(!ZigBee_Power_Initialized) {
		Init_ZigBee_Power();
		ZigBee_Power_Initialized = 1;
        DelayMs(10);
	}
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
	ZigBee_Power_Status = 0;
}
