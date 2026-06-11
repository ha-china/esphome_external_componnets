#pragma once

#include <vector>
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::as762x {

#define AS726x_DEVICE_TYPE 0x00
#define AS726x_HW_VERSION 0x01
#define AS726x_CONTROL_SETUP 0x04
#define AS726x_INT_T 0x05
#define AS726x_DEVICE_TEMP 0x06
#define AS726x_LED_CONTROL 0x07

// The same register locations are shared between the AS7262 and AS7263, they're just called something different
// AS7262 Registers
#define AS7262_V 0x08
#define AS7262_B 0x0A
#define AS7262_G 0x0C
#define AS7262_Y 0x0E
#define AS7262_O 0x10
#define AS7262_R 0x12
#define AS7262_V_CAL 0x14
#define AS7262_B_CAL 0x18
#define AS7262_G_CAL 0x1C
#define AS7262_Y_CAL 0x20
#define AS7262_O_CAL 0x24
#define AS7262_R_CAL 0x28

// AS7263 Registers
#define AS7263_R 0x08
#define AS7263_S 0x0A
#define AS7263_T 0x0C
#define AS7263_U 0x0E
#define AS7263_V 0x10
#define AS7263_W 0x12
#define AS7263_R_CAL 0x14
#define AS7263_S_CAL 0x18
#define AS7263_T_CAL 0x1C
#define AS7263_U_CAL 0x20
#define AS7263_V_CAL 0x24
#define AS7263_W_CAL 0x28

// AS7261 Registers
#define AS7261_X 0x08      // 16b
#define AS7261_Y 0x0A      // 16b
#define AS7261_Z 0x0C      // 16b
#define AS7261_NIR 0x0E    // 16b
#define AS7261_DARK 0x10   // 16b
#define AS7261_CLEAR 0x12  // 16b
#define AS7261_X_CAL 0x14
#define AS7261_Y_CAL 0x18
#define AS7261_Z_CAL 0x1C
#define AS7261_X1931_CAL 0x20
#define AS7261_Y1931_CAL 0x24
#define AS7261_UPRI_CAL 0x28
#define AS7261_VPRI_CAL 0x2C
#define AS7261_U_CAL 0x30
#define AS7261_V_CAL 0x34
#define AS7261_DUV_CAL 0x38
#define AS7261_LUX_CAL 0x3C
#define AS7261_CCT_CAL 0x3E

#define SENSORTYPE_AS7261 0x3D
#define SENSORTYPE_AS7262 0x3E
#define SENSORTYPE_AS7263 0x3F

#define MAX_RETRIES 3
#define POLLING_DELAY 5
#define AS762X_TIMEOUT 3000

enum AS762X_GAIN : uint8_t {
  AS762X_GAIN_1X,
  AS762X_GAIN_3_7X,
  AS762X_GAIN_16X,
  AS762X_GAIN_64X,
};

// MeasurementMode
enum AS762X_CONVERSION_TYPE : uint8_t {
  AS762X_CONVERSION_TYPE_0,  // Mode 0: X, Y, Z and NIR
  AS762X_CONVERSION_TYPE_1,  // Mode 1: X, Y, D and C
  AS762X_CONVERSION_TYPE_2,  // Mode 2: X, Y, Z, NIR, D and C
  AS762X_CONVERSION_TYPE_3,  // Mode 3: One-Shot operation
};

enum AS762X_LED_DRV_CURRENT : uint8_t {
  AS762X_LED_DRV_CURRENT_12_5MA,  // 12.5 mA
  AS762X_LED_DRV_CURRENT_25MA,    // 25 mA
  AS762X_LED_DRV_CURRENT_50MA,    // 50 mA
  AS762X_LED_DRV_CURRENT_100MA,   // 100 mA
};

enum AS762X_LED_IND_CURRENT : uint8_t {
  AS762X_LED_IND_CURRENT_1MA,  // 1 mA
  AS762X_LED_IND_CURRENT_2MA,  // 2 mA
  AS762X_LED_IND_CURRENT_4MA,  // 4 mA
  AS762X_LED_IND_CURRENT_8MA,  // 8 mA
};

class AS762XComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void update() override;
  void loop() override;

  void set_interrupt(bool interrupt) { this->interrupt_output_ = interrupt; }
  void set_gain(AS762X_GAIN gain) { this->gain_ = gain; }
  void set_conversion_type(AS762X_CONVERSION_TYPE conversion_type) { this->conversion_type_ = conversion_type; }
  void set_integration_time(uint8_t time) { this->integration_time_ = time; }
  void set_led_drv_current(AS762X_LED_DRV_CURRENT current) { this->led_drv_current_ = current; }
  void set_led_drv(bool led_drv) { this->led_drv_ = led_drv; }
  void set_led_ind_current(AS762X_LED_IND_CURRENT current) { this->led_ind_current_ = current; }
  void set_led_ind(bool led_ind) { this->led_ind_ = led_ind; }

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_channel_1_sensor(sensor::Sensor *channel_1_sensor) { this->channel_1_sensor_ = channel_1_sensor; }
  void set_channel_2_sensor(sensor::Sensor *channel_2_sensor) { this->channel_2_sensor_ = channel_2_sensor; }
  void set_channel_3_sensor(sensor::Sensor *channel_3_sensor) { this->channel_3_sensor_ = channel_3_sensor; }
  void set_channel_4_sensor(sensor::Sensor *channel_4_sensor) { this->channel_4_sensor_ = channel_4_sensor; }
  void set_channel_5_sensor(sensor::Sensor *channel_5_sensor) { this->channel_5_sensor_ = channel_5_sensor; }
  void set_channel_6_sensor(sensor::Sensor *channel_6_sensor) { this->channel_6_sensor_ = channel_6_sensor; }
  void set_calibrated_x_sensor(sensor::Sensor *calibrated_x_sensor) { this->calibrated_x_sensor_ = calibrated_x_sensor; }
  void set_calibrated_y_sensor(sensor::Sensor *calibrated_y_sensor) { this->calibrated_y_sensor_ = calibrated_y_sensor; }
  void set_calibrated_z_sensor(sensor::Sensor *calibrated_z_sensor) { this->calibrated_z_sensor_ = calibrated_z_sensor; }
  void set_calibrated_x1931_sensor(sensor::Sensor *calibrated_x1931_sensor) { this->calibrated_x1931_sensor_ = calibrated_x1931_sensor;}
  void set_calibrated_y1931_sensor(sensor::Sensor *calibrated_y1931_sensor) { this->calibrated_y1931_sensor_ = calibrated_y1931_sensor; }
  void set_calibrated_upri_sensor(sensor::Sensor *calibrated_upri_sensor) { this->calibrated_upri_sensor_ = calibrated_upri_sensor;}
  void set_calibrated_vpri_sensor(sensor::Sensor *calibrated_vpri_sensor) { this->calibrated_vpri_sensor_ = calibrated_vpri_sensor; }
  void set_calibrated_u_sensor(sensor::Sensor *calibrated_u_sensor) { this->calibrated_u_sensor_ = calibrated_u_sensor; }
  void set_calibrated_v_sensor(sensor::Sensor *calibrated_v_sensor) { this->calibrated_v_sensor_ = calibrated_v_sensor; }
  void set_calibrated_duv_sensor(sensor::Sensor *calibrated_duv_sensor) { this->calibrated_duv_sensor_ = calibrated_duv_sensor;}
  void set_calibrated_lux_sensor(sensor::Sensor *calibrated_lux_sensor) { this->calibrated_lux_sensor_ = calibrated_lux_sensor;}
  void set_calibrated_cct_sensor(sensor::Sensor *calibrated_cct_sensor) { this->calibrated_cct_sensor_ = calibrated_cct_sensor;}

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  // for action
  void reset();

 protected:
  bool interrupt_output_;
  AS762X_GAIN gain_;
  AS762X_CONVERSION_TYPE conversion_type_;
  uint8_t integration_time_;
  AS762X_LED_DRV_CURRENT led_drv_current_;
  bool led_drv_;
  AS762X_LED_IND_CURRENT led_ind_current_;
  bool led_ind_;

  InternalGPIOPin *interrupt_pin_{nullptr};
  sensor::Sensor *channel_1_sensor_{nullptr};
  sensor::Sensor *channel_2_sensor_{nullptr};
  sensor::Sensor *channel_3_sensor_{nullptr};
  sensor::Sensor *channel_4_sensor_{nullptr};
  sensor::Sensor *channel_5_sensor_{nullptr};
  sensor::Sensor *channel_6_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *calibrated_x_sensor_{nullptr};  // calibrated x value
  sensor::Sensor *calibrated_y_sensor_{nullptr};
  sensor::Sensor *calibrated_z_sensor_{nullptr};
  sensor::Sensor *calibrated_x1931_sensor_{nullptr};
  sensor::Sensor *calibrated_y1931_sensor_{nullptr};
  sensor::Sensor *calibrated_upri_sensor_{nullptr};
  sensor::Sensor *calibrated_vpri_sensor_{nullptr};
  sensor::Sensor *calibrated_u_sensor_{nullptr};
  sensor::Sensor *calibrated_v_sensor_{nullptr};
  sensor::Sensor *calibrated_duv_sensor_{nullptr};
  sensor::Sensor *calibrated_lux_sensor_{nullptr};
  sensor::Sensor *calibrated_cct_sensor_{nullptr};
  // handle interrupt
  friend class DataReadyTrigger;
  bool interrupt_{false};  // is it happend
  static void irq(AS762XComponent *c);
  LazyCallbackManager<void()> on_data_ready_;
  template<typename F> void add_on_data_ready_callback(F&& callback) {
    this->on_data_ready_.add(std::forward<F>(callback));
  }


  void get_status(bool *can_read, bool *can_write);
  uint8_t read_virtual_register(uint8_t addr);
  uint8_t write_virtual_register(uint8_t addr, uint8_t data);
  float get_calibrated_value(uint8_t addr);

  void enable_interrupt(bool enable);
  void set_gain_reg(AS762X_GAIN gain);
  AS762X_GAIN get_gain_reg();
  void set_conversion_type_reg(AS762X_CONVERSION_TYPE conversion_type);
  AS762X_CONVERSION_TYPE get_conversion_type_reg();
  void set_control_reg(bool reset, bool interrupt, AS762X_GAIN gain, AS762X_CONVERSION_TYPE conversion_type);
  bool data_available();
  void clear_data_available();
  void set_integration_time_reg(uint8_t time);
  uint8_t get_integration_time_reg(uint8_t time);
  uint8_t get_temperature();
  void set_led_reg(AS762X_LED_DRV_CURRENT drv_current, bool led_drv, AS762X_LED_IND_CURRENT ind_current, bool led_ind);
  uint16_t get_channel(uint8_t addr);
  uint16_t get_x();
  uint16_t get_y();
  uint16_t get_z();
  uint16_t get_nir();
  uint16_t get_dark();
  uint16_t get_clear();
  //
  float get_calibrated_x();
  float get_calibrated_y();
  float get_calibrated_z();
  float get_calibrated_x1931();
  float get_calibrated_y1931();
  float get_calibrated_upri();
  float get_calibrated_vpri();
  float get_calibrated_u();
  float get_calibrated_v();
  float get_calibrated_duv();
  float get_calibrated_lux();
  float get_calibrated_cct();
};

class DataReadyTrigger : public Trigger<> {
 public:
  explicit DataReadyTrigger(AS762XComponent *parent) {
    parent->add_on_data_ready_callback(std::bind(&DataReadyTrigger::trigger, this));
  }
};

template<typename... Ts> class AS762XResetAction : public Action<Ts...> {
 public:
  AS762XResetAction(AS762XComponent *as762x) : as762x_(as762x) {}
  void play(const Ts &...x) override { this->as762x_->reset(); }

 protected:
  AS762XComponent *as762x_;
};

} // namespace esphome::as762x
