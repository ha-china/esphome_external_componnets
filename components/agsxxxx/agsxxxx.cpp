#include "agsxxxx.h"
#include <bitset>
#include "esphome/core/log.h"

namespace esphome::agsxxxx {

static const char *const TAG = "agsxxxx";
static const uint8_t REG_VERSION = 0x11;  // 版本寄存器地址
static const uint8_t REG_DATA = 0x00; // 浓度
static const uint8_t REG_RESISTER = 0x20; // 阻值地址
static const uint8_t REG_CALIBRATE = 0x01; // 校准寄存器地址

uint8_t agsxxxx_crc8(const uint8_t *dat, uint8_t size) {
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

void AGSXXXXComponent::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void AGSXXXXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AGSXXXX:\n"
                "  Version: %d\n"
                     "  Type: %s\n", this->get_version(), this->type_.c_str());
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "GAS SENSOR", this->gas_sensor_);
}

void AGSXXXXComponent::update() {
  if (this->gas_sensor_ != nullptr) {
    this->write(&REG_DATA, 1);
    uint8_t data[5];
    this->read(data, 5);
    uint8_t crc = agsxxxx_crc8(data, 4);
    if (crc != data[4]) {
      ESP_LOGW(TAG, "AGSxxxx CRC error: expected %02X, got %02X", crc, data[4]);
      this->status_set_warning();
      return;  // CRC error
    }
    std::bitset<8> status = data[0];
    if (status[0]) {
      ESP_LOGW(TAG, "AGSxxxx sensor not ready");
      return;  // Sensor not ready
    }
    uint32_t co = (((uint32_t)data[1]) << 16) | (((uint32_t)data[2]) << 8) | ((uint32_t)data[3]);  // 解析CO浓度数据
    this->gas_sensor_->publish_state(co);
  }
  if (this->resistor_sensor_ != nullptr) {
    this->write(&REG_RESISTER, 1);
    uint8_t data[5];
    this->read(data, 5);
    uint8_t crc = agsxxxx_crc8(data, 4);
    if (crc != data[4]) {
      ESP_LOGW(TAG, "AGSxxxx CRC error: expected %02X, got %02X", crc, data[4]);
      this->status_set_warning();
      return;  // CRC error
    }
    uint32_t resistor = (((uint32_t)data[0]) << 16) | (((uint32_t)data[1]) << 8) | ((uint32_t)data[2]);  // 解析阻值数据 todo is this right？
    // https://github.com/RobTillaart/Arduino/blob/48a03abc5948770150802e773848eb8266718969/libraries/AGSxxxx/AGSxxxx.cpp
    this->resistor_sensor_->publish_state(resistor*10);
  }
  this->status_clear_warning();
}

void AGSXXXXComponent::calibrate(uint16_t mode) {
  uint8_t data[5] = {0x00, 0x0C, ( uint8_t )((mode>>8)&0xFF), ( uint8_t )(mode&0xFF), 0x00}; // 初始化数据
  data[4] = agsxxxx_crc8(data, 4);
  this->write_register(REG_CALIBRATE, data, 5); // 写入校准寄存器
}

int AGSXXXXComponent::get_version() {
  this->write(&REG_VERSION, 1);
  uint8_t data[5];
  this->read(data, 5);
  uint8_t crc = agsxxxx_crc8(data, 4);
  if (crc != data[4]) {
    ESP_LOGW(TAG, "AGSxxxx CRC error: expected %02X, got %02X", crc, data[4]);
    this->status_set_warning();
    return -1;  // CRC error
  }
  int version = ((int)data[0] << 24) | ((int)data[1] << 16) | ((int)data[2] << 8) | ((int)data[3]);
  this->status_clear_warning();
  return version;
}

} // namespace esphome::agsxxxx

