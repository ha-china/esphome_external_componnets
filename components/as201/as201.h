#pragma once

#include <array>
#include <vector>
#include <queue>
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

namespace esphome::as201 {

class AS201Component : public Component, public uart::UARTDevice {
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)
  SUB_TEXT_SENSOR(type)
  SUB_TEXT_SENSOR(accuracy)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(factory_reset)
  SUB_BUTTON(restart)
  SUB_BUTTON(reset_z_axis)
  SUB_BUTTON(reset_euler)
#endif
#ifdef USE_SELECT
  SUB_SELECT(direction)
  SUB_SELECT(upload_rate)
  SUB_SELECT(baud_rate)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(enable_upload)
#endif


 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;

  void factory_reset();
  void restart();
  void version();
  void reset_z_axis();
  void reset_euler_angle();
  void get_install_params();
  void set_install_params(const std::string &direction);
  void set_upload_rate(const std::string &rate);
  void set_baud_rate(const std::string &baud_rate);
  void get_config();
  void enable_upload(bool enable);
  void get_data_once();
  void calibrate_accel();
  void start_calibrate_magnetic_field();
  void finish_calibrate_magnetic_field();
  void set_subscribe_flag(bool accel, bool gyro, bool angle,
                                          bool field_strength, bool quaternion,
                                          bool temperature, bool pressure, bool height);

  void parse_data();

  void set_accel_x_sensor(sensor::Sensor *accel_x_sensor) { this->accel_x_sensor_ = accel_x_sensor; }
  void set_accel_y_sensor(sensor::Sensor *accel_y_sensor) { this->accel_y_sensor_ = accel_y_sensor; }
  void set_accel_z_sensor(sensor::Sensor *accel_z_sensor) { this->accel_z_sensor_ = accel_z_sensor; }
  void set_gyro_x_sensor(sensor::Sensor *gyro_x_sensor) { this->gyro_x_sensor_ = gyro_x_sensor; }
  void set_gyro_y_sensor(sensor::Sensor *gyro_y_sensor) { this->gyro_y_sensor_ = gyro_y_sensor; }
  void set_gyro_z_sensor(sensor::Sensor *gyro_z_sensor) { this->gyro_z_sensor_ = gyro_z_sensor; }
  void set_angle_x_sensor(sensor::Sensor *angle_x_sensor) { this->angle_x_sensor_ = angle_x_sensor; }
  void set_angle_y_sensor(sensor::Sensor *angle_y_sensor) { this->angle_y_sensor_ = angle_y_sensor; }
  void set_angle_z_sensor(sensor::Sensor *angle_z_sensor) { this->angle_z_sensor_ = angle_z_sensor; }
  void set_field_strength_x_sensor(sensor::Sensor *field_strength_x_sensor) { this->field_strength_x_sensor_ = field_strength_x_sensor; }
  void set_field_strength_y_sensor(sensor::Sensor *field_strength_y_sensor) { this->field_strength_y_sensor_ = field_strength_y_sensor; }
  void set_field_strength_z_sensor(sensor::Sensor *field_strength_z_sensor) { this->field_strength_z_sensor_ = field_strength_z_sensor; }
  void set_q0_sensor(sensor::Sensor *q0_sensor) { this->q0_sensor_ = q0_sensor; }
  void set_q1_sensor(sensor::Sensor *q1_sensor) { this->q1_sensor_ = q1_sensor; }
  void set_q2_sensor(sensor::Sensor *q2_sensor) { this->q2_sensor_ = q2_sensor; }
  void set_q3_sensor(sensor::Sensor *q3_sensor) { this->q3_sensor_ = q3_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { this->pressure_sensor_ = pressure_sensor; }
  void set_height_sensor(sensor::Sensor *height_sensor) { this->height_sensor_ = height_sensor; }
 protected:
  std::vector<uint8_t> receive_buffer;
  bool head_found{false};

  sensor::Sensor * accel_x_sensor_{nullptr};
  sensor::Sensor * accel_y_sensor_{nullptr};
  sensor::Sensor * accel_z_sensor_{nullptr};
  sensor::Sensor * gyro_x_sensor_{nullptr};
  sensor::Sensor * gyro_y_sensor_{nullptr};
  sensor::Sensor * gyro_z_sensor_{nullptr};
  sensor::Sensor * angle_x_sensor_{nullptr};
  sensor::Sensor * angle_y_sensor_{nullptr};
  sensor::Sensor * angle_z_sensor_{nullptr};
  sensor::Sensor * field_strength_x_sensor_{nullptr};
  sensor::Sensor * field_strength_y_sensor_{nullptr};
  sensor::Sensor * field_strength_z_sensor_{nullptr};
  sensor::Sensor * q0_sensor_{nullptr};
  sensor::Sensor * q1_sensor_{nullptr};
  sensor::Sensor * q2_sensor_{nullptr};
  sensor::Sensor * q3_sensor_{nullptr};
  sensor::Sensor * temperature_sensor_{nullptr};
  sensor::Sensor * pressure_sensor_{nullptr};
  sensor::Sensor * height_sensor_{nullptr};

  void send_command(uint8_t command, const uint8_t *data, uint16_t data_size);

};

template<typename... Ts> class GetDataOnceAction : public Action<Ts...> {
 public:
  GetDataOnceAction(AS201Component *as201) : as201_(as201) {}
  void play(const Ts &...x) override { this->as201_->get_data_once(); }

 protected:
  AS201Component *as201_;
};

template<typename... Ts> class CalibrateAction : public Action<Ts...> {
 public:
  CalibrateAction(AS201Component *as201) : as201_(as201) {}
  void play(const Ts &...x) override { this->as201_->calibrate_accel(); }

 protected:
  AS201Component *as201_;
};

template<typename... Ts> class StartCalibrateMagneticFieldAction : public Action<Ts...> {
 public:
  StartCalibrateMagneticFieldAction(AS201Component *as201) : as201_(as201) {}
  void play(const Ts &...x) override { this->as201_->start_calibrate_magnetic_field(); }

 protected:
  AS201Component *as201_;
};

template<typename... Ts> class FinishCalibrateMagneticFieldAction : public Action<Ts...> {
 public:
  FinishCalibrateMagneticFieldAction(AS201Component *as201) : as201_(as201) {}
  void play(const Ts &...x) override { this->as201_->finish_calibrate_magnetic_field(); }

 protected:
  AS201Component *as201_;
};

template<typename... Ts> class SetSubscribeFlagAction : public Action<Ts...> {
 public:
  SetSubscribeFlagAction(AS201Component *as201) : as201_(as201) {}
  TEMPLATABLE_VALUE(bool, accel)
  TEMPLATABLE_VALUE(bool, gyro)
  TEMPLATABLE_VALUE(bool, angle)
  TEMPLATABLE_VALUE(bool, field_strength)
  TEMPLATABLE_VALUE(bool, quaternion)
  TEMPLATABLE_VALUE(bool, temperature)
  TEMPLATABLE_VALUE(bool, pressure)
  TEMPLATABLE_VALUE(bool, height)
  void play(const Ts &...x) override { this->as201_->set_subscribe_flag(this->accel_.value(x...),
                                                                        this->gyro_.value(x...),
                                                                        this->angle_.value(x...),
                                                                        this->field_strength_.value(x...),
                                                                        this->quaternion_.value(x...),
                                                                        this->temperature_.value(x...),
                                                                        this->pressure_.value(x...),
                                                                        this->height_.value(x...)); }

 protected:
  AS201Component *as201_;
};


} // namespace esphome::as201
