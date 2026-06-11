#include "apm10.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::apm10 {

static const char *const TAG = "apm10";
static const uint8_t START_MEASUREMENT_CMD[5] = {0x00, 0x10, 0x05, 0x00, 0xF6};  // Start measurement command
static const uint8_t STOP_MEASUREMENT_CMD[2] = {0x01, 0x04};
static const uint8_t READ_CMD[2] = {0x03, 0x00};  // Read command


uint8_t apm10_crc8(const uint8_t *dat, uint8_t size) {
  uint8_t crc = 0xFF, i;
  for (uint8_t b = 0; b < size; b++) {
    crc ^= (dat[b]);
    for (i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

void APM10Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->start_measurement();
}

void APM10Component::dump_config() {
  ESP_LOGCONFIG(TAG, "APM10:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "PM1.0", this->pm1_sensor_);
  LOG_SENSOR("  ", "PM2.5", this->pm2_5_sensor_);
  LOG_SENSOR("  ", "PM10.0", this->pm10_sensor_);
}

void APM10Component::start_measurement() {
  this->write(START_MEASUREMENT_CMD, 5);
}

void APM10Component::stop_measurement() {
  this->write(STOP_MEASUREMENT_CMD, 2);
}

void APM10Component::update() {
  this->write(READ_CMD, 2);
  uint8_t data[30];
  this->read(data, 30);
  uint8_t crc;
  crc = apm10_crc8(data, 2);
  if(crc!=data[2]) {
    ESP_LOGW(TAG, "APM10 CRC error: expected %02X, got %02X", crc, data[2]);
    return;  // CRC error
  }
  crc = apm10_crc8((uint8_t *)data + 3, 2);
  if (crc!= data[5]) {
    ESP_LOGW(TAG, "APM10 CRC error: expected %02X, got %02X", crc, data[5]);
    return;  // CRC error
  }
  if(this->type_ == APM10_TYPE_3000) {
    crc = apm10_crc8((uint8_t *)data + 6, 2);
    if(crc!=data[8]) {
      ESP_LOGW(TAG, "APM10 CRC error: expected %02X, got %02X", crc, data[8]);
      return;  // CRC error
    }
  }
  crc = apm10_crc8((uint8_t *)data + 9, 2);
  if (crc != data[11]) {
    ESP_LOGW(TAG, "APM10 CRC error: expected %02X, got %02X", crc, data[11]);
    return;  // CRC error
  }
  uint16_t pm1_0 = (((uint16_t)data[0]) << 8) | ((uint16_t)data[1]);  // PM1.0
  if (this->pm1_sensor_ != nullptr) {
    this->pm1_sensor_->publish_state(pm1_0);
  }
  uint16_t pm2_5 = (((uint16_t)data[3]) << 8) | ((uint16_t)data[4]);  // PM2.5
  if (this->pm2_5_sensor_ != nullptr) {
    this->pm2_5_sensor_->publish_state(pm2_5);
  }
  if(this->type_ == APM10_TYPE_3000) {
    uint16_t pm4 = (((uint16_t)data[6]) << 8) | ((uint16_t)data[7]);  // PM4.0
    if(this->pm4_sensor_!= nullptr) {
      this->pm4_sensor_->publish_state(pm4);
    }
  }
  uint16_t pm10 = (((uint16_t)data[9]) << 8) | ((uint16_t)data[10]);  // PM10.0
  if (this->pm10_sensor_ != nullptr) {
    this->pm10_sensor_->publish_state(pm10);
  }
}

}
