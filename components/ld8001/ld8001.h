#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::ld8001 {

class LD8001Component : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;

  void set_distance_sensor(sensor::Sensor *distance_sensor) { this->distance_sensor_ = distance_sensor; }
 protected:
  sensor::Sensor *distance_sensor_{nullptr};

  bool head_found{false};
  std::vector<uint8_t> receive_buffer;
};

} // namespace esphome::ld8001
