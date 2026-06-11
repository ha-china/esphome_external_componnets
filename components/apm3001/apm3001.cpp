#include "apm3001.h"
#include "esphome/core/log.h"

namespace esphome::apm3001 {

static const char *const TAG = "apm3001";
static const uint8_t START_MEASUREMENT_CMD[5] = {0xFE, 0xA5, 0x00, 0x11, 0xB6};
static const uint8_t STOP_MEASUREMENT_CMD[5] = {0xFE, 0xA5, 0x00, 0x10, 0xB5};
static const uint8_t READ_CMD[5] = {0xFE, 0xA5, 0x00, 0x07, 0xAC};  // Read command

uint8_t checksum(const uint8_t *data, size_t size) {
  uint8_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum;
}

void APM3001Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->start_measurement();
}

void APM3001Component::dump_config() {
  ESP_LOGCONFIG(TAG, "APM3001:");
  LOG_SENSOR("  ", "PM1.0", this->pm1_sensor_);
  LOG_SENSOR("  ", "PM2.5", this->pm2_5_sensor_);
  LOG_SENSOR("  ", "PM4.0", this->pm4_sensor_);
  LOG_SENSOR("  ", "PM10.0", this->pm10_sensor_);
  this->check_uart_settings(9600);
}

void APM3001Component::update() {
  this->write_array(READ_CMD, 5);
  uint8_t data[13];
  this->read_array(data, 13);
  if(data[12] != checksum((uint8_t*)data+1, 11)) {
    ESP_LOGW(TAG, "APM3001 read data checksum error");
    this->status_set_warning();
    return;  // Checksum error
  }
  if(this->pm1_sensor_ != nullptr) {
    this->pm1_sensor_->publish_state((((uint16_t)data[4]) << 8) | (uint16_t)data[5]);
  }
  if(this->pm2_5_sensor_ != nullptr) {
    this->pm2_5_sensor_->publish_state((((uint16_t)data[6]) << 8) | (uint16_t)data[7]);
  }
  if(this->pm4_sensor_ != nullptr) {
    this->pm4_sensor_->publish_state((((uint16_t)data[8]) << 8) | (uint16_t)data[9]);
  }
  if(this->pm10_sensor_ != nullptr) {
    this->pm10_sensor_->publish_state((((uint16_t)data[10]) << 8) | (uint16_t)data[11]);
  }
  this->status_clear_warning();  // Clear warning if everything is fine
}

void APM3001Component::start_measurement() {
  this->write_array(START_MEASUREMENT_CMD, 5);
  uint8_t data[7];
  this->read_array(data, 7);
  if(data[6] != checksum((uint8_t*)data+1, 5)) {
    ESP_LOGW(TAG, "APM3001 start measurement checksum error");
    this->status_set_warning();
    return;  // Checksum error
  }
  if(data[2] != 0x02 || data[3] != 0x00 || data[4] != 0x00 || data[5] != 0x11) {
    ESP_LOGW(TAG, "APM3001 start measurement response error");
    this->status_set_warning();
    return;  // Response error
  }
}

void APM3001Component::stop_measurement() {
  this->write_array(STOP_MEASUREMENT_CMD, 5);
  uint8_t data[7];
  this->read_array(data, 7);
  if(data[6] != checksum((uint8_t*)data+1, 5)) {
    ESP_LOGW(TAG, "APM3001 stop measurement checksum error");
    this->status_set_warning();
    return;  // Checksum error
  }
  if(data[2] != 0x02 || data[3] != 0x00 || data[4] != 0x00 || data[5] != 0x10) {
    ESP_LOGW(TAG, "APM3001 stop measurement response error");
    this->status_set_warning();
    return;  // Response error
  }
}

}
