#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::ash01ib {

enum STATE: uint8_t {
  STATE_WAITING,
  STATE_OK,
  STATE_ERROR,
};

class ASH01IBComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { this->humidity_sensor_ = humidity_sensor; }
  STATE state();
  void start_measurement();
  void stop_measurement();
  void start_calibration();
  void stop_calibration();

 protected:
  sensor::Sensor *humidity_sensor_{nullptr};
  uint16_t sn();
  uint16_t version();
  uint32_t unique_id();
};

template<typename... Ts> class ASH01IBStartMeasurementAction : public Action<Ts...> {
 public:
  ASH01IBStartMeasurementAction(ASH01IBComponent *ash01ib) : ash01ib_(ash01ib) {}
  void play(const Ts &...x) override { this->ash01ib_->start_measurement(); }

 protected:
  ASH01IBComponent *ash01ib_;
};

template<typename... Ts> class ASH01IBStopMeasurementAction : public Action<Ts...> {
 public:
  ASH01IBStopMeasurementAction(ASH01IBComponent *ash01ib) : ash01ib_(ash01ib) {}
  void play(const Ts &...x) override { this->ash01ib_->stop_measurement(); }

 protected:
  ASH01IBComponent *ash01ib_;
};


template<typename... Ts> class ASH01IBStartCalibrationAction : public Action<Ts...> {
 public:
  ASH01IBStartCalibrationAction(ASH01IBComponent *ash01ib) : ash01ib_(ash01ib) {}
  void play(const Ts &...x) override { this->ash01ib_->start_calibration(); }

 protected:
  ASH01IBComponent *ash01ib_;
};

template<typename... Ts> class ASH01IBStopCalibrationAction : public Action<Ts...> {
 public:
  ASH01IBStopCalibrationAction(ASH01IBComponent *ash01ib) : ash01ib_(ash01ib) {}
  void play(const Ts &...x) override { this->ash01ib_->stop_calibration(); }

 protected:
  ASH01IBComponent *ash01ib_;
};

}
