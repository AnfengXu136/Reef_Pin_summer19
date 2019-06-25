#include "stm32f10x_gpio.h"
#include "delay.h"
#include "spi.h"
#include "eink.h"
#include "fonts.h"
#include <stdio.h>
#include <math.h>

// image buffer
static uint8_t image_buff[4736]; // 128 * 296 / 8
static int width = 128;
static int height = 296;

// private functions
void Eink_Reset(void);
void Eink_Sleep(void);
void Eink_SendCommand(uint8_t command);
void Eink_SendData(uint8_t data);
void Eink_WaitUntilIdle(void);


void Eink_Reset() {
    GPIO_ResetBits(GPIOA, RST_PIN);
    DelayMs(200);
		GPIO_SetBits(GPIOA, RST_PIN);
    DelayMs(200);   
}


void Eink_Sleep() {
  Eink_SendCommand(DEEP_SLEEP);
  Eink_SendData(0xa5);
}


void Eink_SendCommand(uint8_t command) {
	GPIO_ResetBits(GPIOA, DC_PIN);
  //DigitalWrite(dc_pin, LOW);
  SPIx_Transfer(command);
}


void Eink_SendData(uint8_t data) {
    GPIO_SetBits(GPIOA, DC_PIN);
  //DigitalWrite(dc_pin, HIGH);
  SPIx_Transfer(data);
}


void Eink_WaitUntilIdle() {
	float time_out = 0;
	float thresh = 15;
    while(GPIO_ReadInputDataBit(GPIOA, BUSY_PIN) == 0 && time_out < thresh) {      //0: busy, 1: idle
        DelayMs(100);
			time_out += 0.1;
    }      
}

// from buffer to display

/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void SetPartialWindowBlack(uint8_t * buffer_black, int x, int y, int w, int l) {
	  int i;
    Eink_SendCommand(PARTIAL_IN);
    Eink_SendCommand(PARTIAL_WINDOW);
    Eink_SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    Eink_SendData(((x & 0xf8) + w  - 1) | 0x07);
    Eink_SendData(y >> 8);        
    Eink_SendData(y & 0xff);
    Eink_SendData((y + l - 1) >> 8);        
    Eink_SendData((y + l - 1) & 0xff);
    Eink_SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    DelayMs(2);
    Eink_SendCommand(DATA_START_TRANSMISSION_1);
    for(i = 0; i < w  / 8 * l; i++) {
        Eink_SendData(buffer_black[i]);  
    }
    DelayMs(2);
    Eink_SendCommand(PARTIAL_OUT);
}


/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void SetPartialWindowRed(uint8_t * buffer_red, int x, int y, int w, int l) {
	  int i;
    Eink_SendCommand(PARTIAL_IN);
    Eink_SendCommand(PARTIAL_WINDOW);
    Eink_SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    Eink_SendData(((x & 0xf8) + w  - 1) | 0x07);
    Eink_SendData(y >> 8);        
    Eink_SendData(y & 0xff);
    Eink_SendData((y + l - 1) >> 8);        
    Eink_SendData((y + l - 1) & 0xff);
    Eink_SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    DelayMs(2);
    Eink_SendCommand(DATA_START_TRANSMISSION_2);
    for(i = 0; i < w  / 8 * l; i++) {
        Eink_SendData(buffer_red[i]);  
    }
    DelayMs(2);
    Eink_SendCommand(PARTIAL_OUT);
}


/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void ClearFrame(void) {
	int i;
    Eink_SendCommand(TCON_RESOLUTION);
    Eink_SendData(width >> 8);
    Eink_SendData(width & 0xff);
    Eink_SendData(height >> 8);        
    Eink_SendData(height & 0xff);

    Eink_SendCommand(DATA_START_TRANSMISSION_1);           
    DelayMs(2);
    for(i = 0; i < width * height / 8; i++) {
        Eink_SendData(0xFF);  
    }  
    DelayMs(2);
    Eink_SendCommand(DATA_START_TRANSMISSION_2);           
    DelayMs(2);
    for(i = 0; i < width * height / 8; i++) {
        Eink_SendData(0xFF);  
    }  
    DelayMs(2);
}


void DisplayFrameOnly(void) {
	printf("Send Display Command\n");
	Eink_SendCommand(DISPLAY_REFRESH);
	printf("Wait for Idle\n");
	Eink_WaitUntilIdle();
	printf("Display finished\n");
}


// draw to buffer

/**
 *  @brief: clear buffer
 */
void ClearBuffer(void) {
	int i, j;
	for(j = 0; j < 296; j++) {
		for(i = 0; i < 16; i++) { // 128/8
			image_buff[j * 16 + i] = (IF_INVERT_COLOR)? 0x00 : 0xFF;
		}
	}
}


void DrawAbsolutePixel(int x, int y, int colored) {
	// 0xFF is white, 0x00 is black
	// draw as black as default
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    if (IF_INVERT_COLOR) {
        if (colored) {
            image_buff[(x + y * width) / 8] |= 0x80 >> (x % 8);
        } else {
            image_buff[(x + y * width) / 8] &= ~(0x80 >> (x % 8));
        }
    } else {
        if (colored) {
            image_buff[(x + y * width) / 8] &= ~(0x80 >> (x % 8));  // call by default
        } else {
            image_buff[(x + y * width) / 8] |= 0x80 >> (x % 8);
        }
		}
}


void DrawPixel(int x, int y, int colored) {
    int point_temp;
	int rotate = ROTATE_90;
    if (rotate == ROTATE_0) {
        if(x < 0 || x >= width || y < 0 || y >= height) {
            return;
        }
        DrawAbsolutePixel(x, y, colored);
    } else if (rotate == ROTATE_90) {
        if(x < 0 || x >= height || y < 0 || y >= width) {
          return;
        }
        point_temp = x;
        x = width - y;
        y = point_temp;
        DrawAbsolutePixel(x, y, colored);
    } else if (rotate == ROTATE_180) {
        if(x < 0 || x >= width || y < 0 || y >= height) {
          return;
        }
        x = width - x;
        y = height - y;
        DrawAbsolutePixel(x, y, colored);
    } else if (rotate == ROTATE_270) {
        if(x < 0 || x >= height || y < 0 || y >= width) {
          return;
        }
        point_temp = x;
        x = y;
        y = height - point_temp;
        DrawAbsolutePixel(x, y, colored);
    }
}

/**
 *  @brief: this draws a charactor on the frame buffer but not refresh
 */
void DrawCharAt(int x, int y, char ascii_char, sFONT* font, float scale, int colored) {
    int i, j, ii, jj;
    unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t * ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if ( *ptr & (0x80 >> (i % 8))) {
								for(jj = (int)(j * scale); jj < (int)((j + 1) * scale); jj++) {
										for(ii = (int)(i * scale); ii < (int)((i + 1) * scale); ii++) {
											  DrawPixel(x + ii, y + jj, colored);
										}
										
								}
            }
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}

/**
*  @brief: this displays a string on the frame buffer but not refresh
*/
void DrawStringAt(int x, int y, char* text, sFONT* font, float scale, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
        /* Display one character on EPD */
        DrawCharAt(refcolumn, y, *p_text, font, scale, colored);
        /* Decrement the column position by 16 */
        refcolumn += (int)(font->Width * scale);
        /* Point on the next character */
        p_text++;
        counter++;
    }
}


// public functions

void Eink_Init(void) {
	  SPIx_Init();
	/* EPD hardware init start */
    Eink_Reset();
    Eink_SendCommand(BOOSTER_SOFT_START);
    Eink_SendData(0x17);
    Eink_SendData(0x17);
    Eink_SendData(0x17);
    Eink_SendCommand(POWER_ON);
    Eink_WaitUntilIdle();
    Eink_SendCommand(PANEL_SETTING);
    Eink_SendData(0x8F);
    Eink_SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    Eink_SendData(0x77);
    Eink_SendCommand(TCON_RESOLUTION);
    Eink_SendData(0x80);
    Eink_SendData(0x01);
    Eink_SendData(0x28);
    Eink_SendCommand(VCM_DC_SETTING_REGISTER);
    Eink_SendData(0X0A);
    /* EPD hardware init end */
}


void Eink_Standby(void) {
	Eink_WaitUntilIdle();
	ClearFrame();
	DisplayFrameOnly();
	Eink_WaitUntilIdle();
	//Eink_Sleep();
}

void DisplayBlackFrame(uint8_t * frame_buffer_black) {
	int i;
    Eink_SendCommand(DATA_START_TRANSMISSION_1);
        DelayMs(2);
        for (i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++) {
            Eink_SendData(frame_buffer_black[i]);
        }
        DelayMs(2);

        Eink_SendCommand(DATA_START_TRANSMISSION_2);
        DelayMs(2);
        for (i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++) {
            Eink_SendData(0xFF);
        }
        DelayMs(2);
		printf("Transmission Complete\n");
    Eink_SendCommand(DISPLAY_REFRESH);
    // Eink_WaitUntilIdle(); // No Stall
		printf("Display Complete\n");
}


void Eink_demo(void) {
	//Eink_Init();
	//if(GPIO_ReadInputDataBit(GPIOA, BUSY_PIN) == 0) printf("!!!!!!!!!!!!!!!!!!!!!!\n");
	//uint8_t * dummy_buffer;
	ClearBuffer();
	//DrawCharAt(0, 0, 'a', &Font24, 1);
	DrawStringAt(0, 30, "12.34", &Font24, 3.5, 1);
	//ClearFrame();
	DisplayBlackFrame(image_buff);
	// DisplayFrameOnly();
}

void Eink_Display_Depth(float depth) {
	char depth_str[10];
	int dig = (int)depth;
	int frac = (int)((fabsf(depth - dig)) * 100 + .5);
	ClearBuffer();
	sprintf(depth_str, "%2d.%02d", dig, frac);
	DrawStringAt(0, 30, depth_str, &Font24, 3.5, 1);
	DisplayBlackFrame(image_buff);
}

void Eink_Display_Welcome(char* line1, char* line2, char* line3){
	ClearBuffer();
	DrawStringAt(0, 0, "SIT  Ver 0.1", &Font24, 1.2, 1);
	DrawStringAt(0, 40, line1, &Font24, 1, 1);
	DrawStringAt(0, 70, line2, &Font24, 1, 1);
	DrawStringAt(0, 100, line3, &Font24, 1, 1);
	DisplayBlackFrame(image_buff);
}
