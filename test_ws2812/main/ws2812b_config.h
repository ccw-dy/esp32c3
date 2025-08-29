#ifndef WS2812B_CONFIG_H
#define WS2812B_CONFIG_H

// ============================================================================
// WS2812B 配置参数 - 用户可以根据需要修改
// ============================================================================

// GPIO引脚配置
#define WS2812B_GPIO_PIN        GPIO_NUM_10    // 数据引脚，可根据实际连接修改

// LED数量配置
#define WS2812B_LED_COUNT       1             // LED数量，当前为1个

// 时序参数配置（单位：纳秒）
#define WS2812B_T0H_NS         350           // 0码高电平时间
#define WS2812B_T0L_NS         800           // 0码低电平时间
#define WS2812B_T1H_NS         700           // 1码高电平时间
#define WS2812B_T1L_NS         600           // 1码低电平时间
#define WS2812B_RESET_TIME_US  280           // 复位时间（微秒）

// RMT外设配置
#define WS2812B_RMT_RESOLUTION_HZ  10000000  // RMT分辨率：10MHz
#define WS2812B_RMT_MEM_BLOCK_SYMBOLS  64    // RMT内存块符号数
#define WS2812B_RMT_TRANS_QUEUE_DEPTH  4     // 传输队列深度

// 测试效果配置
#define WS2812B_TEST_DELAY_MS      1000      // 基本颜色测试间隔（毫秒）
#define WS2812B_RAINBOW_DELAY_MS  50         // 彩虹效果间隔（毫秒）
#define WS2812B_FADE_DELAY_MS     20         // 渐变效果间隔（毫秒）
#define WS2812B_BLINK_DELAY_MS    200        // 闪烁效果间隔（毫秒）
#define WS2812B_BREATH_STEP       5          // 呼吸灯步进值
#define WS2812B_BREATH_DELAY_MS   30         // 呼吸灯间隔（毫秒）

// 颜色配置
#define WS2812B_DEFAULT_BRIGHTNESS  255      // 默认亮度（0-255）
#define WS2812B_COLOR_ORDER_GRB    1         // 颜色顺序：1=GRB（标准），0=RGB

// 调试配置
#define WS2812B_DEBUG_ENABLE       1         // 启用调试输出：1=启用，0=禁用
#define WS2812B_LOG_LEVEL          ESP_LOG_INFO  // 日志级别

// ============================================================================
// 高级配置（一般不需要修改）
// ============================================================================

// 任务配置
#define WS2812B_TASK_STACK_SIZE    4096      // 测试任务堆栈大小
#define WS2812B_TASK_PRIORITY      5         // 测试任务优先级

// 内存配置
#define WS2812B_MAX_COLORS         256       // 最大颜色数量
#define WS2812B_COLOR_BUFFER_SIZE  (WS2812B_LED_COUNT * 3)  // 颜色缓冲区大小

// 错误处理配置
#define WS2812B_MAX_RETRY_COUNT    3         // 最大重试次数
#define WS2812B_TIMEOUT_MS         1000      // 超时时间（毫秒）

// ============================================================================
// 配置验证
// ============================================================================

#if WS2812B_LED_COUNT <= 0
#error "WS2812B_LED_COUNT 必须大于0"
#endif

#if WS2812B_GPIO_PIN < 0 || WS2812B_GPIO_PIN > 21
#error "WS2812B_GPIO_PIN 必须在0-21范围内"
#endif

#if WS2812B_DEFAULT_BRIGHTNESS > 255
#error "WS2812B_DEFAULT_BRIGHTNESS 不能超过255"
#endif

// ============================================================================
// 配置说明
// ============================================================================

/*
配置说明：

1. GPIO引脚选择：
   - 避免使用GPIO0（启动模式选择）
   - 避免使用GPIO1、GPIO3（串口）
   - 推荐使用GPIO2、GPIO4、GPIO5等

2. 时序参数：
   - 这些参数基于WS2812B标准规格
   - 如果LED显示异常，可能需要微调这些参数
   - 单位：ns（纳秒）

3. LED数量：
   - 当前配置为1个LED
   - 如需控制多个LED，修改WS2812B_LED_COUNT
   - 注意：LED数量越多，需要的内存越多

4. 测试效果：
   - 可以调整各种效果的延迟时间
   - 呼吸灯步进值越小，效果越平滑

5. 颜色顺序：
   - WS2812B标准使用GRB顺序
   - 如果颜色显示错误，可能需要调整此参数
*/

#endif // WS2812B_CONFIG_H



