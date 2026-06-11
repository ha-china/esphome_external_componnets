#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::ags2616 {

class AGS2616Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_h2_sensor(sensor::Sensor *h2_sensor) { this->h2_sensor_ = h2_sensor; }
  void set_resistor_sensor(sensor::Sensor *resistor_sensor) { this->resistor_sensor_ = resistor_sensor; }
  void calibrate(uint16_t mode);
 protected:
  sensor::Sensor *h2_sensor_{nullptr};
  sensor::Sensor *resistor_sensor_{nullptr};
  int get_version();
};

template<typename... Ts> class AGS2616CalibrateAction : public Action<Ts...> {
 public:
  AGS2616CalibrateAction(AGS2616Component *ags2616) : ags2616_(ags2616) {}
  TEMPLATABLE_VALUE(uint16_t, mode)
  void play(const Ts &...x) override { this->ags2616_->calibrate(this->mode_.value(x...)); }

 protected:
  AGS2616Component *ags2616_;
};

}  // namespace esphome::ags2616
