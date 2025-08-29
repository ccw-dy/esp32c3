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
#include "wifi_manager.h"
#include "wifi_config.h"
#include "ws2812b_config.h"

static const char *TAG = "MAIN";

// 任务句柄
static TaskHandle_t wifi_monitor_task_handle = NULL;

// WiFi状态回调函数
static void wifi_state_callback(wifi_state_t state, void *user_data)
{
    switch (state) {
        case WIFI_STATE_DISCONNECTED:
            ESP_LOGI(TAG, "WiFi状态: 未连接");
            // 可以在这里添加LED指示，比如红色闪烁
            break;
        case WIFI_STATE_CONNECTING:
            ESP_LOGI(TAG, "WiFi状态: 连接中");
            // 可以在这里添加LED指示，比如黄色常亮
            break;
        case WIFI_STATE_CONNECTED:
            ESP_LOGI(TAG, "WiFi状态: 已连接");
            // 可以在这里添加LED指示，比如绿色常亮
            break;
        case WIFI_STATE_FAILED:
            ESP_LOGE(TAG, "WiFi状态: 连接失败");
            // 可以在这里添加LED指示，比如红色常亮
            break;
        case WIFI_STATE_DISCONNECTING:
            ESP_LOGI(TAG, "WiFi状态: 断开连接中");
            break;
    }
}

// WiFi IP回调函数
static void wifi_ip_callback(const char *ip_addr, void *user_data)
{
    ESP_LOGI(TAG, "获取到IP地址: %s", ip_addr);
    
    // 可以在这里添加网络相关的功能
    // 比如启动HTTP服务器、MQTT客户端等
}

// WiFi监控任务
static void wifi_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "WiFi监控任务启动");
    
    while (1) {
        wifi_info_t wifi_info;
        if (wifi_manager_get_info(&wifi_info) == ESP_OK) {
            // 每10秒打印一次WiFi状态
            if (wifi_info.state == WIFI_STATE_CONNECTED) {
                ESP_LOGI(TAG, "WiFi状态: 已连接 | SSID: %s | RSSI: %d | IP: %s", 
                         wifi_info.ssid, wifi_info.rssi, wifi_manager_get_ip_string());
            } else {
                ESP_LOGI(TAG, "WiFi状态: %d", wifi_info.state);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));
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

// 初始化WiFi
static esp_err_t init_wifi(void)
{
    ESP_LOGI(TAG, "初始化WiFi管理器");
    
    // 初始化WiFi管理器
    ESP_ERROR_CHECK(wifi_manager_init());
    
    // 设置回调函数
    wifi_manager_set_event_callback(wifi_state_callback, NULL);
    wifi_manager_set_ip_callback(wifi_ip_callback, NULL);
    
    // 设置WiFi配置 - 直接连接，不通过配置结构体
    
    // 连接WiFi
    ESP_LOGI(TAG, "连接WiFi: %s", WIFI_SSID);
    esp_err_t ret = wifi_manager_connect(WIFI_SSID, WIFI_PASSWORD);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi连接失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 WS2812B + WiFi 项目启动");
    ESP_LOGI(TAG, "芯片型号: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "ESP-IDF版本: %s", esp_get_idf_version());
    
    // 初始化NVS
    ESP_ERROR_CHECK(init_nvs());
    
    // 初始化WiFi
    ESP_ERROR_CHECK(init_wifi());
    
    // 初始化WS2812B驱动
    ESP_LOGI(TAG, "初始化WS2812B驱动，使用GPIO: %d", WS2812B_GPIO_PIN);
    esp_err_t ret = ws2812b_init(WS2812B_GPIO_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B驱动初始化失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "WS2812B驱动初始化成功！");
    
    // 创建WiFi监控任务
    BaseType_t wifi_task_created = xTaskCreate(
        wifi_monitor_task,        // 任务函数
        "wifi_monitor",           // 任务名称
        4096,                     // 堆栈大小
        NULL,                     // 任务参数
        4,                        // 任务优先级
        &wifi_monitor_task_handle // 任务句柄
    );
    
    if (wifi_task_created != pdPASS) {
        ESP_LOGE(TAG, "创建WiFi监控任务失败");
        return;
    }
    
    // 主任务可以在这里添加其他功能
    while (1) {
        // 主任务保持运行
        vTaskDelay(pdMS_TO_TICKS(30000));
        
        // 每30秒打印一次系统状态
        ESP_LOGI(TAG, "系统运行中... 可用堆内存: %d bytes", esp_get_free_heap_size());
        
        // 检查WiFi状态
        if (wifi_manager_is_connected()) {
            ESP_LOGI(TAG, "WiFi状态: 已连接 | IP: %s", wifi_manager_get_ip_string());
        } else {
            ESP_LOGW(TAG, "WiFi状态: 未连接");
        }
    }
}