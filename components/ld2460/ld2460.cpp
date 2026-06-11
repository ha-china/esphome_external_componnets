#include "ld2460.h"
#include "esphome/core/log.h"

namespace esphome::ld2460 {

static const char *const TAG = "ld2460";
static uint8_t LD2460_UPLOAD_HEAD[4] = {0xF4, 0xF3, 0xF2, 0xF1};
static uint8_t LD2460_UPLOAD_TAIL[4] = {0xF8, 0xF7, 0xF6, 0xF5};

static uint8_t LD2460_CMD_HEAD[4] = {0xFD, 0xFC, 0xFB, 0xFA};
static uint8_t LD2460_CMD_TAIL[4] = {0x04, 0x03, 0x02, 0x01};

void LD2460Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LD2460...");
  this->restart();
  this->set_install_params(this->height_, this->angle_);
  this->set_mode(this->mode_);
  this->set_detect_range(this->detect_distance_, this->detect_start_angle_, this->detect_end_angle_);
  this->set_sensitivity(this->sensitivity_);
  this->set_timeout(1500, [this]() { this->read_all_info(); });
#ifdef USE_SELECT
  if (this->baud_rate_select_ != nullptr) {
    this->baud_rate_select_->publish_state(std::to_string(this->parent_->get_baud_rate()));
  }
#endif
#ifdef USE_NUMBER
  float value;
  if (this->height_number_ != nullptr) {
    this->pref_height_ = this->height_number_->make_entity_preference<float>();
    if (this->pref_height_.load(&value)) {
      this->task_queue_.emplace([this, value]() { this->height_number_->make_call().set_value(value).perform();});
    } // call number->control publish_state set_xxx save_to_flash later publish_state again
  }
  if (this->angle_number_ != nullptr) {
    this->pref_angle_ = this->angle_number_->make_entity_preference<float>();
    if (this->pref_angle_.load(&value)) {
      this->task_queue_.emplace([this, value]() { this->angle_number_->make_call().set_value(value).perform();});
    }
  }
  if (this->detect_distance_number_ != nullptr) {
    this->pref_detect_distance_ = this->detect_distance_number_->make_entity_preference<float>();
    if (this->pref_detect_distance_.load(&value)) {
      this->task_queue_.emplace([this, value]() { this->detect_distance_number_->make_call().set_value(value).perform();});
    }
  }
  if (this->detect_start_angle_number_ != nullptr) {
    this->pref_detect_start_angle_ = this->detect_start_angle_number_->make_entity_preference<float>();
    if (this->pref_detect_start_angle_.load(&value)) {
      this->task_queue_.emplace([this, value]() { this->detect_start_angle_number_->make_call().set_value(value).perform();});
    }
  }
  if (this->detect_end_angle_number_ != nullptr) {
    this->pref_detect_end_angle_ = this->detect_end_angle_number_->make_entity_preference<float>();
    if (this->pref_detect_end_angle_.load(&value)) {
      this->task_queue_.emplace([this, value]() { this->detect_end_angle_number_->make_call().set_value(value).perform();});
    }
  }
#endif
}

void LD2460Component::save_to_flash() {
#ifdef USE_NUMBER
  if (this->height_number_ != nullptr && this->height_number_->has_state()) {
    this->pref_height_.save(&this->height_number_->state);
  }
  if (this->angle_number_ != nullptr && this->angle_number_->has_state()) {
    this->pref_angle_.save(&this->angle_number_->state);
  }
  if (this->detect_distance_number_ != nullptr && this->detect_distance_number_->has_state()) {
    this->pref_detect_distance_.save(&this->detect_distance_number_->state);
  }
  if (this->detect_start_angle_number_ != nullptr && this->detect_start_angle_number_->has_state()) {
    this->pref_detect_start_angle_.save(&this->detect_start_angle_number_->state);
  }
  if (this->detect_end_angle_number_ != nullptr && this->detect_end_angle_number_->has_state()) {
    this->pref_detect_end_angle_.save(&this->detect_end_angle_number_->state);
  }
#endif
}

void LD2460Component::dump_config() {
  ESP_LOGCONFIG(TAG, "LD2460:");
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "Version", this->version_text_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "Target Number", this->target_number_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Has Target", this->target_binary_sensor_);
#endif
#ifdef USE_NUMBER
  LOG_NUMBER("  ", "Height", this->height_number_);
  LOG_NUMBER("  ", "Angle", this->angle_number_);
  LOG_NUMBER("  ", "Detect Distance", this->detect_distance_number_);
  LOG_NUMBER("  ", "Detect Start Angle", this->detect_start_angle_number_);
  LOG_NUMBER("  ", "Detect End Angle", this->detect_end_angle_number_);
#endif
#ifdef USE_SELECT
  LOG_SELECT("  ", "Mode", this->mode_select_);
  LOG_SELECT("  ", "Baud Rate", this->baud_rate_select_);
  LOG_SELECT("  ", "Sensitivity", this->sensitivity_select_);
#endif
}

void LD2460Component::parse_upload() {
  uint8_t cmd = this->receive_buffer[4];  // should be 0x04
  uint16_t frame_size = (uint16_t) (this->receive_buffer[5]) | (((uint16_t) this->receive_buffer[6]) << 8);
  if (frame_size < 11) {
    this->head_found = false;  // invalid frame_size
    this->receive_buffer.clear();
    return;
  }
  if (this->receive_buffer.size() < frame_size) {
    return;  // not full frame
  }
  if (memcmp(this->receive_buffer.data() + frame_size - 4, LD2460_UPLOAD_TAIL, 4) != 0) {
    this->head_found = false;  // invalid tail
    this->receive_buffer.clear();
    return;
  }
  uint8_t target_num = (frame_size - 11) / 4;  // number of targets
#ifdef USE_SENSOR
  if (this->target_number_sensor_ != nullptr) {
    this->target_number_sensor_->publish_state(target_num);
  }
#endif
#ifdef USE_BINARY_SENSOR
  if (this->target_binary_sensor_ != nullptr) {
    if (target_num > 0) {
      this->target_binary_sensor_->publish_state(true);
    } else {
      this->target_binary_sensor_->publish_state(false);  // this never happens
    }
    this->set_timeout("timeout", 1000, [this]() { this->target_binary_sensor_->publish_state(false); });
  }
#endif
  if (target_num > MAX_TARGETS ) {
    target_num = MAX_TARGETS;  // ignore extra targets
  }
  for (uint8_t i = 0; i < target_num; i++) {
    int16_t x = (int16_t) (this->receive_buffer[7 + i * 4]) |
                (((int16_t) this->receive_buffer[7 + i * 4 + 1]) << 8);  // target x
    int16_t y = (int16_t) (this->receive_buffer[7 + i * 4 + 2]) |
                (((int16_t) this->receive_buffer[7 + i * 4 + 3]) << 8);  // target y
#ifdef USE_SENSOR
    if (this->target_x_sensors_[i] != nullptr) {
      this->target_x_sensors_[i]->publish_state((float) x / 10.0f);
    }
    if (this->target_y_sensors_[i] != nullptr) {
      this->target_y_sensors_[i]->publish_state((float) y / 10.0f);
    }
#endif
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + frame_size);
  this->data_callback_.call();
}

void LD2460Component::parse_ack() {
  uint8_t cmd = this->receive_buffer[4];  // now it is useful
  uint16_t frame_size = (uint16_t) (this->receive_buffer[5]) | (((uint16_t) this->receive_buffer[6]) << 8);
  if (frame_size < 11) {
    this->head_found = false;  // invalid frame_size
    this->receive_buffer.clear();
    return;
  }
  if (this->receive_buffer.size() < frame_size) {
    return;  // not full frame
  }
  if (memcmp(this->receive_buffer.data() + frame_size - 4, LD2460_CMD_TAIL, 4) != 0) {
    this->head_found = false;  // invalid tail
    this->receive_buffer.clear();
    return;
  }
  switch (cmd) {
    case 0x06: {  // 雷达开启/关闭上报功能设置回执
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "disable upload fail");
      } else if (d == 0x01) {
        ESP_LOGW(TAG, "enable upload fail");
      }
      break;
    }
    case 0x07: {  // 设置雷达安装参数回执
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "set install params fail");
      }
      break;
    }
    case 0x08: {  // 雷达安装参数回执协议
      uint16_t height = (uint16_t) this->receive_buffer[7] | (((uint16_t) this->receive_buffer[8]) << 8);  // 安装高度
      uint16_t angle = (uint16_t) this->receive_buffer[9] | (((uint16_t) this->receive_buffer[10]) << 8);  // 安装角度
#ifdef USE_NUMBER
      if (this->height_number_ != nullptr) {
        this->height_number_->publish_state((float) height / 100.0f);
      }
      if (this->angle_number_ != nullptr) {
        this->angle_number_->publish_state((float) angle / 100.0f);
      }
#endif
      break;
    }
    case 0x09: {  // 设置安装模式雷达回执协议
      uint8_t d = this->receive_buffer[7];
      if (d == 0x01) {
        ESP_LOGW(TAG, "set side install mode fail");
      } else if (d == 0x02) {
        ESP_LOGW(TAG, "set top install mode fail");
      }
      break;
    }
    case 0x0A: {  // 雷达安装模式回执指令
      uint8_t mode = this->receive_buffer[7];
#ifdef USE_SELECT
      if (this->mode_select_ != nullptr) {
        if (mode == 0x01) {
          this->mode_select_->publish_state("Side");
        } else if (mode == 0x02) {
          this->mode_select_->publish_state("Top");
        }
      }
#endif
      break;
    }
    case 0x0B: {  // 雷达版本号回执协议
      uint8_t year = this->receive_buffer[8];
      uint8_t month = this->receive_buffer[9];
      uint8_t major = this->receive_buffer[10];
      uint8_t minor = this->receive_buffer[11];
#ifdef USE_TEXT_SENSOR
      if (this->version_text_sensor_ != nullptr) {
        char version[20];
        snprintf(version, sizeof(version), "20%02d-%02d V%d.%d", year, month, major, minor);
        this->version_text_sensor_->publish_state(version);
      }
#endif
      break;
    }
    case 0x0E: {  // 雷达修改波特率回执协议
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "set baud rate fail");
      } else {
        this->parent_->load_settings(false);
      }
      break;
    }
    case 0x10: {  // 雷达恢复出厂设置回执
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "factory reset fail");
      }
      break;
    }
    case 0x11: {  // 雷达设置检测范围回执
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "set detect range fail");
      }
      break;
    }
    case 0x12: {  // 查询检测范围回执
      uint8_t distance = this->receive_buffer[7];
      int16_t start_angle = (int16_t) this->receive_buffer[8] | (((int16_t) this->receive_buffer[9]) << 8);
      int16_t end_angle = (int16_t) this->receive_buffer[10] | (((int16_t) this->receive_buffer[11]) << 8);
#ifdef USE_NUMBER
      if (this->detect_distance_number_ != nullptr) {
        this->detect_distance_number_->publish_state((float) distance / 10.0f);
      }
      if (this->detect_start_angle_number_ != nullptr) {
        this->detect_start_angle_number_->publish_state((float) start_angle / 10.0f);
      }
      if (this->detect_end_angle_number_ != nullptr) {
        this->detect_end_angle_number_->publish_state((float) end_angle / 10.0f);
      }
#endif
      break;
    }
    case 0x13: {  // 灵敏度设置回执
      uint8_t d = this->receive_buffer[7];
      if (d == 0x00) {
        ESP_LOGW(TAG, "set sensitivity fail");
      }
      break;
    }
    case 0x14: {  // 灵敏度查询回执协议
      uint8_t sensitivity = this->receive_buffer[7];
#ifdef USE_SELECT
      if (this->sensitivity_select_ != nullptr) {
        if (sensitivity == 0x01) {
          this->sensitivity_select_->publish_state("High");
        } else if (sensitivity == 0x02) {
          this->sensitivity_select_->publish_state("Medium");
        } else if (sensitivity == 0x03) {
          this->sensitivity_select_->publish_state("Low");
        }
      }
#endif
      break;
    }
    default: {
      ESP_LOGW(TAG, "unknown cmd: %02X", cmd);
//      this->status_set_warning(); // 还是直接忽略吧
    }
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + frame_size);
}

void LD2460Component::loop() {
  if (!this->task_queue_.empty()) {
    auto task = std::move(this->task_queue_.front());
    if (task) {
      task();
    }
    this->task_queue_.pop();
  }
  uint8_t peeked;
  while (this->available()) {
    if (!this->head_found && this->peek_byte(&peeked) &&
        (peeked != LD2460_CMD_HEAD[0] && peeked != LD2460_UPLOAD_HEAD[0])) {
      this->read();
    } else {
      this->head_found = true;  // fixme 重复设置head_found
    }
    if (this->head_found) {
      this->receive_buffer.push_back((uint8_t) this->read());
    }
  }
  if (this->receive_buffer.size() < 11) {
    return;  // Not enough data to process
  }
  if (memcmp(this->receive_buffer.data(), LD2460_UPLOAD_HEAD, 4) == 0) {  // target detected
    this->parse_upload();
  } else if (memcmp(this->receive_buffer.data(), LD2460_CMD_HEAD, 4) == 0) {
    this->parse_ack();
  } else {
    this->head_found = false;
    this->receive_buffer.clear();
    return;
  }
}

void LD2460Component::enable_upload(bool enable) {
  uint8_t data = enable ? 0x01 : 0x00;
  this->send_command(0x06, &data, 1);
}

void LD2460Component::set_install_params(float height, float angle) {
  uint16_t data1 = (uint16_t) (height * 100);
  uint16_t data2 = (uint16_t) (angle * 100);
  uint8_t data[4] = {(uint8_t) (data1 & 0xFF), (uint8_t) ((data1 >> 8) & 0xFF), (uint8_t) (data2 & 0xFF),
                     (uint8_t) ((data2 >> 8) & 0xFF)};
  this->send_command(0x07, data, 4);
}

void LD2460Component::get_install_params() {
  uint8_t data = 0x01;
  this->send_command(0x08, &data, 1);
}

void LD2460Component::set_mode(const std::string &mode) {
  uint8_t data = mode == "Side" ? 0x01 : 0x02;
  this->send_command(0x09, &data, 1);
}

void LD2460Component::get_mode() {
  uint8_t data = 0x01;
  this->send_command(0x0A, &data, 1);
}

void LD2460Component::get_version() {
  uint8_t data = 0x01;
  this->send_command(0x0B, &data, 1);
}

void LD2460Component::restart_and_read_all_info() {
  this->restart();
  this->set_timeout(1500, [this]() { this->read_all_info(); });
}

void LD2460Component::restart() {
  uint8_t data = 0x01;
  this->send_command(0x0D, &data, 1);
}

void LD2460Component::set_baud_rate(const std::string &baud_rate) {
  uint8_t data = 0x00;  // default 9600
  if (baud_rate == "9600") {
    data = 0x00;
  } else if (baud_rate == "19200") {
    data = 0x01;
  } else if (baud_rate == "38400") {
    data = 0x02;
  } else if (baud_rate == "57600") {
    data = 0x03;
  } else if (baud_rate == "115200") {
    data = 0x04;
  } else if (baud_rate == "230400") {
    data = 0x05;
  } else if (baud_rate == "256000") {
    data = 0x06;
  } else if (baud_rate == "460800") {
    data = 0x07;
  }
  this->send_command(0x0E, &data, 1);
  uint32_t new_baud_rate = stoi(baud_rate);
  if (this->parent_->get_baud_rate() != new_baud_rate) {
    this->parent_->set_baud_rate(new_baud_rate);
  }
  this->set_timeout(200, [this]() { this->restart(); });
}

void LD2460Component::factory_reset() {
  uint8_t data = 0x01;
  this->send_command(0x10, &data, 1);
  this->set_timeout(200, [this]() { this->restart_and_read_all_info(); });
}

void LD2460Component::set_detect_range(float distance, float start_angle, float end_angle) {
  uint8_t data1 = (uint8_t) (distance * 10);
  int16_t data2 = (int16_t) (start_angle * 10);  // -50
  int16_t data3 = (int16_t) (end_angle * 10);    // +50
  uint8_t data[5] = {data1, (uint8_t) (data2 & 0xFF), (uint8_t) ((data2 >> 8) & 0xFF), (uint8_t) (data3 & 0xFF),
                     (uint8_t) ((data3 >> 8) & 0xFF)};
  this->send_command(0x11, data, 5);
}

void LD2460Component::get_detect_range() {
  uint8_t data = 0x01;
  this->send_command(0x12, &data, 1);
}

void LD2460Component::set_sensitivity(const std::string &sensitivity) {
  uint8_t data = 0x01;  // default high
  if (sensitivity == "Low") {
    data = 0x03;
  } else if (sensitivity == "Medium") {
    data = 0x02;
  } else if (sensitivity == "High") {
    data = 0x01;
  }
  this->send_command(0x13, &data, 1);
}

void LD2460Component::get_sensitivity() {
  uint8_t data = 0x01;
  this->send_command(0x14, &data, 1);
}

void LD2460Component::read_all_info() {
  this->get_install_params();
  this->get_mode();
  this->get_version();
  this->get_detect_range();
  this->get_sensitivity();
}

void LD2460Component::send_command(uint8_t command, const uint8_t *data, uint16_t data_size) {
  uint16_t total_size = data_size + 11;  // head(4) + cmd(1) + len(2) + data + tail(4)
  this->write_array(LD2460_CMD_HEAD, 4);
  this->write_byte(command);
  this->write_byte((uint8_t) (total_size & 0xFF));         // length low byte
  this->write_byte((uint8_t) ((total_size >> 8) & 0xFF));  // length high byte
  if (data != nullptr) {
    this->write_array(data, data_size);
  }
  this->write_array(LD2460_CMD_TAIL, 4);
  this->flush();
}

} // namespace esphome::ld2460
