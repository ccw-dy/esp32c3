#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

// WiFi配置结构体
typedef struct {
    char ssid[32];           // WiFi名称
    char password[64];       // WiFi密码
    uint8_t max_retry;       // 最大重试次数
    uint32_t timeout_ms;     // 连接超时时间（毫秒）
} wifi_manager_config_t;

// WiFi状态枚举
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,    // 未连接
    WIFI_STATE_CONNECTING,          // 连接中
    WIFI_STATE_CONNECTED,           // 已连接
    WIFI_STATE_FAILED,              // 连接失败
    WIFI_STATE_DISCONNECTING        // 断开连接中
} wifi_state_t;

// WiFi信息结构体
typedef struct {
    wifi_state_t state;             // 当前状态
    char ssid[32];                  // 当前连接的SSID
    int8_t rssi;                    // 信号强度
    wifi_auth_mode_t auth_mode;     // 认证模式
    uint8_t channel;                // 信道
    esp_ip4_addr_t ip_addr;        // IP地址
} wifi_info_t;

// 回调函数类型定义
typedef void (*wifi_event_callback_t)(wifi_state_t state, void *user_data);
typedef void (*wifi_ip_callback_t)(const char *ip_addr, void *user_data);

// 函数声明
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_deinit(void);
esp_err_t wifi_manager_connect(const char *ssid, const char *password);
esp_err_t wifi_manager_disconnect(void);
esp_err_t wifi_manager_reconnect(void);
esp_err_t wifi_manager_set_config(const wifi_manager_config_t *config);
esp_err_t wifi_manager_get_config(wifi_manager_config_t *config);
esp_err_t wifi_manager_get_info(wifi_info_t *info);
wifi_state_t wifi_manager_get_state(void);
bool wifi_manager_is_connected(void);
const char* wifi_manager_get_ip_string(void);
esp_err_t wifi_manager_set_event_callback(wifi_event_callback_t callback, void *user_data);
esp_err_t wifi_manager_set_ip_callback(wifi_ip_callback_t callback, void *user_data);

// 便捷函数
esp_err_t wifi_manager_connect_default(void);
esp_err_t wifi_manager_start_ap(const char *ssid, const char *password, uint8_t channel);
esp_err_t wifi_manager_stop_ap(void);

// 默认配置
#define WIFI_DEFAULT_SSID        "YourWiFiSSID"
#define WIFI_DEFAULT_PASSWORD    "YourWiFiPassword"
#define WIFI_DEFAULT_MAX_RETRY   5
#define WIFI_DEFAULT_TIMEOUT_MS  10000

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
