#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>

#define USART_BD_RATE 38400

struct __FILE { 
    int handle; 
};

void USART1_Init(void);

void USART2_Init(void);

void USART3_Init(void);

void USART3_PutChar(char c);

int fputc(int ch, FILE *f);

int sendPackage(uint8_t *package, uint8_t package_length);

extern int numInterrupt; // this is for debug
extern char receive;

#endif
