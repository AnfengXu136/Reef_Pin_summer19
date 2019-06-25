/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/
  ----> http://www.adafruit.com/products/

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#ifndef led_display_h
#define led_display_h

#include "stm32f10x.h"

#define LED_ADDR 0x70
#define LED_ON 1
#define LED_OFF 0

#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3
 
#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5

typedef int boolean;
enum {false, true};

//this is the raw HT16K33 controller

void LED_7_Seg_Init(void);
void LED_7_Seg_Pre_Standby(void);
void LED_7_Seg_Standby(void);
void LED_7_Seg_SetBrightness(uint8_t b);
void LED_7_Seg_BlinkRate(uint8_t b);
void LED_7_Seg_WriteDisplay(void);
void LED_7_Seg_Clear(void);
void LED_7_Seg_Display(double n, uint8_t fracDigits, uint8_t base);
void LED_7_Seg_Demo(void);
void LED_7_Seg_Display_Depth(float depth);

// 7 segment
#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

void writeDigitRaw(uint8_t x, uint8_t bitmask);
void writeDigitNum(uint8_t x, uint8_t num, boolean dot);
void writeColon(void);
void drawColon(boolean state);
void printFloat(double, uint8_t, uint8_t);
void printError(void);

#endif 

