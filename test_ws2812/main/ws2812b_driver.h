#ifndef WS2812B_DRIVER_H
#define WS2812B_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// WS2812B配置参数
#define WS2812B_LED_COUNT          1       // LED数量（您只有一个灯）
#define WS2812B_RMT_RESOLUTION_HZ  10000000 // RMT分辨率：10MHz
#define WS2812B_T0H_NS            350      // T0H时间：350ns
#define WS2812B_T0L_NS            800      // T0L时间：800ns
#define WS2812B_T1H_NS            700      // T1H时间：700ns
#define WS2812B_T1L_NS            600      // T1L时间：600ns
#define WS2812B_RESET_TIME_US      280      // 复位时间：280us

// 颜色结构体
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ws2812b_color_t;

// 预定义颜色
#define WS2812B_COLOR_RED      {255, 0, 0}
#define WS2812B_COLOR_GREEN    {0, 255, 0}
#define WS2812B_COLOR_BLUE     {0, 0, 255}
#define WS2812B_COLOR_WHITE    {255, 255, 255}
#define WS2812B_COLOR_BLACK    {0, 0, 0}
#define WS2812B_COLOR_YELLOW   {255, 255, 0}
#define WS2812B_COLOR_CYAN     {0, 255, 255}
#define WS2812B_COLOR_MAGENTA  {255, 0, 255}
#define WS2812B_COLOR_ORANGE   {255, 165, 0}
#define WS2812B_COLOR_PURPLE   {128, 0, 128}

// 函数声明
esp_err_t ws2812b_init(gpio_num_t gpio_num);
esp_err_t ws2812b_set_pixel(uint16_t pixel_index, ws2812b_color_t color);
esp_err_t ws2812b_set_all_pixels(ws2812b_color_t color);
esp_err_t ws2812b_clear(void);
esp_err_t ws2812b_refresh(void);
esp_err_t ws2812b_deinit(void);

// 测试函数
void ws2812b_test_basic_colors(void);
void ws2812b_test_rainbow(void);
void ws2812b_test_fade(void);
void ws2812b_test_blink(void);

#ifdef __cplusplus
}
#endif

#endif // WS2812B_DRIVER_H
