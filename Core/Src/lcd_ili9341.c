#include "lcd_ili9341.h"
#include "spi.h"   // Để gọi &hspi1

// Các Macro điều khiển chân nhanh
#define CS_LOW()     HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH()    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define DC_CMD()     HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define DC_DATA()    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)
#define RES_LOW()    HAL_GPIO_WritePin(LCD_RES_PORT, LCD_RES_PIN, GPIO_PIN_RESET)
#define RES_HIGH()   HAL_GPIO_WritePin(LCD_RES_PORT, LCD_RES_PIN, GPIO_PIN_SET)

// Gửi 1 byte Lệnh
static void LCD_WriteCmd(uint8_t cmd) {
    DC_CMD();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

// Gửi 1 byte Dữ liệu
static void LCD_WriteData(uint8_t data) {
    DC_DATA();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

// Khởi tạo màn hình
void LCD_Init(void) {
    // 1. Reset phần cứng cứng
    RES_LOW();
    HAL_Delay(50);
    RES_HIGH();
    HAL_Delay(50);

    // 2. Chuỗi mã khởi tạo chuẩn cho ILI9341
    LCD_WriteCmd(0x01); // Software reset
    HAL_Delay(100);

    LCD_WriteCmd(0x11); // Sleep out
    HAL_Delay(120);

    LCD_WriteCmd(0x36); // Memory Access Control (Xoay ngang)
    LCD_WriteData(0x28);

    LCD_WriteCmd(0x3A); // Pixel Format (16-bit RGB565)
    LCD_WriteData(0x55);

    LCD_WriteCmd(0x29); // Display ON
}

// Set vùng cửa sổ để vẽ
static void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_WriteCmd(0x2A); // Cột (X)
    LCD_WriteData(x1 >> 8); LCD_WriteData(x1 & 0xFF);
    LCD_WriteData(x2 >> 8); LCD_WriteData(x2 & 0xFF);

    LCD_WriteCmd(0x2B); // Hàng (Y)
    LCD_WriteData(y1 >> 8); LCD_WriteData(y1 & 0xFF);
    LCD_WriteData(y2 >> 8); LCD_WriteData(y2 & 0xFF);

    LCD_WriteCmd(0x2C); // Chuẩn bị ghi RAM
}

// Vẽ 1 điểm (Dùng cho vẽ sóng)
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    LCD_SetWindow(x, y, x, y);

    uint8_t color_buf[2] = {color >> 8, color & 0xFF};
    DC_DATA();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, color_buf, 2, HAL_MAX_DELAY);
    CS_HIGH();
}

// HÀM VŨ KHÍ: Xóa/Phủ màu toàn màn hình dùng DMA
// Giúp xóa màn hình trong chớp mắt mà không treo CPU
void LCD_FillScreen_DMA(uint16_t color) {
    static uint16_t color_swap; // Đảo byte vì SPI gửi MSB trước
    color_swap = (color >> 8) | (color << 8);

    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    DC_DATA();
    CS_LOW();

    // Gửi tín hiệu báo cho DMA bơm 76800 pixel (320*240) ra màn hình
    // Lưu ý: Tùy vào cấu hình DMA Width là Half-Word hay Byte để điều chỉnh.
    // Nếu CubeMX của ông để Data Width là Byte, hàm này cần đổi sang dùng mảng uint8_t.
    // Ở đây ta dùng cách đẩy liên tục cùng 1 ô nhớ color_swap (Cần cấu hình DMA ko tăng địa chỉ nguồn).

    // CÁCH CƠ BẢN NHẤT (Nếu chưa tối ưu ngắt DMA):
    for(uint32_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++) {
        HAL_SPI_Transmit(&hspi1, (uint8_t*)&color_swap, 2, HAL_MAX_DELAY);
    }
    CS_HIGH();
}
// Bảng mã Font 5x7 cho các ký tự cơ bản (Số, chữ cái, dấu chấm, dấu hai chấm)
static const uint8_t Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Khoảng trắng
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x03, 0x00, 0x03, 0x00}, // "
    {0x24, 0x7F, 0x24, 0x7F, 0x24}, // #
    {0x00, 0x00, 0x24, 0x00, 0x00}, // .
    {0x00, 0x00, 0x00, 0x00, 0x00}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ;
    {0x3E, 0x41, 0x41, 0x22, 0x14}, // C
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // H
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3F, 0x40, 0x40, 0x20, 0x5F}, // V
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // a
    {0x20, 0x54, 0x54, 0x54, 0x78}, // m
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x44, 0x3C, 0x04, 0x7C, 0x44}, // u
    {0x48, 0x54, 0x54, 0x54, 0x20}  // z
};

// Hàm ánh xạ ký tự ASCII sang bảng font thu nhỏ phía trên
static int GetFontIndex(char c) {
    if (c == ' ') return 0; if (c == '.') return 4; if (c == ':') return 16;
    if (c >= '0' && c <= '9') return 6 + (c - '0');
    if (c == 'C') return 18; if (c == 'F') return 19; if (c == 'L') return 20;
    if (c == 'H') return 21; if (c == 'O') return 22; if (c == 'P') return 23; if (c == 'V') return 24;
    if (c == 'a') return 25; if (c == 'm') return 26; if (c == 'p') return 27; if (c == 'u') return 28; if (c == 'z') return 29;
    return 0;
}

// Vẽ 1 ký tự lên màn hình
void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg) {
    int idx = GetFontIndex(c);
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t line = Font5x7[idx][i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & 0x01) {
                LCD_DrawPixel(x + i, y + j, color);
            } else if (bg != color) {
                LCD_DrawPixel(x + i, y + j, bg);
            }
            line >>= 1;
        }
    }
}

// Vẽ một chuỗi ký tự (String)
void LCD_DrawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg) {
    while (*str) {
        LCD_DrawChar(x, y, *str, color, bg);
        x += 6; // Dịch sang phải 6 pixel cho ký tự tiếp theo
        str++;
    }
}
