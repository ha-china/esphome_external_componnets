#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::kanfurco2 {

// enum DARTABCLogic { DART_ABC_NONE = 0, DART_ABC_ENABLED, DART_ABC_DISABLED };
#define SB1_BUFFER_LEN 6  // Length of serial buffer for header + type + length

class KANFURCO2Component : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override;
  void update() override;
  void dump_config() override;
  void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }
  void set_self_calibrate(bool b) { self_calibrate = b; }
  void set_period(uint8_t p) { period = p; }
  void set_base(uint16_t b) { base = b; }
  void calibrate(uint16_t data);
  optional<std::string> version();
  void toggle_self_calibrate(bool open, uint8_t period, uint16_t base);
  optional<std::string> sn();

 protected:
  bool self_calibrate;
  uint8_t period;
  uint16_t base;
  optional<uint32_t> read_co2();
  bool write_command(uint8_t command, const uint8_t *data, uint8_t data_size, uint8_t *response_buf);
  sensor::Sensor *co2_sensor_{nullptr};
};

template<typename... Ts> class ToggleSelfCalibrateAction : public Action<Ts...> {
 public:
  ToggleSelfCalibrateAction(KANFURCO2Component *kanfurco2) : kanfurco2_(kanfurco2) {}
  TEMPLATABLE_VALUE(bool, open)
  TEMPLATABLE_VALUE(uint8_t, period)
  TEMPLATABLE_VALUE(uint16_t, base) // 产生名字base_，py对应需要
  //  template_ = await cg.templatable(config[CONF_VALUE], args, cg.uint16)
  //    cg.add(var.set_base(template_))
  void play(const Ts &...x) override { this->kanfurco2_->toggle_self_calibrate(this->open_.value(x...),
                                                                        this->period_.value(x...),
                                                                        this->base_.value(x...)); }

 protected:
  KANFURCO2Component *kanfurco2_;
};


template<typename... Ts> class CalibrateAction: public Action<Ts...> {
  public:
    CalibrateAction(KANFURCO2Component *kanfurco2) : kanfurco2_(kanfurco2) {}
    TEMPLATABLE_VALUE(uint16_t, base)
    void play(const Ts &...x) override { this->kanfurco2_->calibrate(this->base_.value(x...)); }

  protected:
    KANFURCO2Component *kanfurco2_;
};

} // namespace esphome::kanfurco2
