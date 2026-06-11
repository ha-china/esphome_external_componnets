#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome::ld2413 {

class LD2413Component : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void loop() override;

  void set_distance_sensor(sensor::Sensor *distance_sensor) { this->distance_sensor_ = distance_sensor; }
  void set_update_interval(uint16_t update_interval) { this->update_interval_ = update_interval; }
  void set_max_distance_attr(uint16_t max_distance) { this->max_distance_ = max_distance; }
  void set_min_distance_attr(uint16_t min_distance) { this->min_distance_ = min_distance; }
  // action
  void update_door_limit();
  void set_max_distance(uint16_t max_distance);
  void set_min_distance(uint16_t min_distance);
  void set_report_interval(uint16_t interval);
  void disable_config();
  void enable_config();
 protected:
  sensor::Sensor *distance_sensor_{nullptr};
  uint16_t update_interval_;
  uint16_t max_distance_;
  uint16_t min_distance_;

  uint32_t get_report_interval();
  std::string version();
  void send_command(uint16_t command, const uint8_t *data, size_t data_size,
                                uint16_t *ret_command,
                                uint16_t *status,
                                std::string *value);
  bool in_config{false};
  bool head_found{false};
  uint8_t send_buffer[5];
  std::vector<uint8_t> receive_buffer;
};

template<typename... Ts> class LD2413UpdateDoorLimitAction : public Action<Ts...> {
 public:
  LD2413UpdateDoorLimitAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  void play(const Ts &...x) override { this->ld2413_->update_door_limit(); }

 protected:
  LD2413Component *ld2413_;
};

template<typename... Ts> class LD2413EnableConfigAction : public Action<Ts...> {
 public:
  LD2413EnableConfigAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  void play(const Ts &...x) override { this->ld2413_->enable_config(); }

 protected:
  LD2413Component *ld2413_;
};

template<typename... Ts> class LD2413DisableConfigAction : public Action<Ts...> {
 public:
  LD2413DisableConfigAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  void play(const Ts &...x) override { this->ld2413_->disable_config(); }

 protected:
  LD2413Component *ld2413_;
};

template<typename... Ts> class LD2413SetMaxDistanceAction : public Action<Ts...> {
 public:
  LD2413SetMaxDistanceAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  TEMPLATABLE_VALUE(uint16_t, max_distance)
  void play(const Ts &...x) override { this->ld2413_->set_max_distance(this->max_distance_.value(x...)); }

 protected:
  LD2413Component *ld2413_;
};


template<typename... Ts> class LD2413SetMinDistanceAction : public Action<Ts...> {
 public:
  LD2413SetMinDistanceAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  TEMPLATABLE_VALUE(uint16_t, min_distance)
  void play(const Ts &...x) override { this->ld2413_->set_min_distance(this->min_distance_.value(x...)); }

 protected:
  LD2413Component *ld2413_;
};

template<typename... Ts> class LD2413SetReportIntervalAction : public Action<Ts...> {
 public:
  LD2413SetReportIntervalAction(LD2413Component *ld2413) : ld2413_(ld2413) {}
  TEMPLATABLE_VALUE(uint16_t, update_interval)
  void play(const Ts &...x) override { this->ld2413_->set_report_interval(this->update_interval_.value(x...)); }

 protected:
  LD2413Component *ld2413_;
};

}
