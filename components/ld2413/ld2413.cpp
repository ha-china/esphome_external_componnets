#include "ld2413.h"
#include "esphome/core/log.h"

namespace esphome::ld2413 {

static const char *const TAG = "ld2413";

static const uint8_t LD2413_HEAD[4] = {0xFD, 0xFC, 0xFB, 0xFA};
static const uint8_t LD2413_TAIL[4] = {0x04, 0x03, 0x02, 0x01};
static const uint16_t GET_VERSION_COMMAND = 0x0000;  // Command to get version
static const uint16_t ENABLE = 0x00FF;
static const uint16_t DISABLE = 0x00FE;
static const uint16_t MIN_DISTANCE = 0x0074;
static const uint16_t MAX_DISTANCE = 0x0075;
static const uint16_t UPDATE_DOOR_LIMIT = 0x0072;
static const uint16_t SET_REPORT_INTERVAL = 0x0071;
static const uint16_t GET_REPORT_INTERVAL = 0x0070;

void LD2413Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LD2413...");
  this->enable_config();
  this->set_report_interval(this->update_interval_);
  this->set_min_distance(this->min_distance_);
  this->set_max_distance(this->max_distance_);
  this->disable_config();
}

void LD2413Component::dump_config() {
  ESP_LOGCONFIG(TAG,
                "LD2413:\n"
                "  Version: %s\n",
                this->version().c_str());
  ESP_LOGCONFIG(TAG, "  Update Interval: %d ms", this->update_interval_);
  ESP_LOGCONFIG(TAG, "  Min Distance: %d mm", this->min_distance_);
  ESP_LOGCONFIG(TAG, "  Max Distance: %d mm", this->max_distance_);
  LOG_SENSOR("  ", "Distance Sensor", this->distance_sensor_);
  this->check_uart_settings(115200);
}

void LD2413Component::loop() {
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
  if (this->receive_buffer.size() < 14) {
    return;  // Not enough data to process
  }
  if (memcmp(LD2413_HEAD, this->receive_buffer.data(), 4) != 0) {
    ESP_LOGW(TAG, "Received data does not start with LD2413 head");
//    this->status_set_warning();
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  if (memcmp(LD2413_TAIL, this->receive_buffer.data() + 10, 4) != 0) {
    ESP_LOGW(TAG, "Received data does not end with LD2413 tail");
//    this->status_set_warning();
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  uint8_t *ptr = this->receive_buffer.data() + 6;
  float value;
  memcpy(&value, ptr, 4);
  if (this->distance_sensor_ != nullptr) {
    this->distance_sensor_->publish_state(value);
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + 14);
}

uint32_t LD2413Component::get_report_interval() {
  uint16_t ret_command, status;
  std::string value;
  this->send_command(GET_REPORT_INTERVAL, nullptr, 0, &ret_command, &status, &value);
  if (status != 0) {
    this->status_set_warning();
  }
  uint32_t interval = (uint32_t) status | ((uint32_t) value[0]) << 16 | ((uint32_t) value[1]) << 24;
  return interval;
}

void LD2413Component::set_report_interval(uint16_t interval) {
  this->send_buffer[0] = (uint8_t) (interval & 0xFF);
  this->send_buffer[1] = (uint8_t) ((interval >> 8) & 0xFF);
  uint16_t ret_command, status;
  this->send_command(SET_REPORT_INTERVAL, this->send_buffer, 2, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
}

void LD2413Component::update_door_limit() {
  uint16_t ret_command, status;
  this->send_command(UPDATE_DOOR_LIMIT, nullptr, 0, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
}

// unit: mm 150-10500
void LD2413Component::set_min_distance(uint16_t min_distance) {
  this->send_buffer[0] = (uint8_t) (min_distance & 0xFF);
  this->send_buffer[1] = (uint8_t) ((min_distance >> 8) & 0xFF);
  uint16_t ret_command, status;
  this->send_command(MIN_DISTANCE, this->send_buffer, 2, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
}

// unit: mm 150-10500
void LD2413Component::set_max_distance(uint16_t max_distance) {
  this->send_buffer[0] = (uint8_t) (max_distance & 0xFF);
  this->send_buffer[1] = (uint8_t) ((max_distance >> 8) & 0xFF);
  uint16_t ret_command, status;
  this->send_command(MAX_DISTANCE, this->send_buffer, 2, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
}

void LD2413Component::disable_config() {
  uint16_t ret_command, status;
  this->send_command(DISABLE, nullptr, 0, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
  this->in_config = false;
}

void LD2413Component::enable_config() {
  this->send_buffer[0] = 0x01;  // Enable configuration mode
  this->send_buffer[1] = 0x00;  // Enable configuration mode
  uint16_t ret_command, status;
  this->send_command(ENABLE, this->send_buffer, 2, &ret_command, &status, nullptr);
  if (status != 0) {
    this->status_set_warning();
  }
  this->in_config = true;
}

std::string LD2413Component::version() {
  uint16_t ret_command, major;
  std::string value;
  this->send_command(GET_VERSION_COMMAND, this->send_buffer, 0, &ret_command, &major, &value);
  std::string major_str(reinterpret_cast<const char *>(&major), 2);
  return major_str + value;
}

void LD2413Component::send_command(uint16_t command, const uint8_t *data, size_t data_size, uint16_t *ret_command,
                                   uint16_t *status, std::string *value) {
  while (this->available()) {
    this->read();  // Clear any existing data in the buffer
  }
  this->write_array(LD2413_HEAD, 4);
  uint16_t frame_data_size = data_size + 2;         // 2 bytes for command
  this->write((uint8_t) (frame_data_size & 0xFF));  // little endian
  this->write((uint8_t) ((frame_data_size >> 8) & 0xFF));
  this->write((uint8_t) (command & 0xFF));  // little endian
  this->write((uint8_t) ((command >> 8) & 0xFF));
  if (data_size > 0) {
    this->write_array(data, data_size);
  }
  this->write_array(LD2413_TAIL, 4);
  this->flush();
  // read ack
  uint8_t ack[4];
  this->read_array(ack, 4);
  if (memcmp(ack, LD2413_HEAD, 4) != 0) {
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
  if (status != nullptr) {
    // status is in little endian
    *status = (uint16_t) buffer[2] | ((uint16_t) buffer[3]) << 8;
  }
  if (value != nullptr) {
    std::string retval(reinterpret_cast<const char *>(buffer + 4), frame_data_size - 4);
    *value = retval;
  }
  delete[] buffer;
  this->read_array(ack, 4);  // tail
  if (memcmp(ack, LD2413_TAIL, 4) != 0) {
    ESP_LOGW(TAG, "Failed to send command, received tail: %02X %02X %02X %02X", ack[0], ack[1], ack[2], ack[3]);
    this->status_set_warning();
  }
  this->status_clear_warning();
}

} // namespace esphome::ld2413
