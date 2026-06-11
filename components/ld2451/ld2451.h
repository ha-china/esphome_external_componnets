#pragma once

#include <array>
#include <vector>
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

namespace esphome::ld2451 {

#define MAX_TARGETS 20

enum LD2451_DIRECTION : uint8_t {
  LD2451_DIRECTION_AWAY = 0x00,     // Object is moving away from the sensor
  LD2451_DIRECTION_TOWARDS = 0x01,  // Object is moving towards the sensor
  LD2451_DIRECTION_BOTH = 0x02,
};

enum LD2451_BAUD_RATE : uint16_t {
  LD2451_BAUD_RATE_9600 = 0x0001,
  LD2451_BAUD_RATE_19200 = 0x0002,
  LD2451_BAUD_RATE_38400 = 0x0003,
  LD2451_BAUD_RATE_57600 = 0x0004,
  LD2451_BAUD_RATE_115200 = 0x0005,
  LD2451_BAUD_RATE_230400 = 0x0006,
  LD2451_BAUD_RATE_256000 = 0x0007,
  LD2451_BAUD_RATE_460800 = 0x0008,
};

class LD2451Component : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(target_number)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(has_towards_target)
#endif
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;
#ifdef USE_TEXT_SENSOR
  void set_version_text_sensor(text_sensor::TextSensor *version) {
    this->version_text_sensor_ = version;
  }
#endif
  void set_valid_trigs(uint8_t valid_trigs) { this->valid_trigs_ = valid_trigs; }
  void set_signal_threshold(uint8_t signal_threshold) { this->signal_threshold_ = signal_threshold; }
  void set_max_distance(uint8_t max_distance) { this->max_distance_ = max_distance; }
  void set_direction(LD2451_DIRECTION direction) { this->direction_ = direction; }
  void set_min_speed(uint8_t min_speed) { this->min_speed_ = min_speed; }
  void set_no_target_delay(uint8_t d) {this->delay_ = d; }


  void disable_config();
  void enable_config();
  bool get_target_detect_config(uint8_t *max_distance, LD2451_DIRECTION *direction, uint8_t *min_speed, uint8_t *delay);
  void set_target_detect_config(uint8_t max_distance, LD2451_DIRECTION direction, uint8_t min_speed, uint8_t delay);
  bool get_sensitivity(uint8_t *valid_trigs, uint8_t *signal_threshold);
  void set_sensitivity(uint8_t valid_trigs, uint8_t signal_threshold);
  std::string version();
  void set_baud_rate(LD2451_BAUD_RATE baud_rate);
  bool reset();
  bool restart();
#ifdef USE_SENSOR
  void set_target_angle_sensor(uint8_t target, sensor::Sensor *s) {this->target_angle_sensors_[target] = s;}
  void set_target_distance_sensor(uint8_t target, sensor::Sensor *s) { this->target_distance_sensors_[target] = s;}
  void set_target_speed_sensor(uint8_t target, sensor::Sensor *s) { this->target_speed_sensors_[target] = s; }
  void set_target_signal_strength_sensor(uint8_t target, sensor::Sensor *s) { this->target_signal_strength_sensors_[target] = s; }
#endif
#ifdef USE_TEXT_SENSOR
  void set_target_direction_text_sensor(uint8_t target, text_sensor::TextSensor *s) {
    this->target_direction_text_sensors_[target] = s;
  }
#endif
 protected:
  uint8_t valid_trigs_;
  uint8_t signal_threshold_;
  uint8_t max_distance_;
  LD2451_DIRECTION direction_;
  uint8_t min_speed_;
  uint8_t delay_;
#ifdef USE_SENSOR
  std::array<sensor::Sensor *, MAX_TARGETS> target_angle_sensors_{};
  std::array<sensor::Sensor *, MAX_TARGETS> target_distance_sensors_{};
  std::array<sensor::Sensor *, MAX_TARGETS> target_speed_sensors_{};
  std::array<sensor::Sensor *, MAX_TARGETS> target_signal_strength_sensors_{};
#endif

#ifdef USE_TEXT_SENSOR
  text_sensor::TextSensor *version_text_sensor_;
  std::array<text_sensor::TextSensor *, MAX_TARGETS> target_direction_text_sensors_{};
#endif
  bool in_config{false};
  bool head_found{false};
  void send_command(uint16_t command, const uint8_t *data, size_t data_size, uint16_t *ret_command, std::string *value);

  uint8_t send_buffer[5];
  std::vector<uint8_t> receive_buffer;
};

template<typename... Ts> class LD2451EnableConfigAction : public Action<Ts...> {
 public:
  LD2451EnableConfigAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  void play(const Ts &...x) override { this->ld2451_->enable_config(); }

 protected:
  LD2451Component *ld2451_;
};

template<typename... Ts> class LD2451DisableConfigAction : public Action<Ts...> {
 public:
  LD2451DisableConfigAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  void play(const Ts &...x) override { this->ld2451_->disable_config(); }

 protected:
  LD2451Component *ld2451_;
};

template<typename... Ts> class LD2451SetBaudRateAction : public Action<Ts...> {
 public:
  LD2451SetBaudRateAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  TEMPLATABLE_VALUE(uint8_t, baud_rate)
  void play(const Ts &...x) override { this->ld2451_->set_baud_rate((LD2451_BAUD_RATE) this->baud_rate_.value(x...)); }

 protected:
  LD2451Component *ld2451_;
};

template<typename... Ts> class LD2451ResetAction : public Action<Ts...> {
 public:
  LD2451ResetAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  void play(const Ts &...x) override { this->ld2451_->reset(); }

 protected:
  LD2451Component *ld2451_;
};

template<typename... Ts> class LD2451RestartAction : public Action<Ts...> {
 public:
  LD2451RestartAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  void play(const Ts &...x) override { this->ld2451_->restart(); }

 protected:
  LD2451Component *ld2451_;
};


template<typename... Ts> class LD2451SetSensitivityAction : public Action<Ts...> {
 public:
  LD2451SetSensitivityAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  TEMPLATABLE_VALUE(uint8_t, valid_trigs)
  TEMPLATABLE_VALUE(uint8_t, signal_threshold)
  void play(const Ts &...x) override { this->ld2451_->set_sensitivity(this->valid_trigs_.value(x...),
                                                               this->signal_threshold_.value(x...)); }

 protected:
  LD2451Component *ld2451_;
};

template<typename... Ts> class LD2451SetTargetDetectConfigAction : public Action<Ts...> {
 public:
  LD2451SetTargetDetectConfigAction(LD2451Component *ld2451) : ld2451_(ld2451) {}
  TEMPLATABLE_VALUE(uint8_t, max_distance)
  TEMPLATABLE_VALUE(uint8_t, direction)
  TEMPLATABLE_VALUE(uint8_t, min_speed)
  TEMPLATABLE_VALUE(uint8_t, no_target_delay)
  void play(const Ts &...x) override { this->ld2451_->set_target_detect_config(this->max_distance_.value(x...),
                                                                        (LD2451_DIRECTION)this->direction_.value(x...),
                                                                        this->min_speed_.value(x...),
                                                                        this->no_target_delay_.value(x...)); }

 protected:
  LD2451Component *ld2451_;
};


} // namespace esphome::ld2451
