#include "agr12.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::agr12 {

static const char *const TAG = "agr12";
static const uint8_t READ_CMD[2] = {0xAC, 0x12};  // 读取命令

void AGR12Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  // 设备初始化代码
}

void AGR12Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AGR12:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Pressure Sensor", this->pressure_sensor_);
}

void AGR12Component::update() {
  if (this->pressure_sensor_ != nullptr) {
    this->write(READ_CMD, 2);  // 发送读取命令
    delay(80);  // 等待传感器响应
    // 读取压力传感器数据
    uint8_t data[3];
    this->read(data, 3);
    uint8_t crc = data[0] ^ data[1];
    if(crc!=data[2]) {
      ESP_LOGW(TAG, "AGR12 CRC error: expected %02X, got %02X", crc, data[2]);
      this->status_set_warning();
      return;  // CRC错误
    }
    int32_t pressure = ((int32_t)data[0] << 8) | ((int32_t)data[1]);
    float frac = 10.0f;
    if (this->type_==AGR12) {
      frac = 10.0f;
    } else {
      frac = 100.0f;
    }

    if (data[0] & 0x80) {
      // 负压
      pressure = pressure & 0x7FFF;  // 清除符号位
      this->pressure_sensor_->publish_state((float)(pressure-32768) / frac);
    } else {
      this->pressure_sensor_->publish_state((float) pressure / frac);
    }
    this->status_clear_warning();
  }
}

}
