#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::apm10 {

enum APM10_TYPE: uint8_t {
  APM10_TYPE_10, // apm10 apm2000
  APM10_TYPE_3000,
};

class APM10Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_pm1_sensor(sensor::Sensor *pm1_sensor) { this->pm1_sensor_ = pm1_sensor; }
  void set_pm2_5_sensor(sensor::Sensor *pm2_5_sensor) { this->pm2_5_sensor_ = pm2_5_sensor; }
  void set_pm4_sensor(sensor::Sensor *pm4_sensor) { this->pm4_sensor_ = pm4_sensor; }
  void set_pm10_sensor(sensor::Sensor *pm10_sensor) { this->pm10_sensor_ = pm10_sensor; }
  void set_type(APM10_TYPE type_) { this->type_ = type_; }

  void start_measurement();
  void stop_measurement();
 protected:
  sensor::Sensor *pm1_sensor_{nullptr};
  sensor::Sensor *pm2_5_sensor_{nullptr};
  sensor::Sensor *pm4_sensor_{nullptr};
  sensor::Sensor *pm10_sensor_{nullptr};
  APM10_TYPE type_;

};

}
