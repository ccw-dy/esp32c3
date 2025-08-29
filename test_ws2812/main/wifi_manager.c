#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>

static const char *TAG = "WIFI_MANAGER";

// 事件组位定义
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1
#define WIFI_GOT_IP_BIT       BIT2

// 全局变量
static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t *s_sta_netif = NULL;
static esp_netif_t *s_ap_netif = NULL;
static wifi_manager_config_t s_wifi_config = {0};
static wifi_info_t s_wifi_info = {0};
static wifi_event_callback_t s_event_callback = NULL;
static wifi_ip_callback_t s_ip_callback = NULL;
static void *s_user_data = NULL;
static bool s_manager_initialized = false;
static int s_retry_num = 0;

// 内部函数声明
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data);
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
static void wifi_task(void *pvParameters);

// WiFi事件处理函数
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi站点模式启动");
        s_wifi_info.state = WIFI_STATE_CONNECTING;
        esp_wifi_connect();
        
        if (s_event_callback) {
            s_event_callback(s_wifi_info.state, s_user_data);
        }
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi连接断开");
        s_wifi_info.state = WIFI_STATE_DISCONNECTED;
        memset(s_wifi_info.ssid, 0, sizeof(s_wifi_info.ssid));
        memset(&s_wifi_info.ip_addr, 0, sizeof(s_wifi_info.ip_addr));
        
        // 延迟重连，避免立即重连导致的问题
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        if (s_retry_num < s_wifi_config.max_retry) {
            ESP_LOGI(TAG, "重试连接WiFi... (%d/%d)", s_retry_num + 1, s_wifi_config.max_retry);
            esp_err_t ret = esp_wifi_connect();
            if (ret == ESP_OK) {
                s_retry_num++;
            } else {
                ESP_LOGE(TAG, "重连失败: %s", esp_err_to_name(ret));
            }
        } else {
            ESP_LOGE(TAG, "WiFi连接失败，达到最大重试次数");
            s_wifi_info.state = WIFI_STATE_FAILED;
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        
        if (s_event_callback) {
            s_event_callback(s_wifi_info.state, s_user_data);
        }
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "WiFi连接成功");
        s_wifi_info.state = WIFI_STATE_CONNECTED;
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            strncpy(s_wifi_info.ssid, (char*)ap_info.ssid, sizeof(s_wifi_info.ssid) - 1);
            s_wifi_info.rssi = ap_info.rssi;
            s_wifi_info.auth_mode = ap_info.authmode;
            s_wifi_info.channel = ap_info.primary;
        }
        
        if (s_event_callback) {
            s_event_callback(s_wifi_info.state, s_user_data);
        }
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "WiFi AP模式启动");
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "WiFi AP模式停止");
    }
}

// IP事件处理函数
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "获取到IP地址: " IPSTR, IP2STR(&event->ip_info.ip));
        
        s_wifi_info.ip_addr = event->ip_info.ip;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        
        if (s_ip_callback) {
            char ip_str[16];
            snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
            s_ip_callback(ip_str, s_user_data);
        }
    }
}

// WiFi任务函数
static void wifi_task(void *pvParameters)
{
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_GOT_IP_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               pdMS_TO_TICKS(1000));
        
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi连接成功");
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG, "WiFi连接失败");
            xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        } else if (bits & WIFI_GOT_IP_BIT) {
            ESP_LOGI(TAG, "获取到IP地址");
            xEventGroupClearBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 初始化WiFi管理器
esp_err_t wifi_manager_init(void)
{
    if (s_manager_initialized) {
        ESP_LOGW(TAG, "WiFi管理器已经初始化");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "初始化WiFi管理器");
    
    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "创建事件组失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 初始化TCP/IP适配器
    ESP_ERROR_CHECK(esp_netif_init());
    
    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 创建默认网络接口
    s_sta_netif = esp_netif_create_default_wifi_sta();
    s_ap_netif = esp_netif_create_default_wifi_ap();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                       ESP_EVENT_ANY_ID,
                                                       &wifi_event_handler,
                                                       NULL,
                                                       NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                       IP_EVENT_STA_GOT_IP,
                                                       &ip_event_handler,
                                                       NULL,
                                                       NULL));
    
    // 设置默认配置
    strncpy(s_wifi_config.ssid, WIFI_DEFAULT_SSID, sizeof(s_wifi_config.ssid) - 1);
    strncpy(s_wifi_config.password, WIFI_DEFAULT_PASSWORD, sizeof(s_wifi_config.password) - 1);
    s_wifi_config.max_retry = WIFI_DEFAULT_MAX_RETRY;
    s_wifi_config.timeout_ms = WIFI_DEFAULT_TIMEOUT_MS;
    
    // 初始化WiFi信息
    memset(&s_wifi_info, 0, sizeof(s_wifi_info));
    s_wifi_info.state = WIFI_STATE_DISCONNECTED;
    
    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // 创建WiFi任务
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    
    s_manager_initialized = true;
    ESP_LOGI(TAG, "WiFi管理器初始化完成");
    
    return ESP_OK;
}

// 反初始化WiFi管理器
esp_err_t wifi_manager_deinit(void)
{
    if (!s_manager_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "反初始化WiFi管理器");
    
    // 停止WiFi
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    
    // 删除网络接口
    if (s_sta_netif) {
        esp_netif_destroy(s_sta_netif);
        s_sta_netif = NULL;
    }
    if (s_ap_netif) {
        esp_netif_destroy(s_ap_netif);
        s_ap_netif = NULL;
    }
    
    // 删除事件组
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }
    
    s_manager_initialized = false;
    ESP_LOGI(TAG, "WiFi管理器反初始化完成");
    
    return ESP_OK;
}

// 连接WiFi
esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (!s_manager_initialized) {
        ESP_LOGE(TAG, "WiFi管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!ssid || !password) {
        ESP_LOGE(TAG, "SSID或密码为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查WiFi状态，如果正在连接则先断开
    if (s_wifi_info.state == WIFI_STATE_CONNECTING || s_wifi_info.state == WIFI_STATE_CONNECTED) {
        ESP_LOGI(TAG, "WiFi正在连接或已连接，先断开连接");
        esp_wifi_disconnect();
        vTaskDelay(pdMS_TO_TICKS(100)); // 等待断开完成
    }
    
    ESP_LOGI(TAG, "连接WiFi: %s", ssid);
    
    // 配置WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    // 设置WiFi配置
    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置WiFi配置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 清除事件位
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_GOT_IP_BIT);
    
    // 连接WiFi
    ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "连接WiFi失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_wifi_info.state = WIFI_STATE_CONNECTING;
    s_retry_num = 0;
    
    if (s_event_callback) {
        s_event_callback(s_wifi_info.state, s_user_data);
    }
    
    return ESP_OK;
}

// 断开WiFi连接
esp_err_t wifi_manager_disconnect(void)
{
    if (!s_manager_initialized) {
        ESP_LOGE(TAG, "WiFi管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "断开WiFi连接");
    
    s_wifi_info.state = WIFI_STATE_DISCONNECTING;
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    
    if (s_event_callback) {
        s_event_callback(s_wifi_info.state, s_user_data);
    }
    
    return ESP_OK;
}

// 重新连接WiFi
esp_err_t wifi_manager_reconnect(void)
{
    if (!s_manager_initialized) {
        ESP_LOGE(TAG, "WiFi管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "重新连接WiFi");
    
    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    return ESP_OK;
}

// 设置WiFi配置
esp_err_t wifi_manager_set_config(const wifi_manager_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_wifi_config, config, sizeof(wifi_manager_config_t));
    ESP_LOGI(TAG, "WiFi配置已更新: SSID=%s, 重试次数=%d, 超时=%dms", 
              s_wifi_config.ssid, s_wifi_config.max_retry, s_wifi_config.timeout_ms);
    
    return ESP_OK;
}

// 获取WiFi配置
esp_err_t wifi_manager_get_config(wifi_manager_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_wifi_config, sizeof(wifi_manager_config_t));
    return ESP_OK;
}

// 获取WiFi信息
esp_err_t wifi_manager_get_info(wifi_info_t *info)
{
    if (!info) {
        ESP_LOGE(TAG, "信息参数为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(info, &s_wifi_info, sizeof(wifi_info_t));
    return ESP_OK;
}

// 获取WiFi状态
wifi_state_t wifi_manager_get_state(void)
{
    return s_wifi_info.state;
}

// 检查WiFi是否已连接
bool wifi_manager_is_connected(void)
{
    return (s_wifi_info.state == WIFI_STATE_CONNECTED);
}

// 获取IP地址字符串
const char* wifi_manager_get_ip_string(void)
{
    static char ip_str[16];
    if (s_wifi_info.state == WIFI_STATE_CONNECTED) {
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&s_wifi_info.ip_addr));
        return ip_str;
    }
    return "未连接";
}

// 设置事件回调函数
esp_err_t wifi_manager_set_event_callback(wifi_event_callback_t callback, void *user_data)
{
    s_event_callback = callback;
    s_user_data = user_data;
    return ESP_OK;
}

// 设置IP回调函数
esp_err_t wifi_manager_set_ip_callback(wifi_ip_callback_t callback, void *user_data)
{
    s_ip_callback = callback;
    s_user_data = user_data;
    return ESP_OK;
}

// 使用默认配置连接WiFi
esp_err_t wifi_manager_connect_default(void)
{
    return wifi_manager_connect(s_wifi_config.ssid, s_wifi_config.password);
}

// 启动AP模式
esp_err_t wifi_manager_start_ap(const char *ssid, const char *password, uint8_t channel)
{
    if (!s_manager_initialized) {
        ESP_LOGE(TAG, "WiFi管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!ssid || !password) {
        ESP_LOGE(TAG, "SSID或密码为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "启动AP模式: %s, 信道: %d", ssid, channel);
    
    // 配置AP
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ssid),
            .channel = channel,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false
            },
        },
    };
    
    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
    
    // 设置AP配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    
    // 启动AP
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    return ESP_OK;
}

// 停止AP模式
esp_err_t wifi_manager_stop_ap(void)
{
    if (!s_manager_initialized) {
        ESP_LOGE(TAG, "WiFi管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "停止AP模式");
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    return ESP_OK;
}
