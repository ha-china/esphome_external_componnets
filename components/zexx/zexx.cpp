#include "zexx.h"
#include "esphome/core/log.h"

namespace esphome::zexx {

static const char *const TAG = "zexx";
static const uint8_t ZEXX_REQUEST_LENGTH = 8;
static const uint8_t ZEXX_RESPONSE_LENGTH = 9;

static const uint8_t ZEXX_COMMAND_GET_PPM[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00};  // 问答模式下，请求发送
static const uint8_t ZEXX_COMMAND_SET_QA[] = {0xFF, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00};  // 切换到问答模式
static const uint8_t ZEXX_COMMAND_SET_NQA[] = {0xFF, 0x01, 0x78, 0x40, 0x00, 0x00, 0x00, 0x00};  // 切换到主动上传模式

uint8_t zexx_checksum(const uint8_t *command) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < ZEXX_REQUEST_LENGTH; i++) {
    sum += command[i];
  }
  sum = (~sum) + 1;
  return sum;
}

void ZEXXComponent::setup() {
  uint8_t response[ZEXX_RESPONSE_LENGTH];
  if (this->mode_ == ZEXX_MODE_PASSIVE) {
    if (!this->write_command_(ZEXX_COMMAND_SET_QA, response)) {
      ESP_LOGW(TAG, "Reading data from ZEXX failed!");
      this->status_set_warning();
      return;
    }
  }
}

void ZEXXComponent::update() {
  if (this->mode_ == ZEXX_MODE_PASSIVE) {
    uint8_t response[ZEXX_RESPONSE_LENGTH];
    if (!this->write_command_(ZEXX_COMMAND_GET_PPM, response)) {
      ESP_LOGW(TAG, "Reading data from ZEXX failed!");
      this->status_set_warning();
      return;
    }

    if (response[0] != 0xFF || response[1] != 0x86) {
      ESP_LOGW(TAG, "Invalid preamble from ZEXX!");
      this->status_set_warning();
      return;
    }

    uint8_t checksum = zexx_checksum(response);
    if (response[8] != checksum) {
      ESP_LOGW(TAG, "ZEXX Checksum doesn't match: 0x%02X!=0x%02X", response[8], checksum);
      this->status_set_warning();
      return;
    }

    this->status_clear_warning();
    const uint16_t gas_ppb = uint16_t(response[2]) * 256 + response[3];

    ESP_LOGD(TAG, "ZEXX Received GAS=%u ppb, %X,%X,%X,%X,%X,%X,%X,%X,%X, ", gas_ppb,
             response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7],
             response[8]);
    if (this->gas_sensor_ != nullptr) {
      this->gas_sensor_->publish_state(gas_ppb);
    }
  }
}

void ZEXXComponent::loop() {
  uint16_t gas_ppb = 0;
  if (this->mode_ == ZEXX_MODE_ACTIVE) {
    uint8_t peeked;
    while (this->available()) {
      if (!this->head_found && this->peek_byte(&peeked) && peeked != 0xFF) {
        this->read();
      } else {
        this->head_found = true;  // fixme 重复设置head_found
      }
      if (this->head_found) {
        this->buffer.push_back((uint8_t) this->read());
      }
    }
    if (this->buffer.size() < ZEXX_RESPONSE_LENGTH) {
      return;  // not enough data
    }
    if (zexx_checksum(this->buffer.data()) != this->buffer[8]) {
      ESP_LOGW(TAG, "Received data has invalid checksum: %02X", this->buffer[8]);
      this->buffer.clear();
      this->head_found = false;
      return;
    }
    gas_ppb = uint16_t(this->buffer[4]) * 256 + this->buffer[5];
    if (this->gas_sensor_ != nullptr) {
      this->gas_sensor_->publish_state(gas_ppb);
    }
    this->buffer.erase(this->buffer.begin(), this->buffer.begin() + ZEXX_RESPONSE_LENGTH);
  }
}


bool ZEXXComponent::write_command_(const uint8_t *command, uint8_t *response) {
  // Empty RX Buffer
  while (this->available()) {
    this->read();
  }
  this->write_array(command, ZEXX_REQUEST_LENGTH);
  this->write_byte(zexx_checksum(command));
  this->flush();
  if (response == nullptr) {
    return true;
  }

  return this->read_array(response, ZEXX_RESPONSE_LENGTH);
}

void ZEXXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ZEXX:");
  LOG_SENSOR("  ", "GAS ", this->gas_sensor_);
  this->check_uart_settings(9600);
}

}  // namespace ws_z
