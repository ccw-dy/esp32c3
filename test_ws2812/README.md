# ESP32-C3 WS2812B LED控制项目

这是一个基于ESP32-C3芯片的WS2812B RGB LED灯带控制项目，专门为XL-0807RGBC-WS2812B型号设计。

## 🎯 项目特性

- **芯片支持**: ESP32-C3 (RISC-V架构)
- **LED型号**: XL-0807RGBC-WS2812B
- **控制方式**: RMT外设，单线协议
- **LED数量**: 1个（可扩展）
- **颜色深度**: 24位RGB (8位红 + 8位绿 + 8位蓝)

## 🔧 硬件连接

### WS2812B连接方式
```
ESP32-C3          WS2812B
GPIO2    ----->  DIN (数据输入)
3.3V     ----->  VCC (电源)
GND      ----->  GND (地线)
```

**注意**: 
- 默认使用GPIO2，可在`main.c`中修改`WS2812B_GPIO_PIN`宏定义
- 确保电源稳定，WS2812B对电源要求较高
- 数据线建议使用短距离连接，避免干扰

## 📁 项目结构

```
test_ws2812/
├── main/
│   ├── main.c                 # 主程序
│   ├── ws2812b_driver.h      # WS2812B驱动头文件
│   ├── ws2812b_driver.c      # WS2812B驱动实现
│   └── CMakeLists.txt        # 组件构建配置
├── CMakeLists.txt            # 项目构建配置
├── sdkconfig                 # ESP-IDF配置文件
└── README.md                 # 项目说明文档
```

## 🚀 编译和烧录

### 环境要求
- ESP-IDF 5.5.0 或更高版本
- Python 3.7 或更高版本

### 编译步骤
```bash
# 1. 设置ESP-IDF环境
. $HOME/esp/esp-idf/export.sh

# 2. 进入项目目录
cd test_ws2812

# 3. 配置项目
idf.py menuconfig

# 4. 编译项目
idf.py build

# 5. 烧录到ESP32-C3
idf.py -p [PORT] flash monitor
```

### 烧录参数说明
- `[PORT]`: 串口设备，Windows下通常是`COM3`、`COM4`等
- `flash`: 烧录固件
- `monitor`: 启动串口监视器

## 🎨 功能特性

### 基础功能
- ✅ 单LED颜色控制
- ✅ RGB颜色设置
- ✅ 亮度调节
- ✅ 批量操作

### 测试效果
1. **基本颜色测试**: 红、绿、蓝、白、黑
2. **彩虹效果**: 256色渐变循环
3. **渐变效果**: 单色亮度渐变
4. **闪烁效果**: 白色闪烁10次
5. **自定义颜色**: 黄、青、洋红、橙、紫
6. **呼吸灯效果**: 三色呼吸灯循环

### 预定义颜色
```c
WS2812B_COLOR_RED      // 红色
WS2812B_COLOR_GREEN    // 绿色
WS2812B_COLOR_BLUE     // 蓝色
WS2812B_COLOR_WHITE    // 白色
WS2812B_COLOR_BLACK    // 黑色（关闭）
WS2812B_COLOR_YELLOW   // 黄色
WS2812B_COLOR_CYAN     // 青色
WS2812B_COLOR_MAGENTA  // 洋红色
WS2812B_COLOR_ORANGE   // 橙色
WS2812B_COLOR_PURPLE   // 紫色
```

## 📖 API使用说明

### 初始化驱动
```c
esp_err_t ws2812b_init(gpio_num_t gpio_num);
```

### 设置LED颜色
```c
// 设置单个LED
esp_err_t ws2812b_set_pixel(uint16_t pixel_index, ws2812b_color_t color);

// 设置所有LED
esp_err_t ws2812b_set_all_pixels(ws2812b_color_t color);

// 清除所有LED
esp_err_t ws2812b_clear(void);
```

### 刷新显示
```c
esp_err_t ws2812b_refresh(void);
```

### 自定义颜色
```c
ws2812b_color_t my_color = {red, green, blue};
ws2812b_set_all_pixels(my_color);
ws2812b_refresh();
```

## ⚠️ 注意事项

1. **电源要求**: WS2812B需要稳定的3.3V电源，电流约60mA/个
2. **时序要求**: 严格遵循WS2812B协议时序，使用RMT外设确保精度
3. **GPIO选择**: 避免使用启动时的特殊GPIO（如GPIO0、GPIO1等）
4. **干扰防护**: 数据线尽量短，避免与强干扰源靠近
5. **散热考虑**: 长时间高亮度使用时注意散热

## 🔍 故障排除

### 常见问题
1. **LED不亮**: 检查电源、地线、数据线连接
2. **颜色异常**: 检查RGB顺序，WS2812B使用GRB顺序
3. **闪烁不稳定**: 检查电源稳定性，增加滤波电容
4. **编译错误**: 确保ESP-IDF版本兼容，检查依赖库

### 调试方法
- 使用串口监视器查看日志输出
- 检查GPIO配置是否正确
- 验证RMT外设配置
- 测试单个颜色功能

## 📝 扩展功能

### 增加LED数量
修改`ws2812b_driver.h`中的`WS2812B_LED_COUNT`宏定义：
```c
#define WS2812B_LED_COUNT  10  // 改为您需要的数量
```

### 添加新效果
在`ws2812b_driver.c`中添加新的测试函数，并在主程序中调用。

### 网络控制
可以集成WiFi功能，通过HTTP API远程控制LED颜色和效果。

## 📞 技术支持

如果您在使用过程中遇到问题，请：
1. 检查硬件连接
2. 查看串口日志输出
3. 确认ESP-IDF版本兼容性
4. 参考ESP32-C3官方文档

## 📄 许可证

本项目基于MIT许可证开源，您可以自由使用、修改和分发。

---

**祝您使用愉快！** 🎉
