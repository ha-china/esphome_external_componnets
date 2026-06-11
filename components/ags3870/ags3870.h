#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::ags3870 {

class AGS3870Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_ch4_sensor(sensor::Sensor *ch4_sensor) { this->ch4_sensor_ = ch4_sensor; }
  void set_resistor_sensor(sensor::Sensor *resistor_sensor) { this->resistor_sensor_ = resistor_sensor; }
  void calibrate(uint16_t mode);
 protected:
  sensor::Sensor *ch4_sensor_{nullptr};
  sensor::Sensor *resistor_sensor_{nullptr};
  int get_version();
};

template<typename... Ts> class AGS3870CalibrateAction : public Action<Ts...> {
 public:
  AGS3870CalibrateAction(AGS3870Component *ags3870) : ags3870_(ags3870) {}
  TEMPLATABLE_VALUE(uint16_t, mode)
  void play(const Ts &...x) override { this->ags3870_->calibrate(this->mode_.value(x...)); }

 protected:
  AGS3870Component *ags3870_;
};

}  // namespace esphome::ags3870
