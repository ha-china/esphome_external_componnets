#include "aof1000.h"
#include "esphome/core/log.h"

namespace esphome::aof1000 {

static const char *const TAG = "aof1000";
static const uint8_t GET_DATA_CMD[4] = {0x11, 0x01, 0x01, 0xED};

uint8_t aof1000_crc8(uint8_t *data, size_t len) {
  uint8_t crc = 0x00;
  for (int i = 0; i < len; i++) {
    crc = crc - data[i];
  }
  return crc;
}

void AOF1000Component::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void AOF1000Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AOF1000:");
  LOG_SENSOR("  ", "O2 Sensor", this->o2_sensor_);
  LOG_SENSOR("  ", "Volume Flow Rate Sensor", this->volume_flow_rate_sensor_);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
  this->check_uart_settings(9600);
}

void AOF1000Component::update() {
  while (this->available()) {
    this->read();
  }
  this->write_array(GET_DATA_CMD, 4);
  this->read_array(this->buffer, 12);
  if (this->buffer[0] != 0x16 || this->buffer[1] != 0x09 || this->buffer[2] != 0x01) {
    ESP_LOGW(TAG, "AOF1000 read data header error: expected 0x16 0x09 0x01, got %02X %02X %02X", this->buffer[0],
             this->buffer[1], this->buffer[2]);
    this->status_set_warning();
    return;
  }
  if (this->buffer[9] != 0x00 || this->buffer[10] != 0x00 ) {
    ESP_LOGW(TAG, "AOF1000 read tail error: expected 0x00 0x00, got %02X %02X",
             this->buffer[9], this->buffer[10]);
    this->status_set_warning();
    return;
  }
  uint8_t crc = aof1000_crc8(this->buffer, 11);
  if (crc != this->buffer[11]) {
    ESP_LOGW(TAG, "AOF1000 CRC error: expected %02X, got %02X", crc, this->buffer[11]);
    this->status_set_warning();
    return;  // CRC error
  }
  uint16_t o2 = ((uint16_t) (this->buffer[3])) << 8 | (uint16_t) (this->buffer[4]);
  if (this->o2_sensor_ != nullptr) {
    this->o2_sensor_->publish_state(((float) o2) / 10.0f);
  }
  uint16_t flow_rate = ((uint16_t) (this->buffer[5])) << 8 | (uint16_t) (this->buffer[6]);
  if (this->volume_flow_rate_sensor_ != nullptr) {
    this->volume_flow_rate_sensor_->publish_state(((float) flow_rate) / 10.0f);  // Convert to L/min
  }
  uint16_t temperature = ((uint16_t) (this->buffer[7])) << 8 | (uint16_t) (this->buffer[8]);
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(((float) temperature) / 10.0f);  // Convert to Celsius
  }
  this->status_clear_warning();
}

} // namespace esphome::aof1000
