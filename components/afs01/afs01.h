#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::afs01 {

class AFS01Component: public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_volume_flow_rate_sensor(sensor::Sensor *volume_flow_rate_sensor) {
    this->volume_flow_rate_sensor_ = volume_flow_rate_sensor;
  }

 protected:
  sensor::Sensor *volume_flow_rate_sensor_{nullptr};
  uint32_t unique_id();
};

}

