#ifndef __LCD_ILI9341_H
#define __LCD_ILI9341_H

#include "main.h"

// --- MAP CHÂN VỚI CẤU HÌNH CỦA ÔNG ---
#define LCD_CS_PORT    GPIOA
#define LCD_CS_PIN     GPIO_PIN_2
#define LCD_DC_PORT    GPIOA
#define LCD_DC_PIN     GPIO_PIN_3
#define LCD_RES_PORT   GPIOA
#define LCD_RES_PIN    GPIO_PIN_4

// --- ĐỊNH NGHĨA MÀU SẮC (RGB565) ---
#define COLOR_BLACK    0x0000
#define COLOR_GREEN    0x07E0
#define COLOR_RED      0xF800
#define COLOR_WHITE    0xFFFF
#define COLOR_YELLOW   0xFFE0

// Kích thước màn hình
#define LCD_WIDTH      320
#define LCD_HEIGHT     240

// --- KHAI BÁO HÀM ---
void LCD_Init(void);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_FillScreen_DMA(uint16_t color);
// --- THÊM VÀO CUỐI FILE KHAI BÁO FONT ---
void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg);
void LCD_DrawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);
#endif
