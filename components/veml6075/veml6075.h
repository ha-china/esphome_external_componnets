#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::veml6075 {

#define VEML6075_DEFAULT_UVA_A_COEFF 2.22       ///< Default for no coverglass
#define VEML6075_DEFAULT_UVA_B_COEFF 1.33       ///< Default for no coverglass
#define VEML6075_DEFAULT_UVB_C_COEFF 2.95       ///< Default for no coverglass
#define VEML6075_DEFAULT_UVB_D_COEFF 1.74       ///< Default for no coverglass
#define VEML6075_DEFAULT_UVA_RESPONSE 0.001461  ///< Default for no coverglass
#define VEML6075_DEFAULT_UVB_RESPONSE 0.002591  ///< Default for no coverglass

enum VEML6075IntegrationTime : uint8_t {
  VEML6075_IT_50MS,
  VEML6075_IT_100MS,  // 100 ms
  VEML6075_IT_200MS,  // 200 ms
  VEML6075_IT_400MS,  // 400 ms
  VEML6075_IT_800MS,  // 800 ms
};

enum VEML6075DynamicSetting : uint8_t {
  VEML6075_NORMAL_DYNAMIC,
  VEML6075_HIGH_DYNAMIC,
};

enum VEML6075Trigger : uint8_t {
  VEML6075_TRIGGER_NONE,
  VEML6075_TRIGGER_ONCE,
};

enum VEML6075ActiveForceMode : uint8_t {
  VEML6075_ACTIVE_FORCE_MODE_DISABLE,
  VEML6075_ACTIVE_FORCE_MODE_ENABLE,
};

class VEML6075Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;

  void set_uva_sensor(sensor::Sensor *uva_sensor) { this->uva_sensor_ = uva_sensor; }
  void set_uvb_sensor(sensor::Sensor *uvb_sensor) { this->uvb_sensor_ = uvb_sensor; }
  void set_uvi_sensor(sensor::Sensor *uvi_sensor) { this->uvi_sensor_ = uvi_sensor; }
  void set_coefficients(float UVA_A = VEML6075_DEFAULT_UVA_A_COEFF, float UVA_B = VEML6075_DEFAULT_UVA_B_COEFF,
                        float UVB_C = VEML6075_DEFAULT_UVB_C_COEFF, float UVB_D = VEML6075_DEFAULT_UVB_D_COEFF,
                        float UVA_response = VEML6075_DEFAULT_UVA_RESPONSE,
                        float UVB_response = VEML6075_DEFAULT_UVB_RESPONSE);
  void set_it(VEML6075IntegrationTime time) { this->time_ = time; }
  void set_dynamic_setting(VEML6075DynamicSetting dsetting) { this->dsetting_ = dsetting; }
  void set_active_force_mode(VEML6075ActiveForceMode force_mode) { this->force_mode_ = force_mode; }

 protected:
  sensor::Sensor *uva_sensor_{nullptr};
  sensor::Sensor *uvb_sensor_{nullptr};
  sensor::Sensor *uvi_sensor_{nullptr};
  VEML6075IntegrationTime time_;
  VEML6075DynamicSetting dsetting_;
  VEML6075ActiveForceMode force_mode_;

  void read_config(VEML6075IntegrationTime *time, VEML6075DynamicSetting *dsetting, VEML6075Trigger *trigger,
                   VEML6075ActiveForceMode *force_mode);
  void write_config(VEML6075IntegrationTime time, VEML6075DynamicSetting dsetting, VEML6075Trigger trigger,
                    VEML6075ActiveForceMode force_mode);
  uint16_t read_uva();
  uint16_t read_uvb();
  uint16_t read_comp1();
  uint16_t read_comp2();
  uint16_t read_id();
  i2c::ErrorCode send_command(uint8_t command, uint16_t data);
  uint16_t read_data(uint8_t command);

  float _uva_a;
  float _uva_b;
  float _uvb_c;
  float _uvb_d;
  float _uva_resp;
  float _uvb_resp;
};

template<typename... Ts> class VEML6075SetCoefficientsAction : public Action<Ts...> {
 public:
  VEML6075SetCoefficientsAction(VEML6075Component *veml6075) : veml6075_(veml6075) {}
  TEMPLATABLE_VALUE(float, uva_a)
  TEMPLATABLE_VALUE(float, uva_b)
  TEMPLATABLE_VALUE(float, uvb_c)
  TEMPLATABLE_VALUE(float, uvb_d)
  TEMPLATABLE_VALUE(float, uva_response)
  TEMPLATABLE_VALUE(float, uvb_response)
  void play(const Ts &...x) override { this->veml6075_->set_coefficients(this->uva_a_.value(x...),
                                                                  this->uva_b_.value(x...),
                                                                  this->uvb_c_.value(x...),
                                                                  this->uvb_d_.value(x...),
                                                                  this->uva_response_.value(x...),
                                                                  this->uvb_response_.value(x...)); }

 protected:
  VEML6075Component *veml6075_;
};

} // namespace esphome::veml6075
