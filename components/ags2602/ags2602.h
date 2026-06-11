#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::ags2602 {

class AGS2602Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_tvoc_sensor(sensor::Sensor *tvoc_sensor) { this->tvoc_sensor_ = tvoc_sensor; }
  void set_resistor_sensor(sensor::Sensor *resistor_sensor) { this->resistor_sensor_ = resistor_sensor; }
  void calibrate(uint16_t mode);
 protected:
  sensor::Sensor *tvoc_sensor_{nullptr};
  sensor::Sensor *resistor_sensor_{nullptr};
  int get_version();
};

template<typename... Ts> class AGS2602CalibrateAction : public Action<Ts...> {
 public:
  AGS2602CalibrateAction(AGS2602Component *ags2602) : ags2602_(ags2602) {}
  TEMPLATABLE_VALUE(uint16_t, mode)
  void play(const Ts &...x) override { this->ags2602_->calibrate(this->mode_.value(x...)); }

 protected:
  AGS2602Component *ags2602_;
};

} // esphome::ags2602
