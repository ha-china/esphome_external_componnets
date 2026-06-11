#include "dht30.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::dht30 {

static const char *const TAG = "dht30";
static const uint8_t READ_CMD[3] = {0xAC, 0x33, 0x00};  // Read command

uint8_t dht30_crc8(const uint8_t *dat, uint8_t size) {
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

void DHT30Component::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void DHT30Component::dump_config() {
  ESP_LOGCONFIG(TAG, "DHT30:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity Sensor", this->humidity_sensor_);
}

void DHT30Component::update() {
  this->write(READ_CMD, 3);
  uint8_t data[7];
  delay(80);
  this->read(data, 7);
  uint8_t crc = dht30_crc8(data, 6);
  if (crc != data[6]) {
    ESP_LOGW(TAG, "DHT30 CRC error: expected %02X, got %02X", crc, data[6]);
    return;  // CRC error
  }

  std::bitset<8> status = data[0];  // 状态寄存器
  if (status[2]) {
    // cmp interrupt
    return;
  }
  if (status[7]) {
    return;  // busy sensor
  }

  uint32_t rh = ((uint32_t) data[1]) << 12 | ((uint32_t) data[2]) << 4 | (uint32_t) (data[3] >> 4);
  float rhf = (float) rh / (pow(2, 20));  // 相对湿度计算
  if (this->humidity_sensor_ != nullptr) {
    this->humidity_sensor_->publish_state(rhf);  // 发布湿度数据
  }
  int32_t temp = ((int32_t) (data[3] & 0x0F)) << 16 | ((int32_t) data[4]) << 8 | (int32_t) data[5];
  float tempf = (float) temp / (pow(2, 20)) * 200 - 50;  // 温度计算
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(tempf);  // 发布温度数据
  }
}

} // namespace esphome::dht30

