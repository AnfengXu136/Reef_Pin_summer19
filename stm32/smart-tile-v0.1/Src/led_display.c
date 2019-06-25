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


#include "led_display.h"
#include "i2c.h"
#include "delay.h"

//save the digit on each position
//0 is the left most digit, 2 is the colon dots, 4 is the right most digit
uint16_t displaybuffer[8]; 

static const uint8_t numbertable[] = {
	0x3F, /* 0 */
	0x06, /* 1 */
	0x5B, /* 2 */
	0x4F, /* 3 */
	0x66, /* 4 */
	0x6D, /* 5 */
	0x7D, /* 6 */
	0x07, /* 7 */
	0x7F, /* 8 */
	0x6F, /* 9 */
	0x77, /* a */
	0x7C, /* b */
	0x39, /* C */
	0x5E, /* d */
	0x79, /* E */
	0x71, /* F */
};

void LED_7_Seg_SetBrightness(uint8_t b) {
  if (b > 15) b = 15;
	i2c_write_no_reg(LED_ADDR, HT16K33_CMD_BRIGHTNESS | b); 
}

void LED_7_Seg_BlinkRate(uint8_t b) {
  if (b > 3) b = 0; // turn off if not sure
  i2c_write_no_reg(LED_ADDR, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));
}

void LED_7_Seg_Init() {
	i2c_init();
	i2c_write_no_reg(LED_ADDR,0x21); // turn on oscillator
  LED_7_Seg_BlinkRate(HT16K33_BLINK_OFF);
  LED_7_Seg_SetBrightness(15); // max brightness
}

void LED_7_Seg_Pre_Standby() { // displays 88.88 to indicate entering standby mode
	LED_7_Seg_Display(88.88,2,DEC);
}

void LED_7_Seg_Standby() {
	LED_7_Seg_Clear();
	LED_7_Seg_WriteDisplay();
	LED_7_Seg_BlinkRate(HT16K33_BLINK_OFF);
}

//Display data in buffer on LED
//All the drawing and printing routines only change the displaybuffer
//call writeDispaly() to display on LED
void LED_7_Seg_WriteDisplay(void) {
	uint8_t buf[16];
	uint8_t i;
	for (i=0; i<8; i++) {
    buf[2*i]=displaybuffer[i] & 0xFF;    
    buf[2*i+1]=displaybuffer[i] >> 8;    
  }
	i2c_write_multi_with_reg(LED_ADDR, 0x00, buf,16); // start at address $00
}

void LED_7_Seg_Clear(void) {
	uint8_t i;
  for (i=0; i<8; i++) {
    displaybuffer[i] = 0;
  }
}
void LED_7_Seg_Display(double n, uint8_t fracDigits, uint8_t base){
		printFloat(n, fracDigits, base);
  	LED_7_Seg_WriteDisplay();
}

/******************************* 7 SEGMENT OBJECT */

void writeDigitRaw(uint8_t d, uint8_t bitmask) {
  if (d > 4) return;
  displaybuffer[d] = bitmask;
}

void drawColon(boolean state) {
  if (state==1)
    displaybuffer[2] = 0x2;
  else
    displaybuffer[2] = 0;
}

void writeColon(void) {

	uint8_t buf[2];
	buf[0]=displaybuffer[2] & 0xFF;
	buf[1]=displaybuffer[2] >> 8;
	i2c_write_multi_with_reg(LED_ADDR, 0x04, buf, 2);
}

//set left most bit if has a dot
void writeDigitNum(uint8_t d, uint8_t num, boolean dot) {
  if (d > 4) return;

  writeDigitRaw(d, numbertable[num] | (dot << 7));
}

void printFloat(double n, uint8_t fracDigits, uint8_t base) { 
  uint8_t numericDigits = 4;   // available digits on display
  boolean isNegative = false;  // true if the number is negative
  double toIntFactor;
	uint32_t displayNumber;
	int32_t tooBig;
	int i;
	
  // is the number negative?
  if(n < 0) {
    isNegative = true;  // need to draw sign later
    --numericDigits;    // the sign will take up one digit
    n *= -1;            // pretend the number is positive
  }
  
  // calculate the factor required to shift all fractional digits
  // into the integer part of the number
  toIntFactor = 1.0;
  for(i = 0; i < fracDigits; ++i) toIntFactor *= base;
  
  // create integer containing digits to display by applying
  // shifting factor and rounding adjustment
  displayNumber = n * toIntFactor + 0.5;
  
  // calculate upper bound on displayNumber given
  // available digits on display
  tooBig = 1;
  for(i = 0; i < numericDigits; ++i) tooBig *= base;
  
  // if displayNumber is too large, try fewer fractional digits
  while(displayNumber >= tooBig) {
    --fracDigits;
    toIntFactor /= base;
    displayNumber = n * toIntFactor + 0.5;
  }
  
  // did toIntFactor shift the decimal off the display?
  if (toIntFactor < 1) {
    printError();
  } else {
    // otherwise, display the number
    int8_t displayPos = 4;
    
    if (displayNumber)  //if displayNumber is not 0
    {
      for(i = 0; displayNumber || i <= fracDigits; ++i) {
        boolean displayDecimal = (fracDigits != 0 && i == fracDigits);
        writeDigitNum(displayPos--, displayNumber % base, displayDecimal);
        if(displayPos == 2){
					if (fracDigits==2)
						writeDigitRaw(displayPos--, 0x2);
					else
						writeDigitRaw(displayPos--, 0x00);
				}
        displayNumber /= base;
      }
    }
    else {
      writeDigitNum(displayPos--, 0, false);
    }
  
    // display negative sign if negative
    if(isNegative) writeDigitRaw(displayPos--, 0x40);
  
    // clear remaining display positions
    while(displayPos >= 0) writeDigitRaw(displayPos--, 0x00);
  }
}

void printError(void) {
	uint8_t i;
  for(i = 0; i < SEVENSEG_DIGITS; ++i) {
    writeDigitRaw(i, (i == 2 ? 0x00 : 0x40));
  }
}

void LED_7_Seg_Demo(){
	//LED_7_Seg_Display(12.3,2,DEC);
	LED_7_Seg_Display(12.34,2,DEC);
	//LED_7_Seg_Display(1.234, 2, DEC);
}

void LED_7_Seg_Display_Depth(float depth) {
	LED_7_Seg_Display(depth,2,DEC);
}
