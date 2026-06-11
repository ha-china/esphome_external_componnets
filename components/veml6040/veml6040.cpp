#include "veml6040.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::veml6040 {

static const char *const TAG = "veml6040";
static const uint8_t REG_CONF = 0x00;
static const uint8_t REG_RED = 0x08;
static const uint8_t REG_GREEN = 0x09;
static const uint8_t REG_BLUE = 0x0A;
static const uint8_t REG_WHITE = 0x0B;

void VEML6040Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->write_config(this->integration_time_, this->trig_, this->mode_, false);  // Normal operation
}

void VEML6040Component::dump_config() {
  ESP_LOGCONFIG(TAG, "VEML6040:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Red Sensor", this->red_sensor_);
  LOG_SENSOR("  ", "Green Sensor", this->green_sensor_);
  LOG_SENSOR("  ", "Blue Sensor", this->blue_sensor_);
  LOG_SENSOR("  ", "White Sensor", this->white_sensor_);
}

void VEML6040Component::shutdown() {
  this->write_config(this->integration_time_, this->trig_, this->mode_, true);  // Normal operation
}

void VEML6040Component::wakeup() {
  this->write_config(this->integration_time_, this->trig_, this->mode_, false);  // Normal operation
}

void VEML6040Component::write_config(VEML6040_INTEGRATION_TIME time, bool trig, VEML6040_MODE mode, bool shutdown) {
  std::bitset<8> config;
  switch (time) {
    case VEML6040_INTEGRATION_TIME_40MS:
      config[6] = false;
      config[5] = false;
      config[4] = false;
      break;
    case VEML6040_INTEGRATION_TIME_80MS:
      config[6] = false;
      config[5] = false;
      config[4] = true;
      break;
    case VEML6040_INTEGRATION_TIME_160MS:
      config[6] = false;
      config[5] = true;
      config[4] = false;
      break;
    case VEML6040_INTEGRATION_TIME_320MS:
      config[6] = false;
      config[5] = true;
      config[4] = true;
      break;
    case VEML6040_INTEGRATION_TIME_640MS:
      config[6] = true;
      config[5] = false;
      config[4] = false;
      break;
    case VEML6040_INTEGRATION_TIME_1280MS:
      config[6] = true;
      config[5] = false;
      config[4] = true;
      break;
  }
  if (trig) {
    config[2] = true;  // Trigger mode
  } else {
    config[2] = false;  // Auto mode
  }
  if (mode == VEML6040_MODE_FORCE) {
    config[1] = true;  // Force mode
  } else {
    config[1] = false;  // Auto mode
  }
  if (shutdown) {
    config[0] = true;  // Shutdown mode
  } else {
    config[0] = false;  // Normal operation
  }
  this->send_word(REG_CONF, config.to_ulong());
}

void VEML6040Component::send_word(uint8_t command, uint16_t data) {
  uint8_t buffer[3] = {command, (uint8_t)(data & 0xFF), (uint8_t)((data >> 8) & 0xFF)};
  this->write(buffer, 3);
}

uint16_t VEML6040Component::receive_word(uint8_t command) {
  uint8_t data[2];
  this->write_read(&command, 1, data, 2);
//  this->write(&command, 1, false);  // Write the command
//  this->read(data, 2);
  uint16_t raw = (uint16_t)data[0] | (((uint16_t)data[1]) << 8);  // Combine the two bytes into a single word
  return raw;
}

void VEML6040Component::update() {
  if(this->mode_ == VEML6040_MODE_FORCE) {
    this->write_config(this->integration_time_, 1, this->mode_, false);  // Trigger a measurement
  }
  if (this->red_sensor_ != nullptr) {
    uint16_t red = this->receive_word(REG_RED);
    this->red_sensor_->publish_state(red);
  }
  if (this->green_sensor_ != nullptr) {
    uint16_t green = this->receive_word(REG_GREEN);
    this->green_sensor_->publish_state(green);
  }
  if (this->blue_sensor_ != nullptr) {
    uint16_t blue = this->receive_word(REG_BLUE);
    this->blue_sensor_->publish_state(blue);
  }
  if (this->white_sensor_ != nullptr) {
    uint16_t white = this->receive_word(REG_WHITE);
    this->white_sensor_->publish_state(white);
  }
}


}
