#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::ags3871 {

class AGS3871Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_co_sensor(sensor::Sensor *co_sensor) { this->co_sensor_ = co_sensor; }
  void set_resistor_sensor(sensor::Sensor *resistor_sensor) { this->resistor_sensor_ = resistor_sensor; }
  void calibrate(uint16_t mode);
 protected:
  sensor::Sensor *co_sensor_{nullptr};
  sensor::Sensor *resistor_sensor_{nullptr};
  int get_version();
};

template<typename... Ts> class AGS3871CalibrateAction : public Action<Ts...> {
 public:
  AGS3871CalibrateAction(AGS3871Component *ags3871) : ags3871_(ags3871) {}
  TEMPLATABLE_VALUE(uint16_t, mode)
  void play(const Ts &...x) override { this->ags3871_->calibrate(this->mode_.value(x...)); }

 protected:
  AGS3871Component *ags3871_;
};

}  // namespace esphome::ags3871
