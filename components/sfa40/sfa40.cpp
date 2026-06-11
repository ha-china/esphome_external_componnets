#include "sfa40.h"
#include "esphome/core/log.h"

namespace esphome::sfa40 {

static const char *const TAG = "sfa40";
static const uint16_t SFA40_CMD_START = 0x00AC;
static const uint16_t SFA40_CMD_READ = 0xE06D;
static const uint16_t SFA40_CMD_STOP = 0x50D2;
static const uint16_t SFA40_CMD_ID = 0x02CE;

void SFA40Component::setup() {
  // Serial Number identification
  uint16_t raw_device_marking[3];
  if (!this->get_register(SFA40_CMD_ID, raw_device_marking, 3, 5)) {
    ESP_LOGE(TAG, "Failed to read device marking");
    this->error_code_ = DEVICE_MARKING_READ_FAILED;
    this->mark_failed();
    return;
  }

  for (size_t i = 0; i < 3; i++) {
    this->device_marking_[i * 2] = static_cast<char>(raw_device_marking[i] >> 8);
    this->device_marking_[i * 2 + 1] = static_cast<char>(raw_device_marking[i] & 0xFF);
  }
  ESP_LOGD(TAG, "Device Marking: '%s'", this->device_marking_);

  if (!this->write_command(SFA40_CMD_START)) { // data should be crc, here no data hence no crc
    ESP_LOGE(TAG, "Error starting measurements.");
    this->error_code_ = MEASUREMENT_INIT_FAILED;
    this->mark_failed();
    return;
  }
}

void SFA40Component::dump_config() {
  ESP_LOGCONFIG(TAG, "sfa40:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    switch (this->error_code_) {
      case DEVICE_MARKING_READ_FAILED:
        ESP_LOGW(TAG, "Unable to read device marking!");
        break;
      case MEASUREMENT_INIT_FAILED:
        ESP_LOGW(TAG, "Measurement initialization failed!");
        break;
      default:
        ESP_LOGW(TAG, "Unknown setup error!");
        break;
    }
  }
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Device Marking: '%s'", this->device_marking_);
  LOG_SENSOR("  ", "Formaldehyde", this->formaldehyde_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void SFA40Component::update() {
  if (!this->write_command(SFA40_CMD_READ)) {
    ESP_LOGW(TAG, "Error reading measurement!");
    this->status_set_warning();
    return;
  }

  this->set_timeout(5, [this]() {
    uint16_t raw_data[4];
    if (!this->read_data(raw_data, 4)) {
      ESP_LOGW(TAG, "Error reading measurement data!");
      this->status_set_warning();
      return;
    }

    if (this->formaldehyde_sensor_ != nullptr) {
      const float formaldehyde = static_cast<int16_t>(raw_data[0]) / 10.0f;
      this->formaldehyde_sensor_->publish_state(formaldehyde);
    }

    if (this->humidity_sensor_ != nullptr) {
      const float humidity = 125.0 * (static_cast<int16_t>(raw_data[1]) / 65535.0f) - 6;
      this->humidity_sensor_->publish_state(humidity);
    }

    if (this->temperature_sensor_ != nullptr) {
      const float temperature = 175.0 * (static_cast<int16_t>(raw_data[2]) / 65535.0f) - 45;
      this->temperature_sensor_->publish_state(temperature);
    }
    this->status_clear_warning();
  });
}

} // namespace esphome::sfa40
