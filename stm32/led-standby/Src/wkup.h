#ifndef __WKUP_H
#define __WKUP_H
#include "stm32f10x.h"

void Toggle_LED_Green(void);

void Sys_Standby(void);
void Sys_Enter_Standby(void);
void EXTI0_IRQHandler(void);
void WKUP_Init(void);
uint8_t Check_WKUP(void);
#endif