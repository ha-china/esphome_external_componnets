#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::agr12 {

enum AGR_TYPE: uint8_t {
  AGR12,
  APR5852,
};

class AGR12Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;

  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { this->pressure_sensor_ = pressure_sensor; }
  void set_type(AGR_TYPE type_) { this->type_ = type_; }

 protected:
  sensor::Sensor *pressure_sensor_{nullptr};
  AGR_TYPE type_;
};

} // namespace esphome::agr12
