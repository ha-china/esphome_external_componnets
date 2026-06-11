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
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

namespace esphome::ld2460 {

#define MAX_TARGETS 5

class LD2460Component : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(target_number)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(target)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(height)
  SUB_NUMBER(angle)

  SUB_NUMBER(detect_distance)
  SUB_NUMBER(detect_start_angle)
  SUB_NUMBER(detect_end_angle)
#endif
#ifdef USE_SELECT
  SUB_SELECT(mode)
  SUB_SELECT(baud_rate)
  SUB_SELECT(sensitivity)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(factory_reset)
  SUB_BUTTON(restart)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(enable_upload)
#endif
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;

  std::queue<std::function<void()>> task_queue_{};
#ifdef USE_SENSOR
  void set_target_x_sensor(uint8_t n, sensor::Sensor *target_x_sensor) { this->target_x_sensors_[n] = target_x_sensor; }
  void set_target_y_sensor(uint8_t n, sensor::Sensor *target_y_sensor) { this->target_y_sensors_[n] = target_y_sensor; }
#endif
  void set_height_(float height) {this->height_ = height;} // 安装高度
  void set_angle_(float angle) {this->angle_ = angle;} // 安装角度
  void set_mode_(const std::string mode) {this->mode_ = mode;};
  void set_detect_distance_(float distance) {this->detect_distance_ = distance;};
  void set_detect_start_angle_(float angle) {this->detect_start_angle_ = angle;};
  void set_detect_end_angle_(float angle) {this->detect_end_angle_ = angle;};
  void set_sensitivity_(std::string sensitivity) {this->sensitivity_ = sensitivity;};

  void enable_upload(bool enable);
  void set_install_params(float height, float angle);
  void get_install_params();
  void set_mode(const std::string &mode);
  void get_mode();
  void get_version();
  void restart();
  void factory_reset();
  void set_baud_rate(const std::string &baud_rate);
  void set_detect_range(float distance, float start_angle, float end_angle);
  void get_detect_range();
  void set_sensitivity(const std::string &sensitivity);
  void get_sensitivity();

  void restart_and_read_all_info();
  void read_all_info();
  template<typename F> void add_on_data_callback(F &&callback) {
    this->data_callback_.add(std::forward<F>(callback));
  }

  void save_to_flash();
 protected:
  float height_;
  float angle_;
  std::string mode_;
  float detect_distance_;
  float detect_start_angle_;
  float detect_end_angle_;
  std::string sensitivity_;

  LazyCallbackManager<void()> data_callback_;

#ifdef USE_SENSOR
  std::array<sensor::Sensor *, MAX_TARGETS> target_x_sensors_{};
  std::array<sensor::Sensor *, MAX_TARGETS> target_y_sensors_{};
#endif
#ifdef USE_NUMBER
  ESPPreferenceObject pref_height_;
  ESPPreferenceObject pref_angle_;
  ESPPreferenceObject pref_detect_distance_;
  ESPPreferenceObject pref_detect_start_angle_;
  ESPPreferenceObject pref_detect_end_angle_;
#endif
  void send_command(uint8_t command, const uint8_t *data, uint16_t data_size);
  std::vector<uint8_t> receive_buffer;
  bool head_found{false};

  void parse_upload();
  void parse_ack();
  friend class AngleNumber;
  friend class HeightNumber;
  friend class DetectDistanceNumber;
  friend class DetectStartAngleNumber;
  friend class DetectEndAngleNumber;
};


template<typename... Ts> class LD2460EnableUploadAction : public Action<Ts...> {
 public:
  LD2460EnableUploadAction(LD2460Component *ld2460) : ld2460_(ld2460) {}
  TEMPLATABLE_VALUE(bool, upload)
  void play(const Ts &...x) override { this->ld2460_->enable_upload(this->upload_.value(x...)); }

 protected:
  LD2460Component *ld2460_;
};

template<typename... Ts> class LD2460SetInstallParamsAction : public Action<Ts...> {
 public:
  LD2460SetInstallParamsAction(LD2460Component *ld2460) : ld2460_(ld2460) {}
  TEMPLATABLE_VALUE(float, height)
  TEMPLATABLE_VALUE(float, angle)
  void play(const Ts &...x) override { this->ld2460_->set_install_params(this->height_.value(x...), this->angle_.value(x...)); }

 protected:
  LD2460Component *ld2460_;
};

template<typename... Ts> class LD2460RestartAction : public Action<Ts...> {
 public:
  LD2460RestartAction(LD2460Component *ld2460) : ld2460_(ld2460) {}
  void play(const Ts &...x) override { this->ld2460_->restart(); }

 protected:
  LD2460Component *ld2460_;
};

template<typename... Ts> class LD2460FactoryResetAction : public Action<Ts...> {
 public:
  LD2460FactoryResetAction(LD2460Component *ld2460) : ld2460_(ld2460) {}
  void play(const Ts &...x) override { this->ld2460_->factory_reset(); }

 protected:
  LD2460Component *ld2460_;
};

template<typename... Ts> class LD2460SetDetectRangeAction : public Action<Ts...> {
 public:
  LD2460SetDetectRangeAction(LD2460Component *ld2460) : ld2460_(ld2460) {}
  TEMPLATABLE_VALUE(float, detect_distance)
  TEMPLATABLE_VALUE(float, detect_start_angle)
  TEMPLATABLE_VALUE(float, detect_end_angle)
  void play(const Ts &...x) override { this->ld2460_->set_detect_range(this->detect_distance_.value(x...),
                                                                this->detect_start_angle_.value(x...),
                                                                this->detect_end_angle_.value(x...)); }

 protected:
  LD2460Component *ld2460_;
};

class LD2460DataTrigger : public Trigger<> {
public:
  explicit LD2460DataTrigger(LD2460Component *parent) {
    parent->add_on_data_callback(std::bind(&LD2460DataTrigger::trigger, this));
  }
};

} // namespace esphome::ld2460

