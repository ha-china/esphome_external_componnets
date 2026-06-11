#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::aof1000 {

class AOF1000Component : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void update() override;
  void dump_config() override;
  void set_o2_sensor(sensor::Sensor *o2_sensor) { this->o2_sensor_ = o2_sensor; }
  void set_volume_flow_rate_sensor(sensor::Sensor *volume_flow_rate_sensor) {
    this->volume_flow_rate_sensor_ = volume_flow_rate_sensor;
  }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }

 protected:
  sensor::Sensor *o2_sensor_{nullptr};
  sensor::Sensor *volume_flow_rate_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  uint8_t buffer[12];
};

}
