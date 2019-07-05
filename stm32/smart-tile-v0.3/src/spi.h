#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

#define SPIx_RCC				RCC_APB2Periph_SPI1
#define SPIx						SPI1
#define SPI_GPIO_RCC		RCC_APB2Periph_GPIOA
#define SPI_GPIO				GPIOA

#define BUSY_PIN				GPIO_Pin_1
#define RST_PIN					GPIO_Pin_2
#define DC_PIN					GPIO_Pin_3
#define SPI_PIN_CS			GPIO_Pin_4	//SPI_PIN_SS
#define SPI_PIN_SCK			GPIO_Pin_5
#define SPI_PIN_MOSI		GPIO_Pin_7
// #define SPI_PIN_MISO		GPIO_Pin_6  // not using

void SPIx_Init(void);
uint8_t SPIx_Transfer(uint8_t data);
void SPIx_DisableSlave(void);

#endif
