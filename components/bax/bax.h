#pragma once

#include <vector>
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

namespace esphome::bax {

enum class BAX_TYPE : uint8_t {
  // BA01, // same as BA012
  BA012, // same as BA112
  BA022,
  BA111,
  BA121,
  BA234,
  BA311,
  BAT3U, // aka BA113
  // BAT2U // also same as BA012
};

class BAXComponent : public PollingComponent, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(temperature)
  SUB_SENSOR(temperature2)
  SUB_SENSOR(temperature3)
  SUB_SENSOR(tds)
  SUB_SENSOR(tds2)
  SUB_SENSOR(tds3)
  SUB_SENSOR(ec)
  SUB_SENSOR(ec2)
  SUB_SENSOR(salinity)
  SUB_SENSOR(specific_gravity)
  SUB_SENSOR(hardness)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(zero_point_calibrate)
#endif
  public:
    void setup() override;
    float get_setup_priority() const override { return setup_priority::DATA; }
    void dump_config() override;
    void loop() override;
    void update() override;

    const char *get_name();

    void set_type_( BAX_TYPE type ) { this->type_ = type; }
    void start_measurement();
    void zero_point_calibrate();
    void twice_calibrate(float sal);
    void set_ntc_resistance(uint32_t resistance);
    void set_ntc_b_value(uint16_t b_value);

  protected:
    BAX_TYPE type_;
    std::vector<uint8_t> buffer_;
};

template<typename... Ts> class ZeroPointCalibrateAction : public Action<Ts...> {
  public:
    ZeroPointCalibrateAction(BAXComponent *bax) : bax_(bax) {}
    void play(const Ts &...x) override { this->bax_->zero_point_calibrate(); }

  protected:
    BAXComponent *bax_;
};


template<typename... Ts> class TwiceCalibrateAction : public Action<Ts...> {
  public:
    TwiceCalibrateAction(BAXComponent *bax) : bax_(bax) {}
    TEMPLATABLE_VALUE(float, salinity)
    void play(const Ts &...x) override { this->bax_->twice_calibrate(this->salinity_.value(x...)); }

  protected:
    BAXComponent *bax_;
};

template<typename... Ts> class SetNTCResistanceAction : public Action<Ts...> {
  public:
    SetNTCResistanceAction(BAXComponent *bax) : bax_(bax) {}
    TEMPLATABLE_VALUE(uint32_t, resistance)
    void play(const Ts &...x) override { this->bax_->set_ntc_resistance(this->resistance_.value(x...)); }

  protected:
    BAXComponent *bax_;
};

template<typename... Ts> class SetNTCBValueAction : public Action<Ts...> {
  public:
    SetNTCBValueAction(BAXComponent *bax) : bax_(bax) {}
    TEMPLATABLE_VALUE(uint16_t, b_value)
    void play(const Ts &...x) override { this->bax_->set_ntc_b_value(this->b_value_.value(x...)); }

  protected:
    BAXComponent *bax_;
};


}

