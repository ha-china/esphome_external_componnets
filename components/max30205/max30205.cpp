#include "max30205.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::max30205 {

static const char *const TAG = "max30205";
static uint8_t REG_TEMPERATURE = 0x00;  // Temperature register
static uint8_t REG_CONFIG = 0x01;       // Configuration register
static uint8_t REG_Thyst = 0x02;        // Hysteresis register
static uint8_t REG_Tos = 0x03;          // Over-temperature shutdown register

void IRAM_ATTR MAX30205Component::irq(MAX30205Component *c) { c->interrupt_ = true; }

void MAX30205Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  this->write_config(false, this->timeout_, this->format_, this->fault_queue_, this->pin_polarity_, this->mode_,
                     false);  // Set default configuration

  this->write_threshold(REG_Tos, this->high_threshold_);
  this->write_threshold(REG_Thyst, this->low_threshold_);

  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    this->interrupt_pin_->setup();
    this->interrupt_pin_->attach_interrupt(MAX30205Component::irq, this, gpio::INTERRUPT_FALLING_EDGE);
  }
}

void MAX30205Component::write_threshold(uint8_t reg, int16_t data) {
  uint8_t buffer[2] = { (uint8_t)(data >> 8), (uint8_t)(data & 0xFF) };
  this->write_register(reg, buffer, 2);
}

int16_t MAX30205Component::read_threshold(uint8_t reg) {
  uint8_t temp[2];
  this->write_read(&reg, 1, temp, 2);
//  this->write(&reg, 1, false);
//  this->read(temp, 2);
  int16_t raw = ((int16_t) (temp[0] & 0x7F)) << 8 | (int16_t) temp[1];
  return raw;
}

void MAX30205Component::loop() {
  if (this->interrupt_) {
    float temperature = this->read_temperature();
    this->on_over_temperature_.call(temperature);
    this->interrupt_ = false;
  }
}


void MAX30205Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MAX30205:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
}

void MAX30205Component::update() {
  if(this->temperature_sensor_ == nullptr) {
    float temperature = this->read_temperature();
    this->temperature_sensor_->publish_state(temperature);
  }
}

void MAX30205Component::shutdown() {
  //  bool one_shot;
  //  bool timeout;
  //  MAX30205_DATA_FORMAT format;
  //  MAX30205_FAULT_QUEUE fault_queue;
  //  MAX30205_PIN_POLARITY pin_polarity;
  //  MAX30205_MODE mode;
  //  bool shutdown;
  //  this->read_config(&one_shot, &timeout, &format, &fault_queue, &pin_polarity, &mode, &shutdown);  // Read the
  //  configuration
  this->write_config(false, this->timeout_, this->format_, this->fault_queue_, this->pin_polarity_, this->mode_,
                     true);  // Set the shutdown mode
}

void MAX30205Component::wakeup() {
  //  bool one_shot;
  //  bool timeout;
  //  MAX30205_DATA_FORMAT format;
  //  MAX30205_FAULT_QUEUE fault_queue;
  //  MAX30205_PIN_POLARITY pin_polarity;
  //  MAX30205_MODE mode;
  //  bool shutdown;
  //  this->read_config(&one_shot, &timeout, &format, &fault_queue, &pin_polarity, &mode, &shutdown);  // Read the
  //  configuration
  this->write_config(false, this->timeout_, this->format_, this->fault_queue_, this->pin_polarity_, this->mode_,
                     false);  // Set the shutdown mode
}

void MAX30205Component::write_config(bool one_shot, bool timeout, MAX30205_DATA_FORMAT format,
                                     MAX30205_FAULT_QUEUE fault_queue, MAX30205_PIN_POLARITY pin_polarity,
                                     MAX30205_MODE mode, bool shutdown) {
  std::bitset<8> config;
  if (one_shot) {
    config[7] = true;  // One-shot mode
  } else {
    config[7] = false;
  }
  if (timeout) {
    config[6] = true;  // Timeout enabled
  } else {
    config[6] = false;
  }
  switch (format) {
    case MAX30205_DATA_FORMAT_NORMAL:
      config[5] = false;  // Normal format
      break;
    case MAX30205_DATA_FORMAT_EXTENDED:
      config[5] = true;  // Extended format
      break;
  }
  switch (fault_queue) {
    case MAX30205_FAULT_QUEUE_1:
      config[4] = false;
      config[3] = false;  // Fault queue 1
      break;
    case MAX30205_FAULT_QUEUE_2:
      config[4] = false;
      config[3] = true;  // Fault queue 2
      break;
    case MAX30205_FAULT_QUEUE_4:
      config[4] = true;
      config[3] = false;  // Fault queue 4
      break;
    case MAX30205_FAULT_QUEUE_6:
      config[4] = true;
      config[3] = true;  // Fault queue 6
      break;
  }
  switch (pin_polarity) {
    case MAX30205_PIN_POLARITY_LOW:
      config[2] = false;  // Polarity low
      break;
    case MAX30205_PIN_POLARITY_HIGH:
      config[2] = true;  // Polarity high
      break;
  }
  switch (mode) {
    case MAX30205_MODE_COMPARATOR:
      config[1] = false;  // Comparator mode
      break;
    case MAX30205_MODE_INTERRUPT:
      config[1] = true;  // Interrupt mode
      break;
  }
  if (shutdown) {
    config[0] = true;  // Shutdown mode
  } else {
    config[0] = false;  // Normal operation
  }
  this->reg(REG_CONFIG) = (uint8_t) config.to_ulong();  // Write the configuration register
}

void MAX30205Component::read_config(bool *one_shot, bool *timeout, MAX30205_DATA_FORMAT *format,
                                    MAX30205_FAULT_QUEUE *fault_queue, MAX30205_PIN_POLARITY *pin_polarity,
                                    MAX30205_MODE *mode, bool *shutdown) {
  uint8_t buff;
  this->write_read(&REG_CONFIG, 1, &buff, 1);
//  this->write(&REG_CONFIG, 1, false);  // Write the register address
//  this->read(&buff, 1);          // Read the configuration register
  std::bitset<8> config = buff;  // Convert to bitset for easier debugging
  if (config[7]) {
    *one_shot = true;
  } else {
    *one_shot = false;
  }
  if (config[6]) {
    *timeout = true;
  } else {
    *timeout = false;
  }
  if (config[5]) {
    *format = MAX30205_DATA_FORMAT_EXTENDED;
  } else {
    *format = MAX30205_DATA_FORMAT_NORMAL;
  }
  if (config[4] && config[3]) {
    *fault_queue = MAX30205_FAULT_QUEUE_6;
  } else if (config[4] && !config[3]) {
    *fault_queue = MAX30205_FAULT_QUEUE_4;
  } else if (!config[4] && config[3]) {
    *fault_queue = MAX30205_FAULT_QUEUE_2;
  } else {
    *fault_queue = MAX30205_FAULT_QUEUE_1;
  }
  if (config[2]) {
    *pin_polarity = MAX30205_PIN_POLARITY_HIGH;
  } else {
    *pin_polarity = MAX30205_PIN_POLARITY_LOW;
  }
  if (config[1]) {
    *mode = MAX30205_MODE_INTERRUPT;
  } else {
    *mode = MAX30205_MODE_COMPARATOR;
  }
  if (config[0]) {
    *shutdown = true;
  } else {
    *shutdown = false;
  }
}

float MAX30205Component::read_temperature() {
//  bool one_shot;
//  bool timeout;
//  MAX30205_DATA_FORMAT format;
//  MAX30205_FAULT_QUEUE fault_queue;
//  MAX30205_PIN_POLARITY pin_polarity;
//  MAX30205_MODE mode;
//  bool shutdown;
//  this->read_config(&one_shot, &timeout, &format, &fault_queue, &pin_polarity, &mode,
//                    &shutdown);                                                          // Read the configuration
  this->write_config(true, this->timeout_, this->format_, this->fault_queue_, this->pin_polarity_, this->mode_, false);  // Set the one-shot mode
  delay(50);  // Wait for the conversion to complete (50 ms is typical for MAX30205)
  uint8_t temp[2];
  this->write_read(&REG_TEMPERATURE, 1, temp, 2);
//  this->write(&REG_TEMPERATURE, 1, false);  // Write the register address
//  this->read(temp, 2);                                                  // Read the configuration register
  int16_t raw = ((int16_t) (temp[0] & 0x7F)) << 8 | (int16_t) temp[1];  // Combine the two bytes into a single value
  float temperature = ((float) raw) / (1 << 8);
  if (temp[0] & 0x80) {          // Check if the sign bit is set
    temperature = -temperature;  // If so, negate the value
  }
  if (this->format_ == MAX30205_DATA_FORMAT_EXTENDED) {
    temperature += 64.0f;
  }
  return temperature;
}

} // namespace esphome::max30205
