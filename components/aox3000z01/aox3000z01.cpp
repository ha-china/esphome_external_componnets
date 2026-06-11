#include "aox3000z01.h"
#include "esphome/core/log.h"

namespace esphome::aox3000z01 {

static const char *const TAG = "aox3000z01";
static const uint8_t A0X3000Z01_RESPONSE_LENGTH = 12;

uint8_t aox3000_checksum(const uint8_t *command) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < 11; i++) {
    sum += command[i];
  }
  sum = (~sum) + 1;
  return sum;
}

void AOX3000Z01Component::setup() { ESP_LOGCONFIG(TAG, "Running setup"); }

void AOX3000Z01Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AOX3000Z01:");
  LOG_SENSOR("  ", "O2 Sensor", this->o2_sensor_);
  this->check_uart_settings(2400);
}

void AOX3000Z01Component::update() {
  uint8_t peeked;
  while (this->available() && this->peek_byte(&peeked) && peeked != 0x78) {
    this->read();
  }

  bool read_success = this->read_array(this->buffer, A0X3000Z01_RESPONSE_LENGTH);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from AOX3000Z01 failed!");
    this->status_set_warning();
    return;
  }

  if (this->buffer[0] != 0x78 || this->buffer[1] != 0x09) {
    ESP_LOGW(TAG, "Invalid preamble for AOX3000Z01 response!");
    this->status_set_warning();
    return;
  }
  if (this->buffer[9] != 0x00 || this->buffer[10] != 0x00) {
    ESP_LOGW(TAG, "Invalid trailer for AOX3000Z01 response!");
    this->status_set_warning();
    return;
  }
  uint8_t fcc = aox3000_checksum(this->buffer);
  if (fcc != this->buffer[11]) {
    ESP_LOGW(TAG, "AOX3000Z01 Checksum doesn't match: 0x%02X!=0x%02X", this->buffer[11], fcc);
    this->status_set_warning();
    return;
  }
  uint8_t status = this->buffer[8];
  if (status == 0x01) {
    ESP_LOGE(TAG, "AOX3000Z01 Error");
    return;
  } else if (status == 0x02) {
    ESP_LOGI(TAG, "AOX3000Z01 is heating");
    return;
  }
  uint16_t o2 = ((uint16_t) this->buffer[2]) << 8 | (uint16_t) this->buffer[3];
  if (this->o2_sensor_ != nullptr) {
    this->o2_sensor_->publish_state(((float) o2) / 10.0f);
  }
  this->status_clear_warning();
}

} // namespace esphome::aox3000z01
