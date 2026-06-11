#include "ld8001.h"
#include "esphome/core/log.h"

namespace esphome::ld8001 {

static const char *const TAG = "ld8001";
static const uint8_t SOF = 0x01;

uint8_t ld8001_checksum(uint8_t *data, size_t len) {
  uint8_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum = sum ^ data[i];
  }
  return 0xFF ^ sum;
}

void LD8001Component::setup() { ESP_LOGCONFIG(TAG, "Setting up LD8001..."); }

void LD8001Component::dump_config() {
  ESP_LOGCONFIG(TAG, "LD8001:");
  LOG_SENSOR("  ", "Distance Sensor", this->distance_sensor_);
  this->check_uart_settings(115200);
}

void LD8001Component::loop() {
  uint8_t peeked;
  while (this->available()) {
    if (!this->head_found && this->peek_byte(&peeked) && peeked != SOF) {
      this->read();
    } else {
      this->head_found = true;
    }
    if (this->head_found) {
      this->receive_buffer.push_back((uint8_t) this->read());
    }
  }
  if (this->receive_buffer.size() < 8) {
    return;  // Not enough data to process
  }
  if (ld8001_checksum(this->receive_buffer.data(), 7) != this->receive_buffer[7]) {
    ESP_LOGW(TAG, "Received data has invalid head checksum: %02X", this->receive_buffer[7]);
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  uint16_t frame_size = ((uint16_t) this->receive_buffer[3] << 8) | ((uint16_t) (this->receive_buffer[4]));  // 4
  if (this->receive_buffer.size() < frame_size + 9) {
    return;  // Not enough data to process
  }
  uint8_t data_checksum = this->receive_buffer[8 + frame_size];
  if (data_checksum != ld8001_checksum(this->receive_buffer.data() + 8, frame_size)) {
    ESP_LOGW(TAG, "Received data has invalid data checksum: %02X", data_checksum);
    this->receive_buffer.clear();
    this->head_found = false;
    return;
  }
  uint16_t type_ = ((uint16_t) this->receive_buffer[5] << 8) | ((uint16_t) (this->receive_buffer[6]));  // 5,6
  switch (type_) {
    case 0x0A0C: {  // 报告距离结果
      float myfloat;
      memcpy(&myfloat, this->receive_buffer.data() + 8, 4);
      if (this->distance_sensor_ != nullptr) {
        this->distance_sensor_->publish_state(myfloat);
      }
      break;
    }
    case 0x2D09: {  // 向上位机发送第一次FFT 之后的数据
      float pointI, pointQ;
      uint16_t pointnum = frame_size / 8;  // 8 bytes per point
      for (uint16_t i = 0; i < pointnum; i++) {
        memcpy(&pointI, this->receive_buffer.data() + 8 + i * 8, 4);
        memcpy(&pointQ, this->receive_buffer.data() + 12 + i * 8, 4);
        float A = sqrt(pointI * pointI + pointQ * pointQ);
        if (i == 0 || i == pointnum / 2) {
          A = A / pointnum;
        } else {
          A = A * 2 / pointnum;
        }
        float DB = 20 * log10(A);
      }
      // ... and other points, 1024 of them, that's too much, let's just ignore for now
      break;
    }
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + frame_size + 9);
}

} // namespace esphome::ld8001
