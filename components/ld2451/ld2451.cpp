#include "ld2451.h"
#include "esphome/core/log.h"

namespace esphome::ld2451 {

static const char *const TAG = "ld2451";

static const uint8_t LD2451_HEAD[4] = {0xFD, 0xFC, 0xFB, 0xFA};
static const uint8_t LD2451_TAIL[4] = {0x04, 0x03, 0x02, 0x01};
static const uint16_t ENABLE = 0x00FF;
static const uint16_t DISABLE = 0x00FE;
static const uint16_t SET_TARGET_DETECT_CONF = 0x0002;
static const uint16_t GET_TARGET_DETECT_CONF = 0x0012;
static const uint16_t SET_SENSITIVITY = 0x0003;
static const uint16_t GET_SENSITIVITY = 0x0013;
static const uint16_t GET_VERSION = 0x00A0;
static const uint16_t SET_BAUD_RATE = 0x00A1;
static const uint16_t RESET = 0x00A2;
static const uint16_t RESTART = 0x00A3;

void LD2451Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LD2451...");
  this->enable_config();
  this->set_sensitivity(this->valid_trigs_, this->signal_threshold_);
  this->set_target_detect_config(this->max_distance_, this->direction_, this->min_speed_, this->delay_);
  this->disable_config();
#ifdef USE_TEXT_SENSOR
  if (this->version_text_sensor_ != nullptr) {
    this->version_text_sensor_->publish_state(this->version());
  }
#endif
}

void LD2451Component::dump_config() {
  ESP_LOGCONFIG(TAG,
                "LD2451:\n"
                "  Version: %s\n",
                this->version().c_str());
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Has Towards Target", this->has_towards_target_binary_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "Target Number", this->target_number_sensor_);
  for (uint8_t i = 0; i < MAX_TARGETS; i++) {
    ESP_LOGCONFIG(TAG, "target %d:\n", i + 1);
    if (this->target_angle_sensors_[i] != nullptr) {
      LOG_SENSOR("  ", "Target Angle", this->target_angle_sensors_[i]);
    }
    if (this->target_distance_sensors_[i] != nullptr) {
      LOG_SENSOR("  ", "Target Distance", this->target_distance_sensors_[i]);
    }
    if (this->target_speed_sensors_[i] != nullptr) {
      LOG_SENSOR("  ", "Target Speed", this->target_speed_sensors_[i]);
    }
    if (this->target_signal_strength_sensors_[i] != nullptr) {
      LOG_SENSOR("  ", "Target Signal Strength", this->target_signal_strength_sensors_[i]);
    }
  }
#endif
#ifdef USE_TEXT_SENSOR
  for (uint8_t i = 0; i < MAX_TARGETS; i++) {
    if (this->target_direction_text_sensors_[i] != nullptr) {
      LOG_TEXT_SENSOR("  ", "Target Direction", this->target_direction_text_sensors_[i]);
    }
  }
#endif
  this->check_uart_settings(115200);
}

void LD2451Component::loop() {
  uint8_t peeked;
  while (this->available() && !this->in_config) {
    if (!this->head_found && this->peek_byte(&peeked) && peeked != 0xFD) {
      this->read();
    } else {
      this->head_found = true;
    }
    if (this->head_found) {
      this->receive_buffer.push_back((uint8_t) this->read());
    }
  }
  if (this->receive_buffer.size() < 6) {
    return;  // Not enough data to process
  }
  if (memcmp(LD2451_HEAD, this->receive_buffer.data(), 4) != 0) {
    ESP_LOGW(TAG, "Received data does not start with LD2451 head");
//    this->status_set_warning();
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  uint16_t frame_size = (uint16_t) this->receive_buffer[4] | ((uint16_t) this->receive_buffer[5]) << 8;
  if (this->receive_buffer.size() < (frame_size + 10)) {
    return;
  }
  if (memcmp(LD2451_TAIL, this->receive_buffer.data() + frame_size + 6, 4) != 0) {
    ESP_LOGW(TAG, "Received data does not end with LD2451 tail");
//    this->status_set_warning();
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  uint8_t *ptr = this->receive_buffer.data() + 6;
  uint8_t num = ptr[0];  // 目标数量
#ifdef USE_SENSOR
  if (this->target_number_sensor_ != nullptr) {
    this->target_number_sensor_->publish_state(num);
  }
#endif
#ifdef USE_BINARY_SENSOR
  if (this->has_towards_target_binary_sensor_ != nullptr) {
    if (ptr[1]) {
      this->has_towards_target_binary_sensor_->publish_state(true);
    } else {
      this->has_towards_target_binary_sensor_->publish_state(false);
    }
  }
#endif
  uint8_t angle, distance, direction, speed, signal;
  for (uint8_t i = 0; i < num; i++) {
    if (i >= MAX_TARGETS) {
      ESP_LOGW(TAG, "Received target number %d exceeds maximum of %d", num, MAX_TARGETS);
      break;
    }
    angle = ptr[2 + 5 * i];
    distance = ptr[2 + 5 * i + 1];
    direction = ptr[2 + 5 * i + 2];
    speed = ptr[2 + 5 * i + 3];
    signal = ptr[2 + 5 * i + 4];  // 信噪比 publish_state here
#ifdef USE_SENSOR
    if (this->target_angle_sensors_[i] != nullptr) {
      this->target_angle_sensors_[i]->publish_state(((int16_t)angle)-0x80);
    }
    if (this->target_distance_sensors_[i] != nullptr) {
      this->target_distance_sensors_[i]->publish_state(distance);
    }
#endif
#ifdef USE_TEXT_SENSOR
    if (this->target_direction_text_sensors_[i] != nullptr) {
      this->target_direction_text_sensors_[i]->publish_state(direction ? "towards" : "away");
    }
#endif
#ifdef USE_SENSOR
    if (this->target_speed_sensors_[i] != nullptr) {
      this->target_speed_sensors_[i]->publish_state(speed);
    }
    if (this->target_signal_strength_sensors_[i] != nullptr) {
      this->target_signal_strength_sensors_[i]->publish_state(signal);
    }
#endif
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + frame_size + 10);
}

bool LD2451Component::restart() {
  std::string value;
  this->send_command(RESTART, nullptr, 0, nullptr, &value);
  uint8_t *ptr = (uint8_t *) value.c_str();
  uint16_t status = (uint16_t) ptr[0] | ((uint16_t) ptr[1]) << 8;
  if (status != 0) {
    this->status_set_warning();
    return false;
  }
  this->status_clear_warning();
  return true;
}

bool LD2451Component::reset() {
  std::string value;
  this->send_command(RESET, nullptr, 0, nullptr, &value);
  uint8_t *ptr = (uint8_t *) value.c_str();
  uint16_t status = (uint16_t) ptr[0] | ((uint16_t) ptr[1]) << 8;
  if (status != 0) {
    this->status_set_warning();
    return false;
  }
  this->status_clear_warning();
  return true;
}

void LD2451Component::set_baud_rate(LD2451_BAUD_RATE baud_rate) {
  this->send_buffer[0] = (uint8_t) ((uint16_t) baud_rate & 0xFF);
  this->send_buffer[1] = 0x00;
  this->send_command(SET_BAUD_RATE, this->send_buffer, 2, nullptr, nullptr);
  //  switch (baud_rate) {
  //    case LD2451_BAUD_RATE_9600:
  //      this->parent_->set_baud_rate(9600);
  //      break;
  //    case LD2451_BAUD_RATE_19200:
  //      this->parent_->set_baud_rate(19200);
  //      break;
  //    case LD2451_BAUD_RATE_38400:
  //      this->parent_->set_baud_rate(38400);
  //      break;
  //    case LD2451_BAUD_RATE_57600:
  //      this->parent_->set_baud_rate(57600);
  //      break;
  //    case LD2451_BAUD_RATE_115200:
  //      this->parent_->set_baud_rate(115200);
  //      break;
  //    case LD2451_BAUD_RATE_230400:
  //      this->parent_->set_baud_rate(230400);
  //      break;
  //    case LD2451_BAUD_RATE_256000:
  //      this->parent_->set_baud_rate(256000);
  //      break;
  //    case LD2451_BAUD_RATE_460800:
  //      this->parent_->set_baud_rate(460800);
  //      break;
  //  }
  this->set_timeout(200, [this]() { this->restart(); });
}

std::string LD2451Component::version() {
  std::string value;
  this->send_command(GET_VERSION, nullptr, 0, nullptr, &value);
  return value;
}

bool LD2451Component::get_sensitivity(uint8_t *valid_trigs, uint8_t *signal_threshold) {
  uint16_t ret_command;
  std::string value;
  this->send_command(GET_SENSITIVITY, nullptr, 0, &ret_command, &value);
  uint8_t *ptr = (uint8_t *) value.c_str();
  uint16_t status = (uint16_t) ptr[0] | ((uint16_t) ptr[1]) << 8;
  if (status != 0) {
    this->status_set_warning();
    return false;
  }
  *valid_trigs = ptr[2];       // 累积有效触发次数 1-10 默认1
  *signal_threshold = ptr[3];  // 信噪比阈值等级
  this->status_clear_warning();
  return true;
}

void LD2451Component::set_sensitivity(uint8_t valid_trigs, uint8_t signal_threshold) {
  this->send_buffer[0] = valid_trigs;       // 累积有效触发次数 1-10 默认1
  this->send_buffer[1] = signal_threshold;  // 信噪比阈值等级 3-8 00时候默认4
  this->send_buffer[2] = 0x00;              // 0-120
  this->send_buffer[3] = 0x00;
  uint16_t ret_command;
  this->send_command(SET_SENSITIVITY, this->send_buffer, 4, &ret_command, nullptr);
}

bool LD2451Component::get_target_detect_config(uint8_t *max_distance, LD2451_DIRECTION *direction, uint8_t *min_speed,
                                               uint8_t *delay) {
  uint16_t ret_command;
  std::string value;
  this->send_command(GET_TARGET_DETECT_CONF, nullptr, 0, &ret_command, &value);
  uint8_t *ptr = (uint8_t *) value.c_str();
  uint16_t status = (uint16_t) ptr[0] | ((uint16_t) ptr[1]) << 8;
  if (status != 0) {
    this->status_set_warning();
    return false;
  }
  *max_distance = ptr[2];  // meter
  *direction = (LD2451_DIRECTION) ptr[3];
  *min_speed = ptr[4];  // 120 max
  *delay = ptr[5];      // s
  this->status_clear_warning();
  return true;
}

void LD2451Component::set_target_detect_config(uint8_t max_distance, LD2451_DIRECTION direction, uint8_t min_speed,
                                               uint8_t delay_) {
  this->send_buffer[0] = max_distance;  // meter
  this->send_buffer[1] = (uint8_t) direction;
  this->send_buffer[2] = min_speed;  // 120 max
  this->send_buffer[3] = delay_;     // s
  uint16_t ret_command;
  this->send_command(SET_TARGET_DETECT_CONF, this->send_buffer, 4, &ret_command, nullptr);
}

void LD2451Component::disable_config() {
  uint16_t ret_command;
  this->send_command(DISABLE, nullptr, 0, &ret_command, nullptr);
  this->in_config = false;
}

void LD2451Component::enable_config() {
  this->send_buffer[0] = 0x01;  // Enable configuration mode
  this->send_buffer[1] = 0x00;  // Enable configuration mode
  uint16_t ret_command;
  this->send_command(ENABLE, this->send_buffer, 2, &ret_command, nullptr);
  this->in_config = true;
}

void LD2451Component::send_command(uint16_t command, const uint8_t *data, size_t data_size, uint16_t *ret_command,
                                   std::string *value) {
  while (this->available()) {
    this->read();  // Clear any existing data in the buffer
  }
  this->write_array(LD2451_HEAD, 4);
  uint16_t frame_data_size = data_size + 2;         // 2 bytes for command
  this->write((uint8_t) (frame_data_size & 0xFF));  // little endian
  this->write((uint8_t) ((frame_data_size >> 8) & 0xFF));
  this->write((uint8_t) (command & 0xFF));  // little endian
  this->write((uint8_t) ((command >> 8) & 0xFF));
  if (data_size > 0) {
    this->write_array(data, data_size);
  }
  this->write_array(LD2451_TAIL, 4);
  this->flush();
  // read ack
  uint8_t ack[4];
  this->read_array(ack, 4);
  if (memcmp(ack, LD2451_HEAD, 4) != 0) {
    ESP_LOGW(TAG, "Failed to send command, received head: %02X %02X %02X %02X", ack[0], ack[1], ack[2], ack[3]);
    this->status_set_warning();
  }
  this->read_array(ack, 2);
  frame_data_size = (uint16_t) ack[0] | ((uint16_t) ack[1]) << 8;
  uint8_t *buffer = new uint8_t[frame_data_size];
  this->read_array(buffer, frame_data_size);  // ack data
  if (ret_command != nullptr) {
    *ret_command = (uint16_t) buffer[0] | ((uint16_t) buffer[1]) << 8;
  }
  if (value != nullptr) {
    std::string retval(reinterpret_cast<const char *>(buffer + 2), frame_data_size - 2);
    *value = retval;
  }
  delete[] buffer;
  this->read_array(ack, 4);  // tail
  if (memcmp(ack, LD2451_TAIL, 4) != 0) {
    ESP_LOGW(TAG, "Failed to send command, received tail: %02X %02X %02X %02X", ack[0], ack[1], ack[2], ack[3]);
    this->status_set_warning();
  }
  this->status_clear_warning();
}

} // namespace esphome::ld2451
