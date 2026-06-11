#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::agsxxxx {

class AGSXXXXComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_gas_sensor(sensor::Sensor *gas_sensor) { this->gas_sensor_ = gas_sensor; }
  void set_type(std::string type_) { this->type_ = type_; }
  void set_resistor_sensor(sensor::Sensor *resistor_sensor) { this->resistor_sensor_ = resistor_sensor; }
  void calibrate(uint16_t mode);
 protected:
  sensor::Sensor *gas_sensor_{nullptr};
  sensor::Sensor *resistor_sensor_{nullptr};
  std::string type_;
  int get_version();
};

template<typename... Ts> class AGSXXXXCalibrateAction : public Action<Ts...> {
 public:
  AGSXXXXCalibrateAction(AGSXXXXComponent *agsxxxx) : agsxxxx_(agsxxxx) {}
  TEMPLATABLE_VALUE(uint16_t, mode)
  void play(const Ts &...x) override { this->agsxxxx_->calibrate(this->mode_.value(x...)); }

 protected:
  AGSXXXXComponent *agsxxxx_;
};

}  // namespace esphome::agsxxxx