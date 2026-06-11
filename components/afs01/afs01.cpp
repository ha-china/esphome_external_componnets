#include "afs01.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::afs01 {

static const char *const TAG = "afs01";
static const uint8_t GET_DATA_CMD[2] = {0x10, 0x00};  // Get sensor data command
static const uint8_t GET_ID_CMD[2] = {0x31, 0xAE};  // Get sensor ID command

uint8_t afs01_crc8(const uint8_t *dat, uint8_t size) {
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

void AFS01Component::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void AFS01Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AFS01:\n"
                     "  ID: %d\n", this->unique_id());
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Volume Flow Rate Sensor", this->volume_flow_rate_sensor_);
}

void AFS01Component::update() {
  this->write(GET_DATA_CMD, 2);
  uint8_t data[3];
  this->read(data, 3);
  uint8_t crc = afs01_crc8(data, 2);
  if (crc != data[2]) {
    ESP_LOGW(TAG, "AFS01 CRC error: expected %02X, got %02X", crc, data[2]);
    this->status_set_warning();
    return;  // CRC error
  }
  uint16_t volume_flow_rate = ((uint16_t)data[0]) << 8 | (uint16_t)data[1];  // Combine the two bytes into one
  if (this->volume_flow_rate_sensor_ != nullptr) {
    this->volume_flow_rate_sensor_->publish_state(volume_flow_rate);  // Publish the volume flow rate
  }
  this->status_clear_warning();
}

uint32_t AFS01Component::unique_id() {
  this->write(GET_ID_CMD, 2);
  uint8_t data[6];
  this->read(data, 6);
  uint8_t crc = afs01_crc8(data, 2);
  if(data[2] != crc) {
    ESP_LOGW(TAG, "AFS01 CRC error: expected %02X, got %02X", crc, data[2]);
    this->status_set_warning();
    return 0;  // CRC error
  }
  crc = afs01_crc8((uint8_t*)data+3, 2);
  if(data[5] != crc) {
    ESP_LOGW(TAG, "AFS01 CRC error: expected %02X, got %02X", crc, data[5]);
    this->status_set_warning();
    return 0;  // CRC error
  }
  uint32_t id_ = ((uint32_t)data[0]) << 24 | ((uint32_t)data[1]) << 16 | ((uint32_t)data[3]) << 8 | (uint32_t)data[4];
  this->status_clear_warning();
  return id_;
}

}
