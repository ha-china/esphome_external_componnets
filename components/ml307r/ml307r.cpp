#include "ml307r.h"
#include "esphome/core/log.h"

namespace esphome::ml307r {

static const char *const TAG = "ml307r";

// 长短信超时时间 (30秒)
static const uint32_t CONCAT_TIMEOUT_MS = 30000;

void ML307RComponent::setup() {
  ESP_LOGI(TAG, "Init ML307R 4G module...");

  // 清空长短信缓存
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    this->concat_buffer_[i].in_use = false;
    this->concat_buffer_[i].received_parts = 0;
  }

  // 等待模块启动
  delay(2000);

  // 发送 AT 测试
  int retry = 0;
  while (!this->send_at_and_wait_ok("AT", 1000) && retry < 10) {
    ESP_LOGW(TAG, "AT not response，retrying...");
    delay(500);
    retry++;
  }

  if (retry >= 10) {
    ESP_LOGE(TAG, "ML307R no response");
    this->mark_failed();
#ifdef USE_BINARY_SENSOR
    if (this->online_binary_sensor_ != nullptr) {
      this->online_binary_sensor_->publish_state(false);
    }
#endif
    return;
  }

  ESP_LOGI(TAG, "ML307R AT response ok");

  // 重置 APN（自动识别）
  this->send_at_and_wait_ok("AT+CGDCONT=1,\"IP\",\"\"", 2000);

  // 设置短信自动上报模式
  if (!this->send_at_and_wait_ok("AT+CNMI=2,2,0,0,0", 2000)) {
    ESP_LOGW(TAG, "AT+CNMI failed");
  }

  // 设置 PDU 模式
  if (!this->send_at_and_wait_ok("AT+CMGF=0", 2000)) {
    ESP_LOGW(TAG, "AT+CMGF failed");
  }

  // 等待网络附着
  retry = 0;
  bool attached = false;
  while (!attached && retry < 30) {
    this->send_at_command("AT+CGATT?");
    delay(1000);
    std::string response = this->read_line_();
    if (response.find("+CGATT: 1") != std::string::npos) {
      attached = true;
    }
    retry++;
  }

  if (attached) {
    ESP_LOGI(TAG, "network attached");
  } else {
    ESP_LOGW(TAG, "network attached timeout");
  }

  if (this->online_binary_sensor_ != nullptr) {
    this->online_binary_sensor_->publish_state(true);
  }
  this->version();
}

void ML307RComponent::loop() {
  // 读取 UART 数据
  while (this->available()) {
    char c = this->read();

    if (c == '\n') {
      // 处理完整的一行
      if (!this->line_buffer_.empty() && this->line_buffer_.back() == '\r') {
        this->line_buffer_.pop_back();
      }
      if (!this->line_buffer_.empty()) {
        this->process_line_(this->line_buffer_);
      }
      this->line_buffer_.clear();
    } else if (c != '\r') {
      this->line_buffer_ += c;
      // 防止缓冲区溢出
      if (this->line_buffer_.length() > 500) {
        this->line_buffer_.clear();
      }
    }
  }

  // 检查长短信超时
  this->check_concat_timeout_();
}

void ML307RComponent::update() {
  this->query_signal_strength();
}

void ML307RComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ML307R:\n"
                     "    send sms %d\n"
                     "    receive sms %d", this->sms_sent_count_, this->sms_received_count_);
#ifdef USE_SENSOR
  LOG_SENSOR(" ", "Signal Strength", this->signal_strength_sensor_);
#endif
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR(" ", "SMS Content", this->sms_content_text_sensor_);
  LOG_TEXT_SENSOR(" ", "SMS Sender", this->sms_sender_text_sensor_);
  LOG_TEXT_SENSOR(" ", "SMS Timestamp", this->sms_timestamp_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Network Status", this->network_status_text_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR(" ", "Online", this->online_binary_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON(" ", "Soft Shutdown", this->soft_shutdown_button_);
  LOG_BUTTON(" ", "Save Shutdown", this->save_shutdown_button_);
  LOG_BUTTON(" ", "Soft Reboot", this->soft_reboot_button_);
  LOG_BUTTON(" ", "Hard Reboot", this->hard_reboot_button_);
#endif

}

void ML307RComponent::process_line_(const std::string &line) {
  ESP_LOGD(TAG, "收到: %s", line.c_str());

  if (this->waiting_for_pdu_) {
    // 等待 PDU 数据
    if (this->is_hex_string_(line)) {
      this->process_pdu_(line);
    } else {
      ESP_LOGW(TAG, "预期 PDU 数据，但收到: %s", line.c_str());
    }
    this->waiting_for_pdu_ = false;
  } else if (line.rfind("+CMT:", 0) == 0) {
    // 收到短信通知，等待 PDU 数据
    ESP_LOGI(TAG, "检测到 +CMT，等待 PDU 数据...");
    this->waiting_for_pdu_ = true;
  } else if (line.rfind("+CSQ:", 0) == 0) {
    // 信号强度响应: +CSQ: rssi,ber
    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string value = line.substr(pos + 2);
      size_t comma = value.find(',');
      if (comma != std::string::npos) {
        int rssi = atoi(value.substr(0, comma).c_str());
        // 转换为 dBm: rssi * 2 - 113
        if (rssi < 99) {
          int dbm = rssi * 2 - 113;
#ifdef USE_SENSOR
          if (this->signal_strength_sensor_ != nullptr) {
            this->signal_strength_sensor_->publish_state(dbm);
          }
#endif
        }
      }
    }
  } else if (line.rfind("+CGATT:", 0) == 0) {
    // 网络附着状态
    bool attached = line.find("1") != std::string::npos;
#ifdef USE_TEXT_SENSOR
    if (this->network_status_text_sensor_ != nullptr) {
      this->network_status_text_sensor_->publish_state(attached ? "attached" : "unattached");
    }
#endif
  }
}

bool ML307RComponent::send_at_command(const std::string &cmd, uint32_t timeout_ms) {
  // 清空接收缓冲
  while (this->available()) {
    this->read();
  }

  // 发送命令
  this->write_str((cmd + "\r").c_str());
  this->flush();
  return true;
}

bool ML307RComponent::send_at_and_wait_ok(const std::string &cmd, uint32_t timeout_ms) {
  this->send_at_command(cmd, timeout_ms);

  uint32_t start = millis();
  std::string response;

  while ((millis() - start) < timeout_ms) {
    while (this->available()) {
      char c = this->read();
      response += c;
    }
    if (response.find("OK") != std::string::npos) {
      return true;
    }
    if (response.find("ERROR") != std::string::npos) {
      return false;
    }
    delay(10);
  }
  ESP_LOGW(TAG, "timeout waiting for OK response to %s", cmd.c_str());
  return false;
}

bool ML307RComponent::send_sms(const std::string &phone_number, const std::string &message) {
  ESP_LOGI(TAG, "send sms to %s", phone_number.c_str());

  // 切换到 Text 模式 (简化实现)
  // if (!this->send_at_and_wait_ok("AT+CMGF=1", 1000)) {
  //   ESP_LOGE(TAG, "switch to text mode fail");
  //   return false;
  // }
  // 使用 pdulib 编码 PDU
  this->pdu_.setSCAnumber();  // 使用默认短信中心
  int pduLen = this->pdu_.encodePDU(phone_number.c_str(), message.c_str());
  if (pduLen < 0) {
    ESP_LOGW(TAG, "pdu encode error %d", pduLen);
    return false;
  }

  // 发送 CMGS 命令
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CMGS=%d", pduLen);
  this->send_at_command(cmd);

  // 等待 > 提示符
  uint32_t start = millis();
  bool got_prompt = false;
  while (millis() - start < 5000) {
    if (this->available()) {
      char c = this->read();
      if (c == '>') {
        got_prompt = true;
        break;
      }
    }
  }

  if (!got_prompt) {
    ESP_LOGE(TAG, "timeout receive > prompt");
    return false;
  }

  // 发送消息内容
  this->write_str(this->pdu_.getSMS());
  this->write_byte(0x1A);  // Ctrl+Z

  // 等待 OK
  bool success = false;
  start = millis();
  std::string response;
  while (millis() - start < 30000) {
    while (this->available()) {
      response += (char) this->read();
      if (response.find("OK") != std::string::npos) {
        success = true;
        break;
      }
      if (response.find("ERROR") != std::string::npos) {
        break;
      }
    }
    delay(10);
  }
  if (success) {
    this->sms_sent_count_++;
    ESP_LOGI(TAG, "sms sent successful");
  } else {
    ESP_LOGE(TAG, "sms sent failed");
  }

  return success;
}

void ML307RComponent::shutdown(uint8_t status) { this->send_at_and_wait_ok("AT+MPOF=" + std::to_string(status), 5000); }

void ML307RComponent::reboot(uint8_t status) { this->send_at_and_wait_ok("AT+MREBOOT="+ std::to_string(status),5000); }

void ML307RComponent::version() {
  this->send_at_command("AT+CGMR");
  std::string response;
  while (true) {
    while (this->available()) {
      char c = this->read();
      response += c;
    }
    if (response.find("OK") != std::string::npos) {
      size_t end = trim_crlf(response).find('\n');
      if (end != std::string::npos) {
#ifdef USE_TEXT_SENSOR
        if (this->version_text_sensor_ != nullptr) {
          this->version_text_sensor_->publish_state(response.substr(0, end));
        }
#endif
      }
      break;
    }
    if (response.find("ERROR") != std::string::npos) {
      break;
    }
    delay(10);
  }
}

void ML307RComponent::query_signal_strength() { this->send_at_command("AT+CSQ"); }

void ML307RComponent::query_network_status() { this->send_at_command("AT+CGATT?"); }

void ML307RComponent::ping(const std::string &host) {
  // ESP_LOGI(TAG, "执行 Ping 测试: %s", host.c_str());
  if (!this->send_at_and_wait_ok("AT+CGACT=1,1", 10000)) {
    ESP_LOGW(TAG, "CGACT fail");
    return;
  }

  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+MPING=1,\"%s\",4,32,255", host.c_str());
  this->send_at_and_wait_ok(cmd, 30000);

  delay(5000);
  this->send_at_and_wait_ok("AT+CGACT=0,1", 5000);
}

// ============ PDU 解码相关 ============

void ML307RComponent::process_pdu_(const std::string &pdu_data) {
  if (!this->pdu_.decodePDU(pdu_data.c_str())) {
    ESP_LOGW(TAG, "PDU decode error");
    return;
  }

  int* concat_info = this->pdu_.getConcatInfo();
  int ref_number = concat_info[0];
  int part_number = concat_info[1];
  int total_parts = concat_info[2];

  if (total_parts > 1 && part_number > 0) {
    // 这是长短信的一部分
    // 查找或创建缓存槽位
    int slot = this->find_or_create_concat_slot_(ref_number, this->pdu_.getSender(), total_parts);

    // 存储该分段（partNumber 从 1 开始，数组从 0 开始）
    int part_index = part_number - 1;
    if (part_index >= 0 && part_index < MAX_CONCAT_PARTS) {
      if (!this->concat_buffer_[slot].parts[part_index].valid) {
        this->concat_buffer_[slot].parts[part_index].valid = true;
        this->concat_buffer_[slot].parts[part_index].text = std::string(this->pdu_.getText());
        this->concat_buffer_[slot].received_parts++;

        // 如果是第一个收到的分段，保存时间戳
        if (this->concat_buffer_[slot].received_parts == 1) {
          this->concat_buffer_[slot].timestamp = std::string(this->pdu_.getTimeStamp());
        }
      }
    }
    // 检查是否已收齐所有分段
    if (this->concat_buffer_[slot].received_parts >= total_parts) {
      // 合并所有分段
      std::string full_text = this->assemble_concat_sms_(slot);

      // 处理完整短信
      this->process_sms_content_(this->concat_buffer_[slot].sender,
                       full_text,
                       this->concat_buffer_[slot].timestamp);

      // 清空槽位
      this->clear_concat_slot_(slot);
    }
  } else {
    // 普通短信，直接处理
    this->process_sms_content_(std::string(this->pdu_.getSender()), std::string(this->pdu_.getText()),
                            std::string(this->pdu_.getTimeStamp()));
  }
  this->sms_received_count_++;
}

void ML307RComponent::process_sms_content_(const std::string &sender, const std::string &message,
                                           const std::string &timestamp) {
  ESP_LOGI(TAG, "处理短信: 发送者=%s, 时间=%s", sender.c_str(), timestamp.c_str());
  ESP_LOGI(TAG, "内容: %s", message.c_str());

  // 更新传感器
#ifdef USE_TEXT_SENSOR
  if (this->sms_sender_text_sensor_ != nullptr) {
    this->sms_sender_text_sensor_->publish_state(sender);
  }
  if (this->sms_content_text_sensor_ != nullptr) {
    this->sms_content_text_sensor_->publish_state(message);
  }
  if (this->sms_timestamp_text_sensor_ != nullptr) {
    this->sms_timestamp_text_sensor_->publish_state(timestamp);
  }
#endif
}

int ML307RComponent::hex_to_int_(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return 0;
}

uint8_t ML307RComponent::hex_pair_to_byte_(char h, char l) { return (hex_to_int_(h) << 4) | hex_to_int_(l); }

bool ML307RComponent::is_hex_string_(const std::string &str) {
  if (str.empty()) {
    return false;
  }
  for (char c : str) {
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
      return false;
    }
  }
  return true;
}

std::string ML307RComponent::read_line_() {
  std::string line;
  uint32_t start = millis();

  while (millis() - start < 1000) {
    while (this->available()) {
      char c = this->read();
      if (c == '\n') {
        return line;
      } else if (c != '\r') {
        line += c;
      }
    }
    delay(10);
  }

  return line;
}

// ============ 长短信处理 ============

int ML307RComponent::find_or_create_concat_slot_(int ref_number, const std::string &sender, int total_parts) {
  // 查找已存在的槽位
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (this->concat_buffer_[i].in_use && this->concat_buffer_[i].ref_number == ref_number && this->concat_buffer_[i].sender == sender) {
      return i;
    }
  }

  // 查找空闲槽位
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (!this->concat_buffer_[i].in_use) {
      this->concat_buffer_[i].in_use = true;
      this->concat_buffer_[i].ref_number = ref_number;
      this->concat_buffer_[i].sender = sender;
      this->concat_buffer_[i].total_parts = total_parts;
      this->concat_buffer_[i].received_parts = 0;
      this->concat_buffer_[i].first_part_time = millis();
      for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
        this->concat_buffer_[i].parts[j].valid = false;
        this->concat_buffer_[i].parts[j].text.clear();
      }
      return i;
    }
  }

  // 覆盖最老的槽位
  int oldest = 0;
  uint32_t oldest_time = this->concat_buffer_[0].first_part_time;
  for (int i = 1; i < MAX_CONCAT_MESSAGES; i++) {
    if (this->concat_buffer_[i].first_part_time < oldest_time) {
      oldest_time = this->concat_buffer_[i].first_part_time;
      oldest = i;
    }
  }

  this->concat_buffer_[oldest].in_use = true;
  this->concat_buffer_[oldest].ref_number = ref_number;
  this->concat_buffer_[oldest].sender = sender;
  this->concat_buffer_[oldest].total_parts = total_parts;
  this->concat_buffer_[oldest].received_parts = 0;
  this->concat_buffer_[oldest].first_part_time = millis();
  for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
    this->concat_buffer_[oldest].parts[j].valid = false;
    this->concat_buffer_[oldest].parts[j].text.clear();
  }

  return oldest;
}

std::string ML307RComponent::assemble_concat_sms_(int slot) {
  std::string result;
  for (int i = 0; i < this->concat_buffer_[slot].total_parts; i++) {
    if (this->concat_buffer_[slot].parts[i].valid) {
      result += this->concat_buffer_[slot].parts[i].text;
    } else {
      result += "[missing seg " + std::to_string(i + 1) + "]";
    }
  }
  return result;
}

void ML307RComponent::clear_concat_slot_(int slot) {
  this->concat_buffer_[slot].in_use = false;
  this->concat_buffer_[slot].received_parts = 0;
  this->concat_buffer_[slot].sender.clear();
  this->concat_buffer_[slot].timestamp.clear();
  for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
    this->concat_buffer_[slot].parts[j].valid = false;
    this->concat_buffer_[slot].parts[j].text.clear();
  }
}

void ML307RComponent::check_concat_timeout_() {
  uint32_t now = millis();
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (this->concat_buffer_[i].in_use) {
      if (now - this->concat_buffer_[i].first_part_time >= CONCAT_TIMEOUT_MS) {
        ESP_LOGW(TAG, "long sms timeout, force process anyway");

        std::string full_text = this->assemble_concat_sms_(i);
        this->process_sms_content_(this->concat_buffer_[i].sender, full_text, this->concat_buffer_[i].timestamp);
        this->clear_concat_slot_(i);
      }
    }
  }
}

std::string ML307RComponent::format_timestamp_(const std::string &pdu_timestamp) {
  if (pdu_timestamp.length() < 12)
    return pdu_timestamp;

  int year = 2000 + atoi(pdu_timestamp.substr(0, 2).c_str());
  int month = atoi(pdu_timestamp.substr(2, 2).c_str());
  int day = atoi(pdu_timestamp.substr(4, 2).c_str());
  int hour = atoi(pdu_timestamp.substr(6, 2).c_str());
  int minute = atoi(pdu_timestamp.substr(8, 2).c_str());
  int second = atoi(pdu_timestamp.substr(10, 2).c_str());

  char buf[20];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  return std::string(buf);
}

} // namespace esphome::ml307r

