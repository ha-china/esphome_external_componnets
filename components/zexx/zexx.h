#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::zexx {

enum ZEXX_MODE : uint8_t {
  ZEXX_MODE_PASSIVE = 0,  // 问答模式
  ZEXX_MODE_ACTIVE, // 主动上传模式
};

class ZEXXComponent : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

  void set_gas_sensor(sensor::Sensor *sensor) { this->gas_sensor_ = sensor; }
  void set_mode(ZEXX_MODE mode) { this->mode_ = mode;}
 protected:
  sensor::Sensor *gas_sensor_{nullptr};

  bool write_command_(const uint8_t *command, uint8_t *response);
  ZEXX_MODE mode_;
  std::vector<uint8_t> buffer;
  bool head_found{false};
};


}  // namespace esphome::zexx
