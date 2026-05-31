#ifndef __OSCILLOSCOPE_UI_H
#define __OSCILLOSCOPE_UI_H

#include "lcd_ili9341.h"

// Các thông số khung hiển thị
#define UI_WAVE_X_START   0
#define UI_WAVE_X_END     319
#define UI_WAVE_Y_MIN     0
#define UI_WAVE_Y_MAX     239
#define UI_WAVE_Y_CENTER  120

// Khai báo hàm
void UI_Init(void);
void UI_DrawGrid(void);
void UI_UpdateWaveform(uint16_t* adc_buffer);

#endif
