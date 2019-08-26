#include "stm32f10x_gpio.h"
#include "delay.h"
#include "spi.h"
#include "eink.h"
#include <stdio.h>


// image buffer
static uint8_t image_buff[4736]; // 128 * 296 / 8
static int width = 128;
static int height = 296;

// private functions
void Eink_Reset(void);
void Eink_Sleep(void);
void Eink_SendCommand(uint8_t command);
void Eink_SendData(uint8_t data);
void Eink_SetLut(void);
void Eink_WaitUntilIdle(void);


void Eink_Reset() {
    GPIO_ResetBits(GPIOA, RST_PIN);
    DelayMs(200);
	GPIO_SetBits(GPIOA, RST_PIN);
    DelayMs(200);
}



void Eink_Sleep() {
    Eink_SendCommand(DEEP_SLEEP_MODE);
    Eink_WaitUntilIdle();
    GPIO_ResetBits(GPIOA, RST_PIN);
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

const unsigned char lut_full_update[] =
{
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

/**
 *  @brief: set the look-up table register
 */
void Eink_SetLut() {
	int i;
    Eink_SendCommand(WRITE_LUT_REGISTER);
    /* the length of look-up table is 30 bytes */
    for (i = 0; i < 30; i++) {
        Eink_SendData(lut_full_update[i]);
    }
}


void Eink_WaitUntilIdle() {
	float time_out = 0;
	float thresh = 5;
    while(GPIO_ReadInputDataBit(GPIOA, BUSY_PIN) == 1 && time_out < thresh) {      //LOW: idle, HIGH: busy
        DelayMs(100);
			time_out += 0.1;
    }      
}

/**
 *  @brief: private function to specify the memory area for data R/W
 */
void Eink_SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
    Eink_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    Eink_SendData((x_start >> 3) & 0xFF);
    Eink_SendData((x_end >> 3) & 0xFF);
    Eink_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    Eink_SendData(y_start & 0xFF);
    Eink_SendData((y_start >> 8) & 0xFF);
    Eink_SendData(y_end & 0xFF);
    Eink_SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void Eink_SetMemoryPointer(int x, int y) {
    Eink_SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    Eink_SendData((x >> 3) & 0xFF);
    Eink_SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    Eink_SendData(y & 0xFF);
    Eink_SendData((y >> 8) & 0xFF);
    Eink_WaitUntilIdle();
}

// from buffer to display

/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void Eink_ClearFrameMemory(uint8_t color) {
	int i;
    Eink_SetMemoryArea(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    Eink_SetMemoryPointer(0, 0);
    Eink_SendCommand(WRITE_RAM);
    /* send the color data */
    for (i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        Eink_SendData(color);  // 0xFF for white
    }
}

/**
 *  @brief: update the display
 *          there are 2 memory areas embedded in the e-paper display
 *          but once this function is called,
 *          the the next action of SetFrameMemory or ClearFrame will 
 *          set the other memory area.
 */
void Eink_DisplayFrame(void) {
    Eink_SendCommand(DISPLAY_UPDATE_CONTROL_2);
    Eink_SendData(0xC4);
    Eink_SendCommand(MASTER_ACTIVATION);
    Eink_SendCommand(TERMINATE_FRAME_READ_WRITE);
    // Eink_WaitUntilIdle();
}


void Eink_SetFrameMemory(uint8_t * image_buffer) {
	int i;
    Eink_SetMemoryArea(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    Eink_SetMemoryPointer(0, 0);
    Eink_SendCommand(WRITE_RAM);
    /* send the image data */
    for (i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        Eink_SendData(image_buffer[i]);
    }
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
	int rotate = ROTATE_270;
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
void DrawCharAt(int x, int y, char ascii_char, sFONT* font, float scale_x, float scale_y, int colored) {
    int i, j, ii, jj;
    unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t * ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if ( *ptr & (0x80 >> (i % 8))) {
								for(jj = (int)(j * scale_y); jj < (int)((j + 1) * scale_y); jj++) {
										for(ii = (int)(i * scale_x); ii < (int)((i + 1) * scale_x); ii++) {
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
void DrawStringAt(int x, int y, char* text, sFONT* font, float scale_x, float scale_y, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
        /* Display one character on EPD */
        DrawCharAt(refcolumn, y, *p_text, font, scale_x, scale_y, colored);
        /* Decrement the column position by 16 */
        refcolumn += (int)(font->Width * scale_x);
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
    Eink_SendCommand(DRIVER_OUTPUT_CONTROL);
    Eink_SendData((EPD_HEIGHT - 1) & 0xFF);
    Eink_SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    Eink_SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    Eink_SendCommand(BOOSTER_SOFT_START_CONTROL);
    Eink_SendData(0xD7);
    Eink_SendData(0xD6);
    Eink_SendData(0x9D);
    Eink_SendCommand(WRITE_VCOM_REGISTER);
    Eink_SendData(0xA8);                     // VCOM 7C
    Eink_SendCommand(SET_DUMMY_LINE_PERIOD);
    Eink_SendData(0x1A);                     // 4 dummy lines per gate
    Eink_SendCommand(SET_GATE_TIME);
    Eink_SendData(0x08);                     // 2us per line
    Eink_SendCommand(BORDER_WAVEFORM_CONTROL);
    Eink_SendData(0x03);
    Eink_SendCommand(DATA_ENTRY_MODE_SETTING);
    Eink_SendData(0x03);                     // X increment; Y increment
    Eink_SetLut();
    /* EPD hardware init end */
}


void Eink_Standby(void) {
	Eink_WaitUntilIdle();
	Eink_ClearFrameMemory(0x00);
	Eink_DisplayFrame();
	Eink_WaitUntilIdle();
	Eink_ClearFrameMemory(0xFF);
	Eink_DisplayFrame();
	Eink_WaitUntilIdle();
	Eink_Sleep();
}


// }
// 
// 
// void Eink_demo(void) {
// 	//Eink_Init();
// 	//if(GPIO_ReadInputDataBit(GPIOA, BUSY_PIN) == 0) printf("!!!!!!!!!!!!!!!!!!!!!!\n");
// 	//uint8_t * dummy_buffer;
// 	ClearBuffer();
// 	//DrawCharAt(0, 0, 'a', &Font24, 1);
// 	DrawStringAt(0, 30, "12.34", &Font24, 3.5, 1);
// 	//ClearFrame();
// 	DisplayBlackFrame(image_buff);
// 	// DisplayFrameOnly();
// }
// 



void Eink_SetAndDisplay() {
	Eink_SetFrameMemory(image_buff);
	Eink_DisplayFrame();
}

void Eink_WakeDisplaySleep(){
	Eink_Reset();
	Eink_SetAndDisplay();
	Eink_Sleep();
}

void Eink_Clear(){
	Eink_Reset();
	Eink_ClearFrameMemory(0xFF);
	Eink_DisplayFrame();
	Eink_Sleep();
}

