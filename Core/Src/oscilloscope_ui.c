#include "oscilloscope_ui.h"
#include <stdlib.h>
#include <stdio.h> // Để dùng hàm sprintf chuyển số thành chuỗi

static uint16_t old_wave_y[LCD_WIDTH];

// Biến quản lý tỉ lệ thu phóng (Do nút nhấn điều khiển)
uint8_t volt_div_scale = 1; // 1: Nguyên bản, 2: Phóng to gấp đôi

static void UI_DrawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        // Giới hạn không cho vẽ đè lên khu vực hiển thị thông số (x > 240)
        if(x1 < 240) LCD_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void UI_Init(void) {
    LCD_FillScreen_DMA(COLOR_BLACK);
    for(int i = 0; i < LCD_WIDTH; i++) old_wave_y[i] = UI_WAVE_Y_CENTER;
    UI_DrawGrid();
}

void UI_DrawGrid(void) {
    uint16_t grid_color = 0x0821;
    // Chỉ vẽ lưới trong khu vực hiển thị sóng (0 đến 240)
    for(int x = 0; x < 240; x += 40) {
        for(int y = 0; y < LCD_HEIGHT; y += 4) LCD_DrawPixel(x, y, grid_color);
    }
    for(int y = 0; y < LCD_HEIGHT; y += 30) {
        for(int x = 0; x < 240; x += 4) LCD_DrawPixel(x, y, grid_color);
    }

    // Vẽ đường biên chia cắt vùng sóng và vùng thông số
    for(int y = 0; y < 240; y++) LCD_DrawPixel(240, y, COLOR_WHITE);
}

// HÀM XỬ LÝ CHÍNH ĐÃ ĐƯỢC NÂNG CẤP
void UI_UpdateWaveform(uint16_t* adc_buffer) {
    uint16_t new_wave_y[240];
    uint16_t adc_max = 0;
    uint16_t adc_min = 4095;

    // 1. Tìm Max, Min của tín hiệu để tính toán Vpp
    for(int x = 0; x < 240; x++) {
        if(adc_buffer[x] > adc_max) adc_max = adc_buffer[x];
        if(adc_buffer[x] < adc_min) adc_min = adc_buffer[x];

        // Áp dụng biến volt_div_scale để phóng to/thu nhỏ đồ thị
        int32_t scaled_adc = (int32_t)adc_buffer[x];
        if(volt_div_scale == 2) {
            scaled_adc = (scaled_adc - 2048) * 2 + 2048; // Phóng từ tâm 1.65V
            if(scaled_adc > 4095) scaled_adc = 4095;
            if(scaled_adc < 0) scaled_adc = 0;
        }

        new_wave_y[x] = 239 - (scaled_adc * 239 / 4095);
    }

    // 2. Thuật toán đo Tần số cơ bản (Đếm số lần cắt qua điểm giữa)
    uint32_t zero_cross_count = 0;
    uint16_t mid_level = (adc_max + adc_min) / 2;
    for(int x = 1; x < 240; x++) {
        if((adc_buffer[x-1] < mid_level && adc_buffer[x] >= mid_level) ||
           (adc_buffer[x-1] > mid_level && adc_buffer[x] <= mid_level)) {
            zero_cross_count++;
        }
    }
    // Ước lượng tần số (Tỉ lệ này phụ thuộc vào tốc độ lấy mẫu ADC thực tế của nhóm)
    uint32_t frequency = zero_cross_count * 12;

    // 3. Tẩy sóng cũ và Vẽ sóng mới
    for(int x = 1; x < 240; x++) {
        if(new_wave_y[x-1] != old_wave_y[x-1] || new_wave_y[x] != old_wave_y[x]) {
            UI_DrawLine(x-1, old_wave_y[x-1], x, old_wave_y[x], COLOR_BLACK);
            UI_DrawLine(x-1, new_wave_y[x-1], x, new_wave_y[x], COLOR_GREEN);
            old_wave_y[x-1] = new_wave_y[x-1];
        }
    }
    old_wave_y[239] = new_wave_y[239];
    UI_DrawGrid();

    // 4. HIỂN THỊ CÁC THÔNG SỐ ĐO LƯỜNG LÊN MÀN HÌNH (Vùng x > 240)
    float vpp = ((adc_max - adc_min) * 3.3f) / 4095.0f;
    char str_buf[16];

    // In thông số Vpp
    sprintf(str_buf, "Vpp:%.2fV", vpp);
    LCD_DrawString(245, 20, str_buf, COLOR_YELLOW, COLOR_BLACK);

    // In thông số Tần số Hz
    if(vpp > 0.2f) { // Tín hiệu đủ lớn thì mới hiện tần số tránh nhiễu
        sprintf(str_buf, "Hz :%d", frequency);
    } else {
        sprintf(str_buf, "Hz :0  ");
    }
    LCD_DrawString(245, 50, str_buf, COLOR_WHITE, COLOR_BLACK);

    // In thông số Thang đo hiện tại
    sprintf(str_buf, "Scale:X%d", volt_div_scale);
    LCD_DrawString(245, 90, str_buf, COLOR_RED, COLOR_BLACK);
}
