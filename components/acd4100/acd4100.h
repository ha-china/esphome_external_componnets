#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::acd4100 {

class ACD4100Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_r32_sensor(sensor::Sensor *r32_sensor) { this->r32_sensor_ = r32_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_base_sensor(sensor::Sensor *base_sensor) { this->base_sensor_ = base_sensor; }

  void set_calibrate_mode(bool auto_);
  bool get_calibrate_mode();
  void calibrate(uint16_t data);
  uint16_t read_base();
  void reset();

 protected:
  sensor::Sensor *r32_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *base_sensor_{nullptr};
  void version(char *buffer);
  void sn(char *buffer);
};

template<typename... Ts> class ACD4100SetCalibrateModeAction : public Action<Ts...> {
 public:
  ACD4100SetCalibrateModeAction(ACD4100Component *acd4100) : acd4100_(acd4100) {}
  TEMPLATABLE_VALUE(bool, mode)
  void play(const Ts &...x) override { this->acd4100_->set_calibrate_mode(this->mode_.value(x...)); }

 protected:
  ACD4100Component *acd4100_;
};


template<typename... Ts> class ACD4100CalibrateAction : public Action<Ts...> {
 public:
  ACD4100CalibrateAction(ACD4100Component *acd4100) : acd4100_(acd4100) {}
  TEMPLATABLE_VALUE(uint16_t, base)
  void play(const Ts &...x) override { this->acd4100_->calibrate(this->base_.value(x...)); }

 protected:
  ACD4100Component *acd4100_;
};

template<typename... Ts> class ACD4100ResetAction : public Action<Ts...> {
 public:
  ACD4100ResetAction(ACD4100Component *acd4100) : acd4100_(acd4100) {}
  void play(const Ts &...x) override { this->acd4100_->reset(); }

 protected:
  ACD4100Component *acd4100_;
};

} // namespace esphome::acd4100
