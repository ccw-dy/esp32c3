#include "ws2812b_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>

static const char *TAG = "WS2812B";

// 全局变量
static rmt_channel_handle_t tx_chan = NULL;
static ws2812b_color_t led_strip_pixels[WS2812B_LED_COUNT];
static bool driver_initialized = false;

// WS2812B时序参数（基于10MHz时钟）
static const rmt_symbol_word_t ws2812b_t0h = {
    .level0 = 1,
    .duration0 = 4,  // 350ns / 100ns = 3.5 ≈ 4
    .level1 = 0,
    .duration1 = 8,  // 800ns / 100ns = 8
};

static const rmt_symbol_word_t ws2812b_t1h = {
    .level0 = 1,
    .duration0 = 7,  // 700ns / 100ns = 7
    .level1 = 0,
    .duration1 = 6,  // 600ns / 100ns = 6
};

// 创建WS2812B编码器
static esp_err_t ws2812b_rmt_new_encoder(rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    
    // 创建字节编码器
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = ws2812b_t0h,
        .bit1 = ws2812b_t1h,
        .flags.msb_first = 1,
    };
    
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, ret_encoder), 
                      TAG, "创建字节编码器失败");
    
    return ESP_OK;
}

// 初始化WS2812B驱动
esp_err_t ws2812b_init(gpio_num_t gpio_num)
{
    if (driver_initialized) {
        ESP_LOGW(TAG, "驱动已经初始化");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "初始化WS2812B驱动，GPIO: %d", gpio_num);
    
    // 创建RMT发送通道
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = gpio_num,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,  // 10MHz
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&tx_chan_config, &tx_chan), TAG, "创建RMT通道失败");
    
    // 启用RMT通道
    ESP_RETURN_ON_ERROR(rmt_enable(tx_chan), TAG, "启用RMT通道失败");
    
    // 初始化LED数组
    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
    
    driver_initialized = true;
    ESP_LOGI(TAG, "WS2812B驱动初始化成功");
    
    return ESP_OK;
}

// 设置单个像素颜色
esp_err_t ws2812b_set_pixel(uint16_t pixel_index, ws2812b_color_t color)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (pixel_index >= WS2812B_LED_COUNT) {
        ESP_LOGE(TAG, "像素索引超出范围: %d", pixel_index);
        return ESP_ERR_INVALID_ARG;
    }
    
    led_strip_pixels[pixel_index] = color;
    return ESP_OK;
}

// 设置所有像素颜色
esp_err_t ws2812b_set_all_pixels(ws2812b_color_t color)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    for (int i = 0; i < WS2812B_LED_COUNT; i++) {
        led_strip_pixels[i] = color;
    }
    
    return ESP_OK;
}

// 清除所有LED
esp_err_t ws2812b_clear(void)
{
    return ws2812b_set_all_pixels((ws2812b_color_t){0, 0, 0});
}

// 刷新LED显示
esp_err_t ws2812b_refresh(void)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 创建编码器
    rmt_encoder_handle_t encoder = NULL;
    esp_err_t ret = ws2812b_rmt_new_encoder(&encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建编码器失败");
        return ret;
    }
    
    // 准备发送数据
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    
    // 发送数据
    ret = rmt_transmit(tx_chan, encoder, led_strip_pixels, 
                       sizeof(led_strip_pixels), &tx_config);
    
    // 删除编码器
    rmt_del_encoder(encoder);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "发送数据失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 等待传输完成
    vTaskDelay(pdMS_TO_TICKS(1));
    
    return ESP_OK;
}

// 反初始化驱动
esp_err_t ws2812b_deinit(void)
{
    if (!driver_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "反初始化WS2812B驱动");
    
    if (tx_chan) {
        rmt_disable(tx_chan);
        rmt_del_channel(tx_chan);
        tx_chan = NULL;
    }
    
    driver_initialized = false;
    ESP_LOGI(TAG, "WS2812B驱动反初始化完成");
    
    return ESP_OK;
}

// 测试基本颜色
void ws2812b_test_basic_colors(void)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化，无法测试");
        return;
    }
    
    ESP_LOGI(TAG, "开始基本颜色测试");
    
    // 红色
    ESP_LOGI(TAG, "显示红色");
    ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_RED);
    ws2812b_refresh();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 绿色
    ESP_LOGI(TAG, "显示绿色");
    ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_GREEN);
    ws2812b_refresh();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 蓝色
    ESP_LOGI(TAG, "显示蓝色");
    ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_BLUE);
    ws2812b_refresh();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 白色
    ESP_LOGI(TAG, "显示白色");
    ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_WHITE);
    ws2812b_refresh();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 关闭
    ESP_LOGI(TAG, "关闭LED");
    ws2812b_clear();
    ws2812b_refresh();
    
    ESP_LOGI(TAG, "基本颜色测试完成");
}

// 测试彩虹效果
void ws2812b_test_rainbow(void)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化，无法测试");
        return;
    }
    
    ESP_LOGI(TAG, "开始彩虹效果测试");
    
    for (int i = 0; i < 256; i++) {
        // 生成彩虹颜色
        uint8_t r, g, b;
        if (i < 85) {
            r = 255 - i * 3;
            g = i * 3;
            b = 0;
        } else if (i < 170) {
            i -= 85;
            r = 0;
            g = 255 - i * 3;
            b = i * 3;
        } else {
            i -= 170;
            r = i * 3;
            g = 0;
            b = 255 - i * 3;
        }
        
        ws2812b_set_all_pixels((ws2812b_color_t){r, g, b});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    ws2812b_clear();
    ws2812b_refresh();
    ESP_LOGI(TAG, "彩虹效果测试完成");
}

// 测试渐变效果
void ws2812b_test_fade(void)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化，无法测试");
        return;
    }
    
    ESP_LOGI(TAG, "开始渐变效果测试");
    
    // 红色渐变
    for (int i = 0; i <= 255; i++) {
        ws2812b_set_all_pixels((ws2812b_color_t){i, 0, 0});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    for (int i = 255; i >= 0; i--) {
        ws2812b_set_all_pixels((ws2812b_color_t){i, 0, 0});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    // 绿色渐变
    for (int i = 0; i <= 255; i++) {
        ws2812b_set_all_pixels((ws2812b_color_t){0, i, 0});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    for (int i = 255; i >= 0; i--) {
        ws2812b_set_all_pixels((ws2812b_color_t){0, i, 0});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    // 蓝色渐变
    for (int i = 0; i <= 255; i++) {
        ws2812b_set_all_pixels((ws2812b_color_t){0, 0, i});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    for (int i = 255; i >= 0; i--) {
        ws2812b_set_all_pixels((ws2812b_color_t){0, 0, i});
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    ws2812b_clear();
    ws2812b_refresh();
    ESP_LOGI(TAG, "渐变效果测试完成");
}

// 测试闪烁效果
void ws2812b_test_blink(void)
{
    if (!driver_initialized) {
        ESP_LOGE(TAG, "驱动未初始化，无法测试");
        return;
    }
    
    ESP_LOGI(TAG, "开始闪烁效果测试");
    
    for (int i = 0; i < 10; i++) {
        // 开启
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_WHITE);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // 关闭
        ws2812b_clear();
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    ESP_LOGI(TAG, "闪烁效果测试完成");
}
