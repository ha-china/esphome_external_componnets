#pragma once

#include <string>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/automation.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#include "pdulib.h"

#define MAX_CONCAT_PARTS 10
#define MAX_CONCAT_MESSAGES 5

namespace esphome::ml307r {

// 长短信分段结构
struct SmsPart {
  bool valid = false;
  std::string text;
};

// 长短信缓存结构
struct ConcatSms {
  bool in_use = false;
  int ref_number = 0;
  std::string sender;
  std::string timestamp;
  int total_parts = 0;
  int received_parts = 0;
  uint32_t first_part_time = 0;
  SmsPart parts[MAX_CONCAT_PARTS];  // 最多10个分段
};


inline std::string trim_crlf(const std::string& str) {
  // 要删除的字符集合
  const std::string whitespace = "\r\n";

  // 找到第一个不是 \r 或 \n 的字符位置
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos) {
    return "";  // 字符串全是空白字符
  }

  // 找到最后一个不是 \r 或 \n 的字符位置
  size_t end = str.find_last_not_of(whitespace);

  // 返回子字符串
  return str.substr(start, end - start + 1);
}

class ML307RComponent : public PollingComponent, public uart::UARTDevice {
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(sms_sender)
  SUB_TEXT_SENSOR(sms_content)
  SUB_TEXT_SENSOR(sms_timestamp)
  SUB_TEXT_SENSOR(network_status)
  SUB_TEXT_SENSOR(version)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(signal_strength)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(online)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(soft_shutdown)
  SUB_BUTTON(save_shutdown)
  SUB_BUTTON(soft_reboot)
  SUB_BUTTON(hard_reboot)
#endif
 public:
  // 初始化
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  // AT 命令相关
  bool send_at_command(const std::string &cmd, uint32_t timeout_ms = 1000);
  bool send_at_and_wait_ok(const std::string &cmd, uint32_t timeout_ms = 1000);

  // 短信功能
  bool send_sms(const std::string &phone_number, const std::string &message);
  void shutdown(uint8_t status);
  void reboot(uint8_t status);
  void version();

  // 查询功能
  void query_signal_strength();
  void query_network_status();
  void ping(const std::string &host = "8.8.8.8");

 protected:
  // 状态变量
  bool waiting_for_pdu_{false};
  std::string line_buffer_;

  // 长短信缓存
  ConcatSms concat_buffer_[MAX_CONCAT_MESSAGES];

  // 统计
  uint32_t sms_received_count_{0};
  uint32_t sms_sent_count_{0};

  // 内部方法
  void process_line_(const std::string &line);
  void process_pdu_(const std::string &pdu_data);
  void process_sms_content_(const std::string &sender, const std::string &message, const std::string &timestamp);

  // PDU 解码相关
  std::string decode_pdu_(const std::string &pdu);
  std::string decode_gsm7_(const uint8_t *data, size_t len);
  std::string decode_ucs2_(const uint8_t *data, size_t len);
  int hex_to_int_(char c);
  uint8_t hex_pair_to_byte_(char h, char l);

  // 长短信处理
  int find_or_create_concat_slot_(int ref_number, const std::string &sender, int total_parts);
  std::string assemble_concat_sms_(int slot);
  void clear_concat_slot_(int slot);
  void check_concat_timeout_();

  // 时间戳格式化
  std::string format_timestamp_(const std::string &pdu_timestamp);

  // 辅助函数
  static bool is_hex_string_(const std::string &str);
  std::string read_line_();

  PDU pdu_;

};

template<typename... Ts> class SendAtCommandAction : public Action<Ts...> {
 public:
  SendAtCommandAction(ML307RComponent *ml307r) : ml307r_(ml307r) {}
  TEMPLATABLE_VALUE(std::string, cmd)
  void play(const Ts &...x) override { this->ml307r_->send_at_command(this->cmd_.value(x...)); }

 protected:
  ML307RComponent *ml307r_;
};

template<typename... Ts> class PingAction : public Action<Ts...> {
 public:
  PingAction(ML307RComponent *ml307r) : ml307r_(ml307r) {}
  TEMPLATABLE_VALUE(std::string, host)
  void play(const Ts &...x) override { this->ml307r_->ping(this->host_.value(x...)); }

 protected:
  ML307RComponent *ml307r_;
};

} // namespace esphome::ml307r

