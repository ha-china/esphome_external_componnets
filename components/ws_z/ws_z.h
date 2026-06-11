#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::ws_z {

//enum DARTABCLogic { DART_ABC_NONE = 0, DART_ABC_ENABLED, DART_ABC_DISABLED };
#define SB1_BUFFER_LEN   6     // Length of serial buffer for header + type + length

enum DARTWS_MODE : uint8_t {
  DARTWS_MODE_PASSIVE = 0,  // 问答模式
  DARTWS_MODE_ACTIVE, // 主动上传模式
};

class DARTWSZComponent : public PollingComponent, public uart::UARTDevice {
 public:
  DARTWSZComponent() = default;
  void setup() override;
  float get_setup_priority() const override;
  void update() override;
  void dump_config() override;
  void loop() override;
  void set_formaldehyde_sensor(sensor::Sensor *formaldehyde_sensor)  { formaldehyde_sensor_ = formaldehyde_sensor; }
  void set_formaldehyde_ppb_sensor(sensor::Sensor *formaldehyde_ppb_sensor) { formaldehyde_ppb_sensor_ = formaldehyde_ppb_sensor; }
  void set_mode(DARTWS_MODE mode) { this->mode_ = mode;}
 
 protected:
  bool dart_write_command_(const uint8_t *command, uint8_t *response);
  sensor::Sensor *formaldehyde_sensor_{nullptr};
  sensor::Sensor *formaldehyde_ppb_sensor_{nullptr};
  DARTWS_MODE mode_;
  std::vector<uint8_t> buffer;
  bool head_found{false};
};


}  // namespace dart
