#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::apm3001 {

class APM3001Component : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override {return setup_priority::DATA;}
  void update() override;
  void dump_config() override;
  void set_pm1_sensor(sensor::Sensor *pm1_sensor) { this->pm1_sensor_ = pm1_sensor; }
  void set_pm2_5_sensor(sensor::Sensor *pm2_5_sensor) { this->pm2_5_sensor_ = pm2_5_sensor; }
  void set_pm4_sensor(sensor::Sensor *pm4_sensor) { this->pm4_sensor_ = pm4_sensor; }
  void set_pm10_sensor(sensor::Sensor *pm10_sensor) { this->pm10_sensor_ = pm10_sensor; }

 protected:
  sensor::Sensor *pm1_sensor_{nullptr};
  sensor::Sensor *pm2_5_sensor_{nullptr};
  sensor::Sensor *pm4_sensor_{nullptr};
  sensor::Sensor *pm10_sensor_{nullptr};

  void start_measurement();
  void stop_measurement();
};

} // namespace esphome::apm3001
