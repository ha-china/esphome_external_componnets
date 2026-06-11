#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::gd60914 {

enum GD60914_MODE: uint8_t {
  GD60914_MODE_OBJ = 0xAA,
  GD60914_MDOE_FOREHEAD = 0xAB,
  GD60914_MDOE_WRIST = 0xAC,
};

const static uint8_t SINGLE = 0xA1;  // 单次测量
const static uint8_t CALIBRATE35_CMD[5] = {0xA9, 0xA2, 0x01, 0x0C, 0x05}; // 校准35度命令
const static uint8_t CALIBRATE42_CMD[5] = {0xA9, 0xA2, 0x01, 0x0E, 0x0D}; // 校准42度命令
const static uint8_t RESET_CMD[5] = {0xA9, 0xA2, 0x01, 0x06, 0x02 };

class GD60914Component : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; };
  void update() override;
  void dump_config() override;

  void set_mode(GD60914_MODE mode) {this->mode_ = mode;}
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void reset();
  void calibrate35();
  void calibrate42();
 protected:
  GD60914_MODE mode_;
  sensor::Sensor *temperature_sensor_{nullptr};
};

template<typename... Ts> class GD60914ResetAction : public Action<Ts...> {
 public:
  GD60914ResetAction(GD60914Component *gd60914) : gd60914_(gd60914) {}
  void play(const Ts &...x) override { this->gd60914_->reset(); }

 protected:
  GD60914Component *gd60914_;
};


template<typename... Ts> class GD60914Calibrate35Action : public Action<Ts...> {
 public:
  GD60914Calibrate35Action(GD60914Component *gd60914) : gd60914_(gd60914) {}
  void play(const Ts &...x) override { this->gd60914_->calibrate35(); }

 protected:
  GD60914Component *gd60914_;
};

template<typename... Ts> class GD60914Calibrate42Action : public Action<Ts...> {
 public:
  GD60914Calibrate42Action(GD60914Component *gd60914) : gd60914_(gd60914) {}
  void play(const Ts &...x) override { this->gd60914_->calibrate42(); }

 protected:
  GD60914Component *gd60914_;
};
} // namespace esphome::gd60914



