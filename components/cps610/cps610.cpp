#include "cps610.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::cps610 {

static const char *const TAG = "cps610";
static const uint8_t REG_MEASURE = 0x30;
static const uint8_t REG_HIGH = 0x06;
static const uint8_t REG_MEDIUM = 0x07;
static const uint8_t REG_LOW = 0x08;


void CPS610Component::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void CPS610Component::update() {
  this->reg(REG_MEASURE) = 0x0A;
  while (this->reg(REG_MEASURE).get() != 0x02) {
    delay(50);
  }
  uint8_t read_data[3];
  if (this->write_read(&REG_HIGH, 1, read_data, 3) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "CPS610 I2C read error");
    this->status_set_warning();
    return;
  }
  int32_t pressure_raw = ((int32_t)read_data[0] << 16) | ((int32_t)read_data[1] << 8) | (int32_t)read_data[2];
  float pressure_kpa = (float)pressure_raw * 1.02 / 8388608.0;  // 转换为kPa
  this->pressure_sensor_->publish_state(pressure_kpa * this->a_ + this->b_);
  this->status_clear_warning();
}

void CPS610Component::dump_config() {
  ESP_LOGCONFIG(TAG, "CPS610:\n");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "PRESSURE SENSOR", this->pressure_sensor_);
}

} // namespace esphome::cps610
