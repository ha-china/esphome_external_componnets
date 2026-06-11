#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::acd3100 {

class ACD3100Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_co2_sensor(sensor::Sensor *co2_sensor) { this->co2_sensor_ = co2_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_base_sensor(sensor::Sensor *base_sensor) { this->base_sensor_ = base_sensor; }

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

template<typename... Ts> class ACD3100CalibrateAction : public Action<Ts...> {
 public:
  ACD3100CalibrateAction(ACD3100Component *acd3100) : acd3100_(acd3100) {}
  TEMPLATABLE_VALUE(uint16_t, base)
  void play(const Ts &...x) override { this->acd3100_->calibrate(this->base_.value(x...)); }

 protected:
  ACD3100Component *acd3100_;
};

template<typename... Ts> class ACD3100ResetAction : public Action<Ts...> {
 public:
  ACD3100ResetAction(ACD3100Component *acd3100) : acd3100_(acd3100) {}
  void play(const Ts &...x) override { this->acd3100_->reset(); }

 protected:
  ACD3100Component *acd3100_;
};

} // namespace esphome::acd3100
