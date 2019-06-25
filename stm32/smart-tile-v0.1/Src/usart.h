#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>

struct __FILE { 
    int handle; 
};

void USART1_Init(void);

void USART2_Init(void);

int fputc(int ch, FILE *f);

#endif
