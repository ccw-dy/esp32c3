#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "ws2812b_driver.h"

static const char *TAG = "MAIN";

// 定义GPIO引脚（您可以根据需要修改）
#define WS2812B_GPIO_PIN    GPIO_NUM_10   // 使用GPIO10，根据实际连接修改

// 测试任务句柄
static TaskHandle_t test_task_handle = NULL;

// 测试任务函数
static void ws2812b_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "WS2812B测试任务开始");
    
    // 等待一段时间让系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while (1) {
        ESP_LOGI(TAG, "=== 开始WS2812B测试循环 ===");
        
        // 1. 基本颜色测试
        ws2812b_test_basic_colors();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 2. 彩虹效果测试
        ws2812b_test_rainbow();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 3. 渐变效果测试
        ws2812b_test_fade();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 4. 闪烁效果测试
        ws2812b_test_blink();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 5. 自定义颜色序列测试
        ESP_LOGI(TAG, "开始自定义颜色序列测试");
        
        // 黄色
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_YELLOW);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 青色
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_CYAN);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 洋红色
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_MAGENTA);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 橙色
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_ORANGE);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 紫色
        ws2812b_set_all_pixels((ws2812b_color_t)WS2812B_COLOR_PURPLE);
        ws2812b_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 6. 呼吸灯效果
        ESP_LOGI(TAG, "开始呼吸灯效果测试");
        for (int i = 0; i < 3; i++) {
            // 红色呼吸
            for (int brightness = 0; brightness <= 255; brightness += 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){brightness, 0, 0});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            for (int brightness = 255; brightness >= 0; brightness -= 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){brightness, 0, 0});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            
            // 绿色呼吸
            for (int brightness = 0; brightness <= 255; brightness += 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){0, brightness, 0});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            for (int brightness = 255; brightness >= 0; brightness -= 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){0, brightness, 0});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            
            // 蓝色呼吸
            for (int brightness = 0; brightness <= 255; brightness += 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){0, 0, brightness});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            for (int brightness = 255; brightness >= 0; brightness -= 5) {
                ws2812b_set_all_pixels((ws2812b_color_t){0, 0, brightness});
                ws2812b_refresh();
                vTaskDelay(pdMS_TO_TICKS(30));
            }
        }
        
        // 7. 关闭LED
        ws2812b_clear();
        ws2812b_refresh();
        
        ESP_LOGI(TAG, "=== WS2812B测试循环完成，等待5秒后重新开始 ===");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// 初始化NVS
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    return ESP_OK;
}

// 初始化网络接口
static esp_err_t init_netif(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 WS2812B测试程序启动");
    ESP_LOGI(TAG, "芯片型号: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "ESP-IDF版本: %s", esp_get_idf_version());
    
    // 初始化NVS
    ESP_ERROR_CHECK(init_nvs());
    
    // 初始化网络接口
    ESP_ERROR_CHECK(init_netif());
    
    // 初始化WS2812B驱动
    ESP_LOGI(TAG, "初始化WS2812B驱动，使用GPIO: %d", WS2812B_GPIO_PIN);
    esp_err_t ret = ws2812b_init(WS2812B_GPIO_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B驱动初始化失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "WS2812B驱动初始化成功！");
    
    // 创建测试任务
    BaseType_t task_created = xTaskCreate(
        ws2812b_test_task,           // 任务函数
        "ws2812b_test",              // 任务名称
        4096,                        // 堆栈大小
        NULL,                        // 任务参数
        5,                           // 任务优先级
        &test_task_handle            // 任务句柄
    );
    
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "创建WS2812B测试任务失败");
        ws2812b_deinit();
        return;
    }
    
    ESP_LOGI(TAG, "WS2812B测试任务创建成功，系统启动完成！");
    ESP_LOGI(TAG, "LED将开始显示各种颜色和效果...");
    
    // 主任务可以在这里添加其他功能
    while (1) {
        // 主任务保持运行
        vTaskDelay(pdMS_TO_TICKS(10000));
        
        // 每10秒打印一次系统状态
        ESP_LOGI(TAG, "系统运行中... 可用堆内存: %d bytes", esp_get_free_heap_size());
    }
}