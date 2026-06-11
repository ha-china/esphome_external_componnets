#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::cps610 {

class CPS610Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_a(float a) { this->a_ = a; }
  void set_b(float b) { this->b_ = b; }
  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { this->pressure_sensor_ = pressure_sensor; }
 protected:
  float a_{1.0};
  float b_{0.0};
  sensor::Sensor *pressure_sensor_{nullptr};
};



}

