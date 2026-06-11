#include "ash01ib.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::ash01ib {

const char *const TAG = "ash01ib";
const uint8_t START_MEASUREMENT_CMD[2] = {0x06, 0x01};  // Start measurement command
const uint8_t STOP_MEASUREMENT_CMD[2] = {0x07, 0x01};
const uint8_t GET_STATE_CMD[2] = {0x0F, 0x02};  // Get sensor state command
const uint8_t GET_DATA_CMD[2] = {0x00, 0x02};  // Get sensor data
const uint8_t START_CALIBRATION_CMD[2] = {0x06, 0x17};  // Start calibration command
const uint8_t STOP_CALIBRATION_CMD[2] = {0x07, 0x17};
const uint8_t GET_SN_CMD[2] = {0x08, 0x02};
const uint8_t GET_VERSION_CMD[2] = {0x0A, 0x01};  // Get version command
const uint8_t GET_UNIQUE_ID_CMD[2] = {0x0B, 0x04};  // Get unique ID command

uint8_t ash01ib_crc8(const uint8_t *dat, uint8_t size) {
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

void ASH01IBComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->start_measurement();
}

void ASH01IBComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ASH01IB:\n"
                     "  SN: %d\n"
                     "  VERSION: %d\n"
                     "  UNIQUE ID: %d\n", this->sn(), this->version(), this->unique_id());
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Humidity Sensor", this->humidity_sensor_);
}

void ASH01IBComponent::update() {
  this->write(GET_DATA_CMD, 2);
  uint8_t data[3];
  this->read(data, 3);
  uint8_t crc = ash01ib_crc8(data, 2);
  if(crc!=data[2]) {
    ESP_LOGW(TAG, "ASH01IB CRC error: expected %02X, got %02X", crc, data[2]);
  }
  uint16_t humidity = ((uint16_t)data[0]) << 8 | (uint16_t)data[1];  // Combine the two bytes into one
  if(this->humidity_sensor_!= nullptr) {
    this->humidity_sensor_->publish_state(humidity);  // Convert to percentage
  }
}

void ASH01IBComponent::start_measurement() {
  this->write(START_MEASUREMENT_CMD, 2);
}

void ASH01IBComponent::stop_measurement() {
  this->write(STOP_MEASUREMENT_CMD, 2);
}


void ASH01IBComponent::start_calibration() {
  this->write(START_CALIBRATION_CMD, 2);
}

void ASH01IBComponent::stop_calibration() {
  this->write(STOP_CALIBRATION_CMD, 2);
}

STATE ASH01IBComponent::state() {
  this->write(GET_STATE_CMD, 2);
  uint8_t data[3];
  this->read(data, 3);
  uint8_t crc = ash01ib_crc8(data, 2);
  if(crc!=data[2]) {
    ESP_LOGW(TAG, "ASH01IB CRC error: expected %02X, got %02X", crc, data[2]);
    return STATE_ERROR;  // CRC error
  }
  std::bitset<8> status = data[0];
  if(!status[7] && !status[6]) {
    return STATE_WAITING;
  }
  if(!status[7] && status[6]) {
    return STATE_OK;
  }
  return STATE_ERROR;
}

uint16_t ASH01IBComponent::sn() {
  this->write(GET_SN_CMD, 2);
  uint8_t data[3];
  this->read(data, 3);
  uint8_t crc = ash01ib_crc8(data, 2);
  if(crc!=data[2]) {
    ESP_LOGW(TAG, "ASH01IB CRC error: expected %02X, got %02X", crc, data[2]);
    return 0;  // CRC error
  }
  uint16_t sn = ((uint16_t)data[0]) << 8 | (uint16_t)data[1];
  return sn;
}

uint16_t ASH01IBComponent::version() {
  this->write(GET_VERSION_CMD, 2);
  uint8_t data[3];
  this->read(data, 3);
  uint8_t crc = ash01ib_crc8(data, 2);
  if(crc!=data[2]) {
    ESP_LOGW(TAG, "ASH01IB CRC error: expected %02X, got %02X", crc, data[2]);
    return 0;  // CRC error
  }
  uint16_t version = ((uint16_t)data[0]) << 8 | (uint16_t)data[1];
  return version;
}

uint32_t ASH01IBComponent::unique_id() {
  this->write(GET_UNIQUE_ID_CMD, 2);
  uint8_t data[5];
  this->read(data, 5);
  uint8_t crc = ash01ib_crc8(data, 4);
  if(crc!=data[4]) {
    ESP_LOGW(TAG, "ASH01IB CRC error: expected %02X, got %02X", crc, data[4]);
    return 0;  // CRC error
  }
  uint32_t unique_id = ((uint32_t)data[0]) << 24 | ((uint32_t)data[1]) <<16 | ((uint32_t)data[2]) << 8 | (uint32_t)data[3];
  return unique_id;
}

}

