#pragma once

#include <vector>
#include <queue>
#include <functional>
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

namespace esphome::rs200 {

// 帧标识: 数据编号 (低7位), 见数据手册 表 2.4
enum RS200Tag : uint8_t {
  RS200_TAG_VERSION = 0,        // 固件版本
  RS200_TAG_RAIN = 1,           // 雨量状态
  RS200_TAG_SYSTEM_STATUS = 2,  // 系统状态
  RS200_TAG_OPTICAL = 3,        // 光学系统 (写0=执行校准; 主机读取后回送校准值)
  RS200_TAG_REALTIME_RAIN = 4,  // 实时雨量模式 (0=退出 1=进入)
  RS200_TAG_OUTPUT_FREQ = 5,    // 雨量状态输出频率, 0~9, 每单位 50ms
  RS200_TAG_V1 = 6,             // 无雨与小雨的阈值 V1
  RS200_TAG_V2 = 7,             // 小雨与中雨的阈值 V2
  RS200_TAG_V3 = 8,             // 中雨与大雨的阈值 V3
  RS200_TAG_S1 = 9,             // 无雨与小雨的阈值 S1
  RS200_TAG_S2 = 10,            // 小雨与中雨的阈值 S2
  RS200_TAG_S3 = 11,            // 中雨与大雨的阈值 S3
  RS200_TAG_N1 = 12,            // 10 次中判定为大雨的次数阈值 N1
  RS200_TAG_N2 = 13,            // 10 次中判定为中雨的次数阈值 N2
  RS200_TAG_N3 = 14,            // 10 次中判定为小雨的次数阈值 N3
  RS200_TAG_LIGHT = 15,         // 环境光测量模式 (0=退出 1=进入)
  RS200_TAG_TEMPERATURE = 16,   // 模块温度
  RS200_TAG_SLEEP = 17,         // 光学睡眠模式 (0=退出 1=进入)
  RS200_TAG_BOOT = 18,         // RS200向主机发送Boot启动指令
};

class RS200Component : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(temperature)  // 模块温度, 单位 ℃
  SUB_SENSOR(light)        // 环境光反馈值, 范围 0~1024, 数值越小光越强
  SUB_SENSOR(raw_rain)   // 原始雨量数值 uint16_t
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)        // 模块固件版本, 格式 主版本.副本号
  SUB_TEXT_SENSOR(rain)           // 雨量状态: no rain / small rain / medium rain / heavy rain
  SUB_TEXT_SENSOR(system_status)  // 系统状态文字描述
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(output_interval)      // 雨量状态输出频率, 单位 ms
  SUB_NUMBER(v1)                   // 阈值 V1
  SUB_NUMBER(v2)                   // 阈值 V2
  SUB_NUMBER(v3)                   // 阈值 V3
  SUB_NUMBER(s1)                   // 阈值 S1
  SUB_NUMBER(s2)                   // 阈值 S2
  SUB_NUMBER(s3)                   // 阈值 S3
  SUB_NUMBER(n1)                   // 大雨次数阈值 N1
  SUB_NUMBER(n2)                   // 中雨次数阈值 N2
  SUB_NUMBER(n3)                   // 小雨次数阈值 N3
  SUB_NUMBER(optical_calibration)  // 光学系统校准值
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(sleep_mode)          // 光学睡眠模式
  SUB_SWITCH(realtime_rain_mode)  // 实时雨量模式
  SUB_SWITCH(ambient_light_mode)  // 环境光测量模式
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(calibrate)
#endif
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;

  // 发送一帧指令: write=true 表示写属性, tag 为数据编号(低7位), data 为 16bit 帧数据(低位在前)
  void send_command(bool write, uint8_t tag, uint16_t data);

  void set_temperature_update_interval(uint32_t interval) { this->temperature_update_interval_ = interval; }

  // 所有子实体(write_state/control)都通过此队列串行执行, 避免并发往 UART 写数据
  std::queue<std::function<void()>> task_queue_{};

 protected:
  friend class CalibrateButton;
  // 串口接收缓冲, 只在 loop() 里读写
  std::vector<uint8_t> buffer_{};

  // 周期请求模块温度的间隔(ms), 0 表示不自动请求
  uint32_t temperature_update_interval_{60000};

  // 解析完整的一帧: buffer_ 的前 5 字节. 成功处理后从 buffer_ 中移除
  void handle_frame_(uint8_t frame_tag, uint16_t frame_data);

  // 读取所有可配置参数, 在 setup 阶段调用一次, 让 number 实体显示真实值
  void read_all_parameters();

  // 雨量状态 -> 文字
  static const char *rain_state_str_(uint16_t data);
};

template<typename... Ts> class SendCommandAction : public Action<Ts...> {
 public:
  SendCommandAction(RS200Component *rs200) : rs200_(rs200) {}
  TEMPLATABLE_VALUE(bool, write)
  TEMPLATABLE_VALUE(uint8_t, tag)
  TEMPLATABLE_VALUE(uint16_t, data)
  void play(const Ts &...x) override {
    this->rs200_->send_command(this->write_.value(x...), this->tag_.value(x...), this->data_.value(x...));
  }

 protected:
  RS200Component *rs200_;
};

// CRC-8: 多项式 0x31, 初始值 0xFF, MSB First, 异或输出 0x00. 见数据手册 表 2.5
uint8_t rs200_crc8(const uint8_t *dat, uint8_t size);

}  // namespace esphome::rs200
