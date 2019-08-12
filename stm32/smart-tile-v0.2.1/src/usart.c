#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "protocol.h"
#include "parser.h"

// #pragma import(__use_no_semihosting)
FILE __stdout;

// Buffer for store received chars
#define BUF_SIZE	16
static char buf[BUF_SIZE];
static uint8_t USART1_STATUS = 0;
static uint8_t USART2_STATUS = 0;
static uint8_t USART3_STATUS = 0;

static uint8_t send_frame_buff[MAX_FRAME_LEN];
static uint8_t recv_package_buff[MAX_PACKAGE_LEN];

// ====================== Handler =====================

void echoHandler(char c) {
	// Index for receive buffer
	static uint8_t i = 0;
	// Read chars until newline
	if (c != '\n')
	{
		// Concat char to buffer
		// If maximum buffer size is reached, then reset i to 0
		if (i < BUF_SIZE - 1)
		{
			buf[i] = c;
			i++;
		}
		else
		{
			buf[i] = c;
			i = 0;
		}
	}
	else
	{
		// Echo received string to USART1
		printf("%s\n", buf);
		// Clear buffer
		memset(&buf[0], 0, sizeof(buf));
		i = 0;
	}
}


// ====================== USART1 ======================

void USART1_Init()
{
	// Initialization struct
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Step 1: Initialize USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	USART_InitStruct.USART_BaudRate = USART_BD_RATE;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);
	
	// Step 2: Initialize GPIO for Tx and Rx pin
	// Tx pin (PA9) initialization as push-pull alternate function
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	// Rx pin (PA10) initialization as input floating
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// Step 3: Enable USART receive interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	// Step 4: Initialize NVIC for USART IRQ
	// Set NVIC prority group to group 4 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	// Set System Timer IRQ at higher priority
	NVIC_SetPriority(SysTick_IRQn, 0);
	// Set USART1 IRQ at lower priority
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART1_STATUS = 1;
}


void USART1_PutChar(char c)
{
	// Wait until transmit data register is empty
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
	// Send a char using USART1
	USART_SendData(USART1, c);
}


void USART1_IRQHandler()
{
	// Check if the USART1 receive interrupt flag was set
	if (USART_GetITStatus(USART1, USART_IT_RXNE))
	{
		// Index for receive buffer
		static uint8_t i = 0;
		
		// Read received char
		char c = USART_ReceiveData(USART1);
		
		// Read chars until newline
		if (c != '\n')
		{
			// Concat char to buffer
			// If maximum buffer size is reached, then reset i to 0
			if (i < BUF_SIZE - 1)
			{
				buf[i] = c;
				i++;
			}
			else
			{
				buf[i] = c;
				i = 0;
			}
		}
		else
		{			
			// Echo received string to USART1
			printf("%s\n", buf);
			// Clear buffer
			memset(&buf[0], 0, sizeof(buf));
			i = 0;
		}
	}
}


// ====================== USART2 ======================

void USART2_Init()
{
	// Initialization struct
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Step 1: Initialize USART2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	USART_InitStruct.USART_BaudRate = USART_BD_RATE;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);
	USART_Cmd(USART2, ENABLE);
	
	// Step 2: Initialize GPIO for Tx and Rx pin
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// Tx pin (PA2) initialization as push-pull alternate function
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	// Rx pin (PA3) initialization as input floating
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// Step 3: Enable USART receive interrupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	// Step 4: Initialize NVIC for USART IRQ
	// Set NVIC prority group to group 4 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	// Set System Timer IRQ at higher priority
	NVIC_SetPriority(SysTick_IRQn, 0);
	// Set USART2 IRQ at lower priority
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART2_STATUS = 1;
}


void USART2_PutChar(char c)
{
	// Wait until transmit data register is empty
	while (!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
	// Send a char using USART2
	USART_SendData(USART2, c);
}


void USART2_IRQHandler()
{
	// Check if the USART2 receive interrupt flag was set
	if (USART_GetITStatus(USART2, USART_IT_RXNE))
	{
		// Index for receive buffer
		static uint8_t i = 0;
		
		// Read received char
		char c = USART_ReceiveData(USART2);
		
		// Read chars until newline
		if (c != '\n')
		{
			// Concat char to buffer
			// If maximum buffer size is reached, then reset i to 0
			if (i < BUF_SIZE - 1)
			{
				buf[i] = c;
				i++;
			}
			else
			{
				buf[i] = c;
				i = 0;
			}
		}
		else
		{			
			// Echo received string to USART1
			printf("%s\n", buf);
			// Clear buffer
			memset(&buf[0], 0, sizeof(buf));
			i = 0;
		}
	}
}


// ====================== USART3 ======================

void USART3_Init()
{
	// Initialization struct
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Step 1: Initialize USART3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	USART_InitStruct.USART_BaudRate = USART_BD_RATE;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStruct);
	USART_Cmd(USART3, ENABLE);
	
	// Step 2: Initialize GPIO for Tx and Rx pin
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// Tx pin (PB10) initialization as push-pull alternate function
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Rx pin (PB11) initialization as input floating
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// Step 3: Enable USART receive interrupt
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	// Step 4: Initialize NVIC for USART IRQ
	// Set NVIC prority group to group 4 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	// Set System Timer IRQ at higher priority
	NVIC_SetPriority(SysTick_IRQn, 0);
	// Set USART3 IRQ at lower priority
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART3_STATUS = 1;
}


void USART3_PutChar(char c)
{
	// Wait until transmit data register is empty
	while (!USART_GetFlagStatus(USART3, USART_FLAG_TXE));
	// Send a char using USART3
	USART_SendData(USART3, c);
}

void USART3_IRQHandler()
{
	// Check if the USART3 receive interrupt flag was set
	if (USART_GetITStatus(USART3, USART_IT_RXNE))
	{
		// Read received char
		char c = USART_ReceiveData(USART3);
		// echoHandler(c);
		uint8_t package_length = 0;
		if(recvByteHandler(c, recv_package_buff, &package_length)) {
			parsePackage(recv_package_buff, package_length);
		}
	}
}


// ====================== Printf ======================

_sys_exit(int x) 
{ 
    // x = x;
}

void putCharToPorts(int ch) {
	if(USART1_STATUS) {
	USART1_PutChar(ch);
	}
	
	if(USART2_STATUS) {
	USART2_PutChar(ch);
	}
	
	if(USART3_STATUS) {
		USART3_PutChar(ch);
	}
}

int fputc(int ch, FILE *f){
	putCharToPorts(ch);
    return ch;
}

int __io_putchar(int ch) {
	putCharToPorts(ch);
	return ch;
}

int sendPackage(uint8_t *package, uint8_t package_length) {
	uint8_t frame_length, i;
	packageToFrame(package, package_length, send_frame_buff, &frame_length);
	for(i = 0; i < frame_length; i++) {
		USART3_PutChar(send_frame_buff[i]);
	}
	return 1;
}
