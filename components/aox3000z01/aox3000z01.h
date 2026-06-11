#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::aox3000z01 {

class AOX3000Z01Component: public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void update() override;
  void dump_config() override;
  void set_o2_sensor(sensor::Sensor *o2_sensor) { this->o2_sensor_ = o2_sensor; }

 protected:
  sensor::Sensor *o2_sensor_{nullptr};
  uint8_t buffer[12];
};

}

