#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::veml6040 {

enum VEML6040_INTEGRATION_TIME: uint8_t {
  VEML6040_INTEGRATION_TIME_40MS,
  VEML6040_INTEGRATION_TIME_80MS,
  VEML6040_INTEGRATION_TIME_160MS,
  VEML6040_INTEGRATION_TIME_320MS,
  VEML6040_INTEGRATION_TIME_640MS,
  VEML6040_INTEGRATION_TIME_1280MS,
};

enum VEML6040_MODE: uint8_t {
  VEML6040_MODE_AUTO,
  VEML6040_MODE_FORCE,
};

class VEML6040Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void update() override;

  void set_red_sensor(sensor::Sensor *red_sensor) { this->red_sensor_ = red_sensor; }
  void set_green_sensor(sensor::Sensor *green_sensor) { this->green_sensor_ = green_sensor; }
  void set_blue_sensor(sensor::Sensor *blue_sensor) { this->blue_sensor_ = blue_sensor; }
  void set_white_sensor(sensor::Sensor *white_sensor) { this->white_sensor_ = white_sensor; }
  void set_integration_time(VEML6040_INTEGRATION_TIME time) {
    this->integration_time_ = time;
  }
  void set_trig(bool trig) {this->trig_ = trig; }
  void set_mode(VEML6040_MODE mode) { this->mode_ = mode; }
  void wakeup();
  void shutdown();
 protected:
  sensor::Sensor *red_sensor_{nullptr};
  sensor::Sensor *green_sensor_{nullptr};
  sensor::Sensor *blue_sensor_{nullptr};
  sensor::Sensor *white_sensor_{nullptr};
  VEML6040_INTEGRATION_TIME integration_time_;
  bool trig_;
  VEML6040_MODE mode_;

  void write_config(VEML6040_INTEGRATION_TIME time, bool trig, VEML6040_MODE mode, bool shutdown);
  void send_word(uint8_t command, uint16_t data);
  uint16_t receive_word(uint8_t command);
};

template<typename... Ts> class VEML6040ShutdownAction : public Action<Ts...> {
 public:
  VEML6040ShutdownAction(VEML6040Component *veml6040) : veml6040_(veml6040) {}
  void play(const Ts &...x) override { this->veml6040_->shutdown(); }

 protected:
  VEML6040Component *veml6040_;
};

template<typename... Ts> class VEML6040WakeupAction : public Action<Ts...> {
 public:
  VEML6040WakeupAction(VEML6040Component *veml6040) : veml6040_(veml6040) {}
  void play(const Ts &...x) override { this->veml6040_->wakeup(); }

 protected:
  VEML6040Component *veml6040_;
};

}
