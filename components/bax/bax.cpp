#include "bax.h"
#include "esphome/core/log.h"

namespace esphome::bax {

static const char *const TAG = "bax";

static const uint8_t BA234_DETECT_CMD[6] = {0xA0, 0x00, 0x00, 0x00, 0x00, 0xA0};                // ba234 ba121 ba111 ba012 ba022 bat2u bat3u
static const uint8_t BA234_ZERO_POINT_CALIBRATE_CMD[6] = {0xA1, 0x00, 0x00, 0x00, 0x00, 0xA1};  // ba234
static const uint8_t BA121_BASELINE_CALIBRATE_CMD[6] = {0xA6, 0x00, 0x00, 0x00, 0x00, 0xA6};    // ba121 ba111 ba012 ba022 bat2u

uint8_t bax_crc8(const uint8_t *dat, uint8_t size) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < size; i++) {
    sum += dat[i];
  }
  return sum;
}

void BAXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "%s:", this->get_name());
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "TDS", this->tds_sensor_);
  LOG_SENSOR("  ", "EC", this->ec_sensor_);
  LOG_SENSOR("  ", "Salinity", this->salinity_sensor_);
  LOG_SENSOR("  ", "Specific Gravity", this->specific_gravity_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Hardness", this->hardness_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "Zero Point Calibrate", this->zero_point_calibrate_button_);
#endif
  this->check_uart_settings(9600);
}

const char *BAXComponent::get_name() {
  const char *name;
  switch (this->type_) {
    case BAX_TYPE::BA012:
      name = "BA012";
      break;
    case BAX_TYPE::BA022:
      name = "BA022";
      break;
    case BAX_TYPE::BA111:
      name = "BA111";
      break;
    case BAX_TYPE::BA121:
      name = "BA121";
      break;
    case BAX_TYPE::BA234:
      name = "BA234";
      break;
    case BAX_TYPE::BA311:
      name = "BA311";
      break;
    case BAX_TYPE::BAT3U:
      name = "BAT3U";
      break;
    default:
      name = "Unknown";
      break;
  }
  return name;
}

void BAXComponent::setup() {
  const char *name = this->get_name();
  ESP_LOGCONFIG(TAG, "Setting up %s...", name);
}

void BAXComponent::start_measurement() {
  switch (this->type_) {
    case BAX_TYPE::BA012:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));
      break;
    case BAX_TYPE::BA022:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));
      break;
    case BAX_TYPE::BA111:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));
      break;
    case BAX_TYPE::BA121:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));
      break;
    case BAX_TYPE::BA234:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));  // todo 每种传感器都是这个吗
      break;
    case BAX_TYPE::BA311:
      // BA311是自动上传的，不需要发送测量指令
      break;
    case BAX_TYPE::BAT3U:
      this->write_array(BA234_DETECT_CMD, sizeof(BA234_DETECT_CMD));
      break;
  }
}

void BAXComponent::zero_point_calibrate() {
  switch (this->type_) {
    case BAX_TYPE::BA012:
      // 两个通道同时校准
      this->write_array(BA121_BASELINE_CALIBRATE_CMD, sizeof(BA121_BASELINE_CALIBRATE_CMD));
      break;
    case BAX_TYPE::BA022:
      this->write_array(BA121_BASELINE_CALIBRATE_CMD, sizeof(BA121_BASELINE_CALIBRATE_CMD));
      break;
    case BAX_TYPE::BA111:
      this->write_array(BA121_BASELINE_CALIBRATE_CMD, sizeof(BA121_BASELINE_CALIBRATE_CMD));
      break;
    case BAX_TYPE::BA121:
      this->write_array(BA121_BASELINE_CALIBRATE_CMD, sizeof(BA121_BASELINE_CALIBRATE_CMD));
      break;
    case BAX_TYPE::BA234:
      this->write_array(BA234_ZERO_POINT_CALIBRATE_CMD,
                        sizeof(BA234_ZERO_POINT_CALIBRATE_CMD));  // todo 每种传感器都是这个吗
      break;
    case BAX_TYPE::BA311:
      break;
    case BAX_TYPE::BAT3U:
      this->write_array(BA121_BASELINE_CALIBRATE_CMD, sizeof(BA121_BASELINE_CALIBRATE_CMD));
      break;
  }
}

void BAXComponent::twice_calibrate(float sal) {
  switch (this->type_) {
    case BAX_TYPE::BA012:
      ESP_LOGD(TAG, "Zero point calibrating BA012...");
      break;
    case BAX_TYPE::BA022:
      break;
    case BAX_TYPE::BA111:
      break;
    case BAX_TYPE::BA121: {
      break;
    }
    case BAX_TYPE::BA234: {
      // 4%浓度就输入4.0 以此类推
      uint16_t s = sal * 100;
      uint8_t cmd[6] = {0xA7, (uint8_t) ((s >> 8) & 0xFF), (uint8_t) (s & 0xFF), 0x00, 0x00, 0x00};
      uint8_t crc = bax_crc8(cmd, 5);
      cmd[5] = crc;
      this->write_array(cmd, 6);
      break;
    }
    case BAX_TYPE::BA311:
      break;
    case BAX_TYPE::BAT3U:
      break;
  }
}

void BAXComponent::set_ntc_resistance(uint32_t resistance) {
  switch (this->type_) {
    case BAX_TYPE::BA012: {
      uint8_t data[6] = {0xA3,
                         (uint8_t) ((resistance >> 24) & 0xFF),
                         (uint8_t) ((resistance >> 16) & 0xFF),
                         (uint8_t) ((resistance >> 8) & 0xFF),
                         (uint8_t) (resistance & 0xFF),
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA022: {
      uint8_t data[6] = {0xA3,
                         (uint8_t) ((resistance >> 24) & 0xFF),
                         (uint8_t) ((resistance >> 16) & 0xFF),
                         (uint8_t) ((resistance >> 8) & 0xFF),
                         (uint8_t) (resistance & 0xFF),
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA111: {
      uint8_t data[6] = {0xA3,
                         (uint8_t) ((resistance >> 24) & 0xFF),
                         (uint8_t) ((resistance >> 16) & 0xFF),
                         (uint8_t) ((resistance >> 8) & 0xFF),
                         (uint8_t) (resistance & 0xFF),
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA121: {
      uint8_t data[6] = {0xA3,
                         (uint8_t) ((resistance >> 24) & 0xFF),
                         (uint8_t) ((resistance >> 16) & 0xFF),
                         (uint8_t) ((resistance >> 8) & 0xFF),
                         (uint8_t) (resistance & 0xFF),
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA234:
      break;
    case BAX_TYPE::BA311:
      break;
    case BAX_TYPE::BAT3U: {
      uint8_t data[6] = {0xA3,
                         (uint8_t) ((resistance >> 24) & 0xFF),
                         (uint8_t) ((resistance >> 16) & 0xFF),
                         (uint8_t) ((resistance >> 8) & 0xFF),
                         (uint8_t) (resistance & 0xFF),
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
  }
}

void BAXComponent::set_ntc_b_value(uint16_t b_value) {
  switch (this->type_) {
    case BAX_TYPE::BA012: {
      uint8_t data[6] = {0xA5,
                         (uint8_t) ((b_value >> 8) & 0xFF),
                         (uint8_t) (b_value & 0xFF),
                         0x00,
                         0x00,
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA022: {
      uint8_t data[6] = {0xA5,
                         (uint8_t) ((b_value >> 8) & 0xFF),
                         (uint8_t) (b_value & 0xFF),
                         0x00,
                         0x00,
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA111: {
      uint8_t data[6] = {0xA5,
                         (uint8_t) ((b_value >> 8) & 0xFF),
                         (uint8_t) (b_value & 0xFF),
                         0x00,
                         0x00,
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA121: {
      uint8_t data[6] = {0xA5,
                         (uint8_t) ((b_value >> 8) & 0xFF),
                         (uint8_t) (b_value & 0xFF),
                         0x00,
                         0x00,
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
    case BAX_TYPE::BA234:
      break;
    case BAX_TYPE::BA311:
      break;
    case BAX_TYPE::BAT3U: {
      uint8_t data[6] = {0xA5,
                         (uint8_t) ((b_value >> 8) & 0xFF),
                         (uint8_t) (b_value & 0xFF),
                         0x00,
                         0x00,
                         0x00};
      uint8_t crc = bax_crc8(data, 5);
      data[5] = crc;
      this->write_array(data, 6);
      break;
    }
  }
}

void BAXComponent::update() { this->start_measurement(); }

void BAXComponent::loop() {
  while (this->available()) {
    this->buffer_.push_back(static_cast<uint8_t>(this->read()));
  }
  if (this->buffer_.empty()) {
    return;  // not full frame
  }
  switch (this->type_) {
    case BAX_TYPE::BA012: {
      if (this->buffer_[0] == 0xAA) {
        // tds检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t tds = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->tds_sensor_ != nullptr) {
          this->tds_sensor_->publish_state(static_cast<float>(tds));
        }
        uint16_t tds2 = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->tds2_sensor_ != nullptr) {
          this->tds2_sensor_->publish_state(static_cast<float>(tds2));
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAB) {
        // temperature检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t temp = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 100.0);
        }
        uint16_t temp2 = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->temperature2_sensor_ != nullptr) {
          this->temperature2_sensor_->publish_state(static_cast<float>(temp2) / 100.0);
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else if (this->buffer_[1] == 0x03) {
          ESP_LOGW(TAG, "calibration fail");
        } else {
          ESP_LOGW(TAG, "temperature out of range");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      }
      break;
    }
    case BAX_TYPE::BA022: {
      if (this->buffer_[0] == 0xAA) {
        // ec检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t ec = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->ec_sensor_ != nullptr) {
          this->ec_sensor_->publish_state(static_cast<float>(ec));
        }
        uint16_t ec2 = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->ec2_sensor_ != nullptr) {
          this->ec2_sensor_->publish_state(static_cast<float>(ec2));
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAB) {
        // temperature检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t temp = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 100.0);
        }
        uint16_t temp2 = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->temperature2_sensor_ != nullptr) {
          this->temperature2_sensor_->publish_state(static_cast<float>(temp2) / 100.0);
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else if (this->buffer_[1] == 0x03) {
          ESP_LOGW(TAG, "calibration fail");
        } else {
          ESP_LOGW(TAG, "temperature out of range");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      }
      break;
    }
    case BAX_TYPE::BA111: {
      if (this->buffer_[0] == 0xAA) {
        // 检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t tds = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->tds_sensor_ != nullptr) {
          this->tds_sensor_->publish_state(static_cast<float>(tds));
        }
        uint16_t temp = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 100.0);
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else if (this->buffer_[1] == 0x03) {
          ESP_LOGW(TAG, "calibration fail");
        } else {
          ESP_LOGW(TAG, "temperature out of range");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      }
      break;
    }
    case BAX_TYPE::BA121: {
      if (this->buffer_[0] == 0xAA) {
        // 检测结果
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t ec = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->ec_sensor_ != nullptr) {
          this->ec_sensor_->publish_state(static_cast<float>(ec));
        }
        uint16_t temp = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 100.0);
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else if (this->buffer_[1] == 0x03) {
          ESP_LOGW(TAG, "calibration fail");
        } else {
          ESP_LOGW(TAG, "temperature out of range");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      }
      break;
    }
    case BAX_TYPE::BA234: {
      if (this->buffer_[0] == 0xAA) {
        // 检测结果
        if (this->buffer_.size() < 16) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 15);
        if (this->buffer_[15] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[15]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t tds = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->tds_sensor_ != nullptr) {
          this->tds_sensor_->publish_state(static_cast<float>(tds));
        }
        uint32_t ec = (((uint32_t) this->buffer_[3]) << 24) | (((uint32_t) this->buffer_[4]) << 16) |
                      (((uint32_t) this->buffer_[5]) << 8) | ((uint32_t) this->buffer_[6]);
        if (this->ec_sensor_ != nullptr) {
          this->ec_sensor_->publish_state(static_cast<float>(ec));
        }
        uint16_t sal = (((uint16_t) this->buffer_[7]) << 8) | ((uint16_t) this->buffer_[8]);
        if (this->salinity_sensor_ != nullptr) {
          this->salinity_sensor_->publish_state(static_cast<float>(sal) / 100.0);
        }
        uint16_t sg = (((uint16_t) this->buffer_[9]) << 8) | ((uint16_t) this->buffer_[10]);
        if (this->specific_gravity_sensor_ != nullptr) {
          this->specific_gravity_sensor_->publish_state(static_cast<float>(sg) / 10000.0);
        }
        uint16_t temp = (((uint16_t) this->buffer_[11]) << 8) | ((uint16_t) this->buffer_[12]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 10.0);
        }
        uint16_t har = (((uint16_t) this->buffer_[13]) << 8) | ((uint16_t) this->buffer_[14]);
        if (this->hardness_sensor_ != nullptr) {
          this->hardness_sensor_->publish_state(static_cast<float>(har));
        }

        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 16);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else {
          ESP_LOGW(TAG, "calibration fail");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      } else {
        ESP_LOGW(TAG, "unknown frame header for ba234: %02X", this->buffer_[0]);
        this->buffer_.clear();
        return;
      }
      break;
    }
    case BAX_TYPE::BA311: {
      if (this->buffer_.size() < 6) {
        return;  // not full frame
      }
      if (this->buffer_[0] != 0xAA) {
        ESP_LOGW(TAG, "wrong header: expected %02X, got %02X", 0xAA, this->buffer_[0]);
        this->buffer_.clear();
        return;
      }
      uint8_t crc = bax_crc8(this->buffer_.data(), 5);
      if (this->buffer_[5] != crc) {
        ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
        this->buffer_.clear();
        return;
      }
      uint16_t tds = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
      if (this->tds_sensor_ != nullptr) {
        this->tds_sensor_->publish_state(static_cast<float>(tds));
      }
      this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
      break;
    }
    case BAX_TYPE::BAT3U: {
      if (this->buffer_[0] == 0xAA) {
        // 3通道检测结果
        if (this->buffer_.size() < 14) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 13);
        if (this->buffer_[13] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[13]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        uint16_t tds = (((uint16_t) this->buffer_[1]) << 8) | ((uint16_t) this->buffer_[2]);
        if (this->tds_sensor_ != nullptr) {
          this->tds_sensor_->publish_state(static_cast<float>(tds));
        }
        uint16_t temp = (((uint16_t) this->buffer_[3]) << 8) | ((uint16_t) this->buffer_[4]);
        if (this->temperature_sensor_ != nullptr) {
          this->temperature_sensor_->publish_state(static_cast<float>(temp) / 100.0);
        }
        // channel 2
        uint16_t tds2 = (((uint16_t) this->buffer_[5]) << 8) | ((uint16_t) this->buffer_[6]);
        if (this->tds2_sensor_ != nullptr) {
          this->tds2_sensor_->publish_state(static_cast<float>(tds2));
        }
        uint16_t temp2 = (((uint16_t) this->buffer_[7]) << 8) | ((uint16_t) this->buffer_[8]);
        if (this->temperature2_sensor_ != nullptr) {
          this->temperature2_sensor_->publish_state(static_cast<float>(temp2) / 100.0);
        }
        // channel 3
        uint16_t tds3 = (((uint16_t) this->buffer_[9]) << 8) | ((uint16_t) this->buffer_[10]);
        if (this->tds3_sensor_ != nullptr) {
          this->tds3_sensor_->publish_state(static_cast<float>(tds3));
        }
        uint16_t temp3 = (((uint16_t) this->buffer_[11]) << 8) | ((uint16_t) this->buffer_[12]);
        if (this->temperature3_sensor_ != nullptr) {
          this->temperature3_sensor_->publish_state(static_cast<float>(temp3) / 100.0);
        }

        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 14);  // remove the processed frame
        return;
      } else if (this->buffer_[0] == 0xAC) {
        // 状态返回指令
        if (this->buffer_.size() < 6) {
          return;  // not full frame
        }
        uint8_t crc = bax_crc8(this->buffer_.data(), 5);
        if (this->buffer_[5] != crc) {
          ESP_LOGW(TAG, "CRC mismatch: expected %02X, got %02X", crc, this->buffer_[5]);
          this->buffer_.clear();  // discard the first byte and try again
          return;
        }
        if (this->buffer_[1] == 0x00) {
          ESP_LOGI(TAG, "command return ok");
        } else if (this->buffer_[1] == 0x01) {
          ESP_LOGW(TAG, "command return error");
        } else if (this->buffer_[1] == 0x02) {
          ESP_LOGW(TAG, "busy");
        } else if (this->buffer_[1] == 0x03) {
          ESP_LOGW(TAG, "calibration fail");
        } else {
          ESP_LOGW(TAG, "temperature out of range");
        }
        this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 6);  // remove the processed frame
        return;
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace esphome::bax
