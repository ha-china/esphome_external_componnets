#include "gd60914.h"
#include "esphome/core/log.h"

namespace esphome::gd60914 {

static const char *const TAG = "gd60914";

void GD60914Component::setup() {
  ESP_LOGW(TAG, "setup gd60914 sensor");
  this->write_byte(SINGLE); //  打开单次测量功能，只需发一次
  this->flush();
}

void GD60914Component::update() {
  while (this->available()) {
    this->read();
  }
  this->write_byte(this->mode_);
  this->flush();
  uint8_t buffer[8];
  buffer[7] = 0;
  this->read_array(buffer, 7);
  int temperature = atoi((const char*)buffer);
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state((float)temperature / 10.0f); // 温度传感器精度为0.1度
  }
}

void GD60914Component::dump_config() {
  ESP_LOGCONFIG(TAG, "gd60914:");
  LOG_SENSOR("  ", "TEMPERATURE", this->temperature_sensor_);
  this->check_uart_settings(9600);
}

void GD60914Component::reset() {
  while (this->available()) {
    this->read();
  }
  this->write_array(RESET_CMD, 5);
  this->flush();
}

void GD60914Component::calibrate35() {
  while (this->available()) {
    this->read();
  }
  this->write_array(CALIBRATE35_CMD, 5);
  this->flush();
}

void GD60914Component::calibrate42() {
  while (this->available()) {
    this->read();
  }
  this->write_array(CALIBRATE42_CMD, 5);
  this->flush();
}

}
