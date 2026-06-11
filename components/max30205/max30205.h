#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome::max30205 {

enum MAX30205_DATA_FORMAT: uint8_t {
  MAX30205_DATA_FORMAT_NORMAL   = 0x00,        /**< normal format */
  MAX30205_DATA_FORMAT_EXTENDED = 0x01,        /**< extended format */
};

enum MAX30205_FAULT_QUEUE : uint8_t {
  MAX30205_FAULT_QUEUE_1 = 0x00,        /**< fault queue 1 */
  MAX30205_FAULT_QUEUE_2 = 0x01,        /**< fault queue 2 */
  MAX30205_FAULT_QUEUE_4 = 0x02,        /**< fault queue 4 */
  MAX30205_FAULT_QUEUE_6 = 0x03,        /**< fault queue 6 */
};

enum MAX30205_PIN_POLARITY: uint8_t {
  MAX30205_PIN_POLARITY_LOW  = 0x00,        /**< polarity low */
  MAX30205_PIN_POLARITY_HIGH = 0x01,        /**< polarity high */
};

enum MAX30205_MODE: uint8_t {
  MAX30205_MODE_COMPARATOR  = 0x00,
  MAX30205_MODE_INTERRUPT = 0x01,
};

class MAX30205Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_timeout(bool timeout) {this->timeout_ = timeout; }
  void set_data_format(MAX30205_DATA_FORMAT format) {this->format_ = format; }
  void set_fault_queue(MAX30205_FAULT_QUEUE fault_queue) { this->fault_queue_ = fault_queue;}
  void set_polarity(MAX30205_PIN_POLARITY pin_polarity) { this->pin_polarity_ = pin_polarity; }
  void set_mode(MAX30205_MODE mode) { this->mode_ = mode; }
  void set_high_threshold(int16_t high_threshold) {
    this->high_threshold_ =  high_threshold;
  }
  void set_low_threshold(int16_t low_threshold) {
    this->low_threshold_ = low_threshold;
  }
  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  float read_temperature();
  // action
  void shutdown();
  void wakeup();
 protected:
  sensor::Sensor *temperature_sensor_{nullptr};
  bool timeout_;
  MAX30205_DATA_FORMAT format_;
  MAX30205_FAULT_QUEUE fault_queue_;
  MAX30205_PIN_POLARITY pin_polarity_;
  MAX30205_MODE mode_;
  int16_t high_threshold_;
  int16_t low_threshold_;

  InternalGPIOPin* interrupt_pin_{nullptr};

  friend class OverTemperatureTrigger;
  CallbackManager<void(float)> on_over_temperature_;

  template<typename F> void add_on_over_temperature_callback(F&& callback) {
    this->on_over_temperature_.add(std::forward<F>(callback));
  }
  static void irq(MAX30205Component *c);
  bool interrupt_{false};
  void write_threshold(uint8_t reg, int16_t data);
  int16_t read_threshold(uint8_t reg);
  void write_config(bool one_shot, bool timeout, MAX30205_DATA_FORMAT format, MAX30205_FAULT_QUEUE fault_queue,
                                       MAX30205_PIN_POLARITY pin_polarity, MAX30205_MODE mode, bool shutdown);
  void read_config(bool* one_shot, bool *timeout, MAX30205_DATA_FORMAT* format, MAX30205_FAULT_QUEUE* fault_queue,
                                      MAX30205_PIN_POLARITY* pin_polarity, MAX30205_MODE *mode, bool *shutdown);
};

class OverTemperatureTrigger : public Trigger<float> {
 public:
  explicit OverTemperatureTrigger(MAX30205Component *parent) {
    parent->add_on_over_temperature_callback(std::bind(&OverTemperatureTrigger::trigger, this, std::placeholders::_1));
  }
};

template<typename... Ts> class MAX30205ShutdownAction : public Action<Ts...> {
 public:
  MAX30205ShutdownAction(MAX30205Component *max30205) : max30205_(max30205) {}
  void play(const Ts &...x) override { this->max30205_->shutdown(); }

 protected:
  MAX30205Component *max30205_;
};

template<typename... Ts> class MAX30205WakeupAction : public Action<Ts...> {
 public:
  MAX30205WakeupAction(MAX30205Component *max30205) : max30205_(max30205) {}
  void play(const Ts &...x) override { this->max30205_->wakeup(); }

 protected:
  MAX30205Component *max30205_;
};

}
