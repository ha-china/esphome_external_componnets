#include "veml6075.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::veml6075 {

static const char *const TAG = "veml6075";
static const uint8_t REG_UV_CONF = 0x00;
static const uint8_t REG_UVA = 0x07;
static const uint8_t REG_UVB = 0x09;
static const uint8_t REG_UVCOMP1 = 0x0A;
static const uint8_t REG_UVCOMP2 = 0x0B;
static const uint8_t REG_ID = 0x0C;  // Device ID register

void VEML6075Component::setup() {
  this->write_config(this->time_, this->dsetting_, VEML6075_TRIGGER_NONE, this->force_mode_);
  this->set_coefficients();
}

void VEML6075Component::dump_config() {
  ESP_LOGCONFIG(TAG, "VEML6075:\n"
                     "  DEVICE ID: 0x%02X\n", this->read_id());
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "UVA Sensor", this->uva_sensor_);
  LOG_SENSOR("  ", "UVB Sensor", this->uvb_sensor_);
  LOG_SENSOR("  ", "UVI Sensor", this->uvi_sensor_);
}

void VEML6075Component::update() {
  VEML6075IntegrationTime time;
  VEML6075DynamicSetting dsetting;
  VEML6075Trigger trigger;
  VEML6075ActiveForceMode force_mode;
  this->read_config(&time, &dsetting, &trigger, &force_mode);
  if(force_mode==VEML6075_ACTIVE_FORCE_MODE_ENABLE) {
    // trigger one reading
    this->write_config(time, dsetting, VEML6075_TRIGGER_ONCE, force_mode);
    switch (time) {
      case VEML6075_IT_50MS:
        delay(50*1.1);
        break;
      case VEML6075_IT_100MS:
        delay(50*1.1);
        break;
      case VEML6075_IT_200MS:
        delay(50*1.1);
        break;
      case VEML6075_IT_400MS:
        delay(50*1.1);
        break;
      case VEML6075_IT_800MS:
        delay(50*1.1);
        break;
    }
  }
  uint16_t uva = this->read_uva();
  uint16_t uvb = this->read_uvb();
  uint16_t uvcomp1 = this->read_comp1();
  uint16_t uvcomp2 = this->read_comp2();

  float _uva_calc = uva - (this->_uva_a * uvcomp1) - (this->_uva_b * uvcomp2);
  if (this->uva_sensor_ != nullptr) {
    this->uva_sensor_->publish_state(_uva_calc);
  }
  float _uvb_calc = uvb - (this->_uvb_c * uvcomp1) - (this->_uvb_d * uvcomp2);
  if (this->uvb_sensor_ != nullptr) {
    this->uvb_sensor_->publish_state(_uvb_calc);
  }
  float uvi = ((_uva_calc * this->_uva_resp) + (_uvb_calc * this->_uvb_resp)) / 2;
  if (this->uvi_sensor_ != nullptr) {
    this->uvi_sensor_->publish_state(uvi);
  }
}

void VEML6075Component::set_coefficients(float UVA_A,
                                         float UVA_B,
                                         float UVB_C,
                                         float UVB_D,
                                         float UVA_response,
                                         float UVB_response) {
  this->_uva_a = UVA_A;
  this->_uva_b = UVA_B;
  this->_uvb_c = UVB_C;
  this->_uvb_d = UVB_D;
  this->_uva_resp = UVA_response;
  this->_uvb_resp = UVB_response;
}

void VEML6075Component::read_config(VEML6075IntegrationTime *time, VEML6075DynamicSetting *dsetting,
                                    VEML6075Trigger *trigger, VEML6075ActiveForceMode *force_mode) {
  uint16_t data = this->read_data(REG_UV_CONF);
  std::bitset<8> conf = data & 0xFF;
  if (!conf[6] && !conf[5] && !conf[4]) {
    *time = VEML6075_IT_50MS;
  } else if (!conf[6] && !conf[5] && conf[4]) {
    *time = VEML6075_IT_100MS;
  } else if (!conf[6] && conf[5] && !conf[4]) {
    *time = VEML6075_IT_200MS;
  } else if (!conf[6] && conf[5] && conf[4]) {
    *time = VEML6075_IT_400MS;
  } else if (conf[6] && !conf[5] && !conf[4]) {
    *time = VEML6075_IT_800MS;
  }
  if (conf[3]) {
    *dsetting = VEML6075_HIGH_DYNAMIC;
  } else {
    *dsetting = VEML6075_NORMAL_DYNAMIC;
  }
  if (conf[2]) {
    *trigger = VEML6075_TRIGGER_ONCE;
  } else {
    *trigger = VEML6075_TRIGGER_NONE;
  }
  if (conf[1]) {
    *force_mode = VEML6075_ACTIVE_FORCE_MODE_ENABLE;
  } else {
    *force_mode = VEML6075_ACTIVE_FORCE_MODE_DISABLE;
  }
}

void VEML6075Component::write_config(VEML6075IntegrationTime time, VEML6075DynamicSetting dsetting,
                                     VEML6075Trigger trigger, VEML6075ActiveForceMode force_mode) {
  std::bitset<8> config;
  switch (time) {
    case VEML6075_IT_50MS:
      config[6] = false;
      config[5] = false;
      config[4] = false;
      break;
    case VEML6075_IT_100MS:
      config[6] = false;
      config[5] = false;
      config[4] = true;
      break;
    case VEML6075_IT_200MS:
      config[6] = false;
      config[5] = true;
      config[4] = false;
      break;
    case VEML6075_IT_400MS:
      config[6] = false;
      config[5] = true;
      config[4] = true;
      break;
    case VEML6075_IT_800MS:
      config[6] = true;
      config[5] = false;
      config[4] = false;
      break;
  }
  switch (dsetting) {
    case VEML6075_NORMAL_DYNAMIC:
      config[3] = false;
      break;
    case VEML6075_HIGH_DYNAMIC:
      config[3] = true;
      break;
  }
  switch (trigger) {
    case VEML6075_TRIGGER_NONE:
      config[2] = false;
      break;
    case VEML6075_TRIGGER_ONCE:
      config[2] = true;
      break;
  }
  switch (force_mode) {
    case VEML6075_ACTIVE_FORCE_MODE_DISABLE:
      config[1] = false;
      break;
    case VEML6075_ACTIVE_FORCE_MODE_ENABLE:
      config[1] = true;
      break;
  }
  // ignore sd, aka power on/shut down
  uint16_t data = (uint16_t) config.to_ulong();
  this->send_command(REG_UV_CONF, data);
}

uint16_t VEML6075Component::read_uva() { return this->read_data(REG_UVA); }

uint16_t VEML6075Component::read_uvb() { return this->read_data(REG_UVB); }

uint16_t VEML6075Component::read_comp1() { return this->read_data(REG_UVCOMP1); }

uint16_t VEML6075Component::read_comp2() { return this->read_data(REG_UVCOMP2); }

uint16_t VEML6075Component::read_id() { return this->read_data(REG_ID); }

i2c::ErrorCode VEML6075Component::send_command(uint8_t command, uint16_t data) {
  uint8_t buffer[3] = {command, (uint8_t) (data & 0xFF), (uint8_t) ((data >> 8) & 0xFF)};
  return this->write(buffer, 3);
}

uint16_t VEML6075Component::read_data(uint8_t command) {
  uint8_t data[2];
  this->write_read(&command, 1, data, 2);
//  this->write(&command, 1, false);  // 发送命令
//  this->read(data, 2);                                    // 读取2字节数据
  return ((uint16_t) data[0]) | ((uint16_t) data[1]) << 8;  // 返回
}

} // namespace esphome::veml6075
