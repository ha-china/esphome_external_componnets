#include "kanfurco2.h"
#include "esphome/core/log.h"

namespace esphome::kanfurco2 {

static const char *const TAG = "kanfurco2";
static const uint8_t KANFURCO2_HEAD = 0x11;
static const uint8_t KANFURCO2_COMMAND_READ = 0x01;
static const uint8_t KANFURCO2_COMMAND_CALIBRATE = 0x03;
static const uint8_t KANFURCO2_COMMAND_VERSION = 0x1E;
static const uint8_t KANFURCO2_COMMAND_TOGGLE_SELF_CALIBRATE = 0x10;
static const uint8_t KANFURCO2_COMMAND_SN = 0x1F;

// uint8_t checksum(const uint8_t *command, uint8_t size) {
//   uint8_t sum = 0;
//   for (uint8_t i = 0; i < size - 1; i++) {
//     sum += command[i];
//   }
//   return 0xFF - sum + 0x01;
// }

void KANFURCO2Component::setup() {
  ESP_LOGW(TAG, "setup KANFUR CO2 sensor");
  auto version = this->version();
  if (version.has_value()) {
    ESP_LOGW(TAG, "version: %s", version.value().c_str());
  }
  auto sn = this->sn();
  if (sn.has_value()) {
    ESP_LOGW(TAG, "sn: %s", sn.value().c_str());
  }
  this->toggle_self_calibrate(this->self_calibrate, this->period, this->base);
}

float KANFURCO2Component::get_setup_priority() const { return setup_priority::DATA; }

void KANFURCO2Component::update() {
  auto val = this->read_co2();
  if (!val.has_value()) {
    return;
  }
  uint32_t co2 = val.value();
  if (this->co2_sensor_ != nullptr) {
    this->co2_sensor_->publish_state(co2);
  }
}

optional<uint32_t> KANFURCO2Component::read_co2() {
  uint8_t buf[8];
  bool read_success = this->write_command(KANFURCO2_COMMAND_READ, nullptr, 0, buf);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from KANFUR CO2 failed!");
    this->status_set_warning();
    return nullopt;
  }
  if (buf[0] != 0x16) {
    ESP_LOGW(TAG, "KANFUR CO2 resp header error!");
    this->status_set_warning();
    return nullopt;
  }
  return (uint32_t) (buf[3]) * 256 + (uint32_t) buf[4];
}

void KANFURCO2Component::calibrate(uint16_t c) {
  uint8_t buf[4];
  uint8_t data[2];
  data[0] = c >> 8;
  data[1] = c & 0xFF;
  bool read_success = this->write_command(KANFURCO2_COMMAND_CALIBRATE, data, 2, buf);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from KANFUR CO2 failed!");
    this->status_set_warning();
    return;
  }
  if (buf[0] != 0x16 || buf[1] != 0x01 || buf[2] != 0x03 || buf[3] != 0xE6) {
    ESP_LOGW(TAG, "KANFUR CO2 resp header error!");
    this->status_set_warning();
    return;
  }
}

optional<std::string> KANFURCO2Component::version() {
  uint8_t buf[15];
  bool read_success = this->write_command(KANFURCO2_COMMAND_VERSION, nullptr, 0, buf);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from KANFUR CO2 failed!");
    this->status_set_warning();
    return nullopt;
  }
  if (buf[0] != 0x16 || buf[1] != 0x0C || buf[2] != 0x1E) {
    ESP_LOGW(TAG, "KANFUR CO2 resp header error!");
    this->status_set_warning();
    return nullopt;
  }
  std::string str(reinterpret_cast<const char *>(buf + 3), 10);
  return str;
}

void KANFURCO2Component::toggle_self_calibrate(bool open, uint8_t period, uint16_t base) {
  uint8_t buf[4];
  uint8_t data[6] = {100, 0, 7, 0, 0, 100};
  if (!open) {
    data[1] = 2;
  }
  data[2] = period;
  data[3] = base >> 8;
  data[4] = base & 0xFF;
  bool read_success = this->write_command(KANFURCO2_COMMAND_TOGGLE_SELF_CALIBRATE, data, 6, buf);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from KANFUR CO2 failed!");
    this->status_set_warning();
    return;
  }
  if (buf[0] != 0x16 || buf[1] != 0x01 || buf[2] != 0x10 || buf[3] != 0xD9) {
    ESP_LOGW(TAG, "KANFUR CO2 resp header error!");
    this->status_set_warning();
    return;
  }
}

optional<std::string> KANFURCO2Component::sn() {
  uint8_t buf[9];
  bool read_success = this->write_command(KANFURCO2_COMMAND_SN, nullptr, 0, buf);
  if (!read_success) {
    ESP_LOGW(TAG, "Reading data from KANFUR CO2 failed!");
    this->status_set_warning();
    return nullopt;
  }
  if (buf[0] != 0x16) {
    ESP_LOGW(TAG, "KANFUR CO2 resp header error!");
    this->status_set_warning();
    return nullopt;
  }
  std::string str(reinterpret_cast<const char *>(buf + 3), 5);
  return str;
}

bool KANFURCO2Component::write_command(uint8_t command, const uint8_t *data, uint8_t data_size, uint8_t *response_buf) {
  while (this->available()) {
    this->read();
  }  // clear rx buf
  uint8_t checksum = KANFURCO2_HEAD;
  this->write(KANFURCO2_HEAD);
  this->write(data_size + 1);
  checksum += (data_size + 1);
  this->write(command);
  checksum += command;
  if (data != nullptr) {
    this->write_array(data, data_size);
    for (uint8_t i = 0; i < data_size; i++) {
      checksum += data[i];
    }
  }
  checksum = 0xFF - checksum + 0x01;
  this->write(checksum);
  this->flush();
  if (response_buf == nullptr) {
    return true;
  }
  switch (command) {
    case KANFURCO2_COMMAND_READ: {
      return this->read_array(response_buf, 8);
    }
    case KANFURCO2_COMMAND_CALIBRATE: {
      return this->read_array(response_buf, 4);
    }
    case KANFURCO2_COMMAND_VERSION: {
      return this->read_array(response_buf, 15);
    }
    case KANFURCO2_COMMAND_TOGGLE_SELF_CALIBRATE: {
      return this->read_array(response_buf, 4);
    }
    case KANFURCO2_COMMAND_SN: {
      return this->read_array(response_buf, 9);
    }
    default: {
      return false;
    }
  }
}

void KANFURCO2Component::dump_config() {
  ESP_LOGCONFIG(TAG, "KANFUR CO2:");
  LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  this->check_uart_settings(9600);
}
} // namespace esphome::kanfurco2

