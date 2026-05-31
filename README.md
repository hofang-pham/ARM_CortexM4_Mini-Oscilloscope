# 📈 Máy Hiện Sóng Cầm Tay (Mini Oscilloscope) 

Dự án thiết kế và lập trình máy hiện sóng mini cầm tay, tập trung vào việc tối ưu hóa tốc độ hiển thị và xử lý tín hiệu thời gian thực trên hệ thống nhúng.

## 🛠 Phân hệ Firmware & UI (Hoàng)
Kho lưu trữ này chứa mã nguồn cấu hình phần cứng và giao diện đồ họa.
- **Vi điều khiển:** STM32F407VET6 (ARM Cortex-M4)
- **Màn hình:** TFT LCD 3.2" ILI9341 (Giao tiếp SPI)
- **Kỹ thuật cốt lõi:**
  - Tối ưu hóa băng thông SPI bằng **DMA** (Direct Memory Access).
  - Thuật toán vẽ đồ thị liền mạch **Bresenham**.
  - Lấy mẫu tín hiệu ADC bất đồng bộ kết hợp ngắt Timer.

## ⚙️ Sơ đồ chân cắm (Pinout)
| Module | Chân STM32 | Chức năng |
| :--- | :---: | :--- |
| **TFT SPI** | `PA5` | SCK (Xung nhịp SPI) |
| | `PA7` | MOSI (Truyền dữ liệu SPI) |
| **TFT Control**| `PA2` | CS (Chip Select) |
| | `PA3` | DC (Data / Command) |
| | `PA4` | RES (Reset phần cứng) |
| **ADC Input** | `PA0` | Kênh đo tín hiệu Analog |
| **Buttons** | `PE4` | Nút K0 (Chuyển đổi Scale Volt/Div) |

## 🚀 Hướng dẫn sử dụng & Build
1. Clone repository này về máy tính.
2. Mở file `.ioc` bằng phần mềm **STM32CubeIDE**.
3. Bấm biểu tượng cây búa (Build) để biên dịch project.
4. Nạp file `.elf` hoặc `.hex` trong thư mục `Debug/` xuống board STM32.

---
