#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::acd1100 {

class ACD1100Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_co2_sensor(sensor::Sensor *co2_sensor) { this->co2_sensor_ = co2_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_base_sensor(sensor::Sensor *base_sensor) { this->base_sensor_ = base_sensor; }

  void set_calibrate_mode(bool auto_);
  bool get_calibrate_mode();
  void calibrate(uint16_t data);
  uint16_t read_base();
  void reset();

 protected:
  sensor::Sensor *co2_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *base_sensor_{nullptr};
  void version(char *buffer);
  void sn(char *buffer);
};

template<typename... Ts> class ACD1100SetCalibrateModeAction : public Action<Ts...> {
 public:
  ACD1100SetCalibrateModeAction(ACD1100Component *acd1100) : acd1100_(acd1100) {}
  TEMPLATABLE_VALUE(bool, mode)
  void play(const Ts &...x) override { this->acd1100_->set_calibrate_mode(this->mode_.value(x...)); }

 protected:
  ACD1100Component *acd1100_;
};

template<typename... Ts> class ACD1100CalibrateAction : public Action<Ts...> {
 public:
  ACD1100CalibrateAction(ACD1100Component *acd1100) : acd1100_(acd1100) {}
  TEMPLATABLE_VALUE(uint16_t, base)
  void play(const Ts &...x) override { this->acd1100_->calibrate(this->base_.value(x...)); }

 protected:
  ACD1100Component *acd1100_;
};

template<typename... Ts> class ACD1100ResetAction : public Action<Ts...> {
 public:
  ACD1100ResetAction(ACD1100Component *acd1100) : acd1100_(acd1100) {}
  void play(const Ts &...x) override { this->acd1100_->reset(); }

 protected:
  ACD1100Component *acd1100_;
};

} // namespace esphome::acd1100
