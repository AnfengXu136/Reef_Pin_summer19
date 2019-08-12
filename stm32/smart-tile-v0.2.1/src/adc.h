#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

void ADC1_Init(void);

float ADC1_ReadBattery(void);

#endif
