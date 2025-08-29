#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// ============================================================================
// WiFi 配置参数 - 用户可以根据需要修改
// ============================================================================

// WiFi连接配置
#define WIFI_SSID               "E3A Health"        // 您的WiFi名称
#define WIFI_PASSWORD           "E3A88888888@"    // 您的WiFi密码

// 连接参数配置
#define WIFI_MAX_RETRY          5                     // 最大重试次数
#define WIFI_TIMEOUT_MS         10000                 // 连接超时时间（毫秒）
#define WIFI_RECONNECT_DELAY_MS 5000                  // 重连延迟时间（毫秒）

// AP模式配置（可选）
#define WIFI_AP_SSID            "ESP32-C3-AP"         // AP模式下的WiFi名称
#define WIFI_AP_PASSWORD        "12345678"            // AP模式下的WiFi密码
#define WIFI_AP_CHANNEL         1                     // AP模式下的信道
#define WIFI_AP_MAX_CONN        4                     // AP模式下的最大连接数

// 网络配置
#define WIFI_AUTH_MODE          WIFI_AUTH_WPA2_PSK    // 认证模式
#define WIFI_PMF_REQUIRED       false                 // 是否要求PMF

// 调试配置
#define WIFI_DEBUG_ENABLE       1                     // 启用调试输出：1=启用，0=禁用
#define WIFI_LOG_LEVEL          ESP_LOG_INFO          // 日志级别

// ============================================================================
// 高级配置（一般不需要修改）
// ============================================================================

// 任务配置
#define WIFI_TASK_STACK_SIZE    4096                  // WiFi任务堆栈大小
#define WIFI_TASK_PRIORITY      5                     // WiFi任务优先级

// 事件配置
#define WIFI_EVENT_QUEUE_SIZE   32                    // 事件队列大小
#define WIFI_EVENT_TIMEOUT_MS   1000                  // 事件超时时间

// 内存配置
#define WIFI_MAX_SSID_LEN       32                    // 最大SSID长度
#define WIFI_MAX_PASSWORD_LEN   64                    // 最大密码长度

// ============================================================================
// 配置验证
// ============================================================================

#if WIFI_MAX_RETRY <= 0
#error "WIFI_MAX_RETRY 必须大于0"
#endif

#if WIFI_TIMEOUT_MS <= 0
#error "WIFI_TIMEOUT_MS 必须大于0"
#endif

#if WIFI_AP_CHANNEL < 1 || WIFI_AP_CHANNEL > 13
#error "WIFI_AP_CHANNEL 必须在1-13范围内"
#endif

#if WIFI_AP_MAX_CONN <= 0 || WIFI_AP_MAX_CONN > 10
#error "WIFI_AP_MAX_CONN 必须在1-10范围内"
#endif

// ============================================================================
// 配置说明
// ============================================================================

/*
配置说明：

1. WiFi连接配置：
   - WIFI_SSID: 您的WiFi网络名称
   - WIFI_PASSWORD: 您的WiFi网络密码
   - 请确保这些信息正确，否则无法连接

2. 连接参数：
   - WIFI_MAX_RETRY: 连接失败时的最大重试次数
   - WIFI_TIMEOUT_MS: 连接超时时间，单位毫秒
   - WIFI_RECONNECT_DELAY_MS: 断开连接后的重连延迟

3. AP模式配置：
   - 当ESP32-C3作为WiFi热点时的配置
   - 可以用于配置其他设备或调试

4. 网络配置：
   - WIFI_AUTH_MODE: WiFi认证模式，一般使用WPA2_PSK
   - WIFI_PMF_REQUIRED: 是否要求PMF（Protected Management Frames）

5. 调试配置：
   - WIFI_DEBUG_ENABLE: 是否启用调试输出
   - WIFI_LOG_LEVEL: 日志输出级别

使用步骤：
1. 修改WIFI_SSID和WIFI_PASSWORD为您的WiFi信息
2. 根据需要调整其他参数
3. 编译并烧录程序
4. 观察串口输出，确认WiFi连接状态
*/

#endif // WIFI_CONFIG_H
