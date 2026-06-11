#pragma once

#include <vector>
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome::max30105 {

enum MAX30105_MODE : uint8_t {
  MAX30105_MODE_HR_ONLY = 0x02,    // 仅红光模式
  MAX30105_MODE_SPO2_HR = 0x03,    // 红光+IR模式
  MAX30105_MODE_MULTI_LED = 0x07,  // 多LED模式
};

enum MAX30105_ADC_RANGE : uint8_t {
  MAX30105_ADC_RANGE_2048 = 0x00,   // 2048
  MAX30105_ADC_RANGE_4096 = 0x01,   // 4096
  MAX30105_ADC_RANGE_8192 = 0x02,   // 8192
  MAX30105_ADC_RANGE_16384 = 0x03,  // 16384
};

enum MAX30105_SAMPLE_AVERAGING : uint8_t {
  MAX30105_SAMPLE_AVERAGING_1 = 0x00,  /**< no averaging */
  MAX30105_SAMPLE_AVERAGING_2 = 0x01,  /**< averaging 2 */
  MAX30105_SAMPLE_AVERAGING_4 = 0x02,  /**< averaging 4 */
  MAX30105_SAMPLE_AVERAGING_8 = 0x03,  /**< averaging 8 */
  MAX30105_SAMPLE_AVERAGING_16 = 0x04, /**< averaging 16 */
  MAX30105_SAMPLE_AVERAGING_32 = 0x05, /**< averaging 32 */
};

enum MAX30105_SAMPLE_RATE : uint8_t {
  MAX30105_SAMPLE_RATE_50 = 0x00,
  MAX30105_SAMPLE_RATE_100 = 0x01,
  MAX30105_SAMPLE_RATE_200 = 0x02,
  MAX30105_SAMPLE_RATE_400 = 0x03,
  MAX30105_SAMPLE_RATE_800 = 0x04,
  MAX30105_SAMPLE_RATE_1000 = 0x05,
  MAX30105_SAMPLE_RATE_1600 = 0x06,
  MAX30105_SAMPLE_RATE_3200 = 0x07,
};

enum MAX30105_RESOLUTION : uint8_t {
  MAX30105_RESOLUTION_15_BIT = 0,  //  15位分辨率
  MAX30105_RESOLUTION_16_BIT = 1,  // 16位分辨率
  MAX30105_RESOLUTION_17_BIT = 2,  // 17位分辨率
  MAX30105_RESOLUTION_18_BIT = 3,  // 18位分辨率
};
// 多LED模式时隙常量
enum MAX30105_SLOT : uint8_t {
  MAX30105_SLOT_NONE = 0x00,
  MAX30105_SLOT_RED = 0x01,
  MAX30105_SLOT_IR = 0x02,
  MAX30105_SLOT_GREEN = 0x03,
  MAX30105_SLOT_RED_PILOT = 0x05,
  MAX30105_SLOT_IR_PILOT = 0x06,
  MAX30105_SLOT_GREEN_PILOT = 0x07,
};

enum MAX30105_INTERRUPT : uint8_t {
  MAX30105_INTERRUPT_NONE = 0,
  MAX30105_INTERRUPT_PWR_RDY = 0x01,    // 电源就绪 (0x00 bit0)
  MAX30105_INTERRUPT_PROXIMITY = 0x10,  // 接近检测 (0x00 bit4)
  MAX30105_INTERRUPT_ALC_OVF = 0x20,    // ALC溢出 (0x00 bit5)
  MAX30105_INTERRUPT_DATA_RDY = 0x40,   // 数据就绪 (0x00 bit6)
  MAX30105_INTERRUPT_FIFO_FULL = 0x80,  // FIFO满 (0x00 bit7)
  MAX30105_INTERRUPT_TEMP_RDY = 0x02,   // 温度就绪 (0x01 bit1)
};

class MAX30105Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;
  // sensors
#ifdef USE_SENSOR
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_led1_sensor(sensor::Sensor *led1_sensor) { this->led1_sensor_ = led1_sensor; }
  void set_led2_sensor(sensor::Sensor *led2_sensor) { this->led2_sensor_ = led2_sensor; }
  void set_led3_sensor(sensor::Sensor *led3_sensor) { this->led3_sensor_ = led3_sensor; }
  void set_led4_sensor(sensor::Sensor *led4_sensor) { this->led4_sensor_ = led4_sensor; }
  void set_fifo_overflow_counter_sensor(sensor::Sensor *fifo_overflow_counter_sensor) {
    this->fifo_overflow_counter_sensor_ = fifo_overflow_counter_sensor;
  }
  void set_wr_ptr_sensor(sensor::Sensor *wr_ptr_sensor) { this->wr_ptr_sensor_ = wr_ptr_sensor; }
  void set_rd_ptr_sensor(sensor::Sensor *rd_ptr_sensor) { this->rd_ptr_sensor_ = rd_ptr_sensor; }
#endif
  // binary sensors
#ifdef USE_BINARY_SENSOR
  void set_power_ready_binary_sensor(binary_sensor::BinarySensor *power_ready_binary_sensor) {
    this->power_ready_binary_sensor_ = power_ready_binary_sensor;
  }
  void set_target_binary_sensor(binary_sensor::BinarySensor *target_binary_sensor) {
    this->target_binary_sensor_ = target_binary_sensor;
  }
  void set_alc_overflow_binary_sensor(binary_sensor::BinarySensor *alc_overflow_binary_sensor) {
    this->alc_overflow_binary_sensor_ = alc_overflow_binary_sensor;
  }
  void set_data_ready_binary_sensor(binary_sensor::BinarySensor *data_ready_binary_sensor) {
    this->data_ready_binary_sensor_ = data_ready_binary_sensor;
  }
  void set_fifo_full_binary_sensor(binary_sensor::BinarySensor *fifo_full_binary_sensor) {
    this->fifo_full_binary_sensor_ = fifo_full_binary_sensor;
  }
  void set_temperature_ready_binary_sensor(binary_sensor::BinarySensor *temperature_ready_binary_sensor) {
    this->temperature_ready_binary_sensor_ = temperature_ready_binary_sensor;
  }
#endif
  void set_mode(MAX30105_MODE mode) { this->mode_ = mode; }
  void set_adc_range(MAX30105_ADC_RANGE adc_range) { this->adc_range_ = adc_range; }
  void set_sample_avg(MAX30105_SAMPLE_AVERAGING sample_avg) { this->sample_avg_ = sample_avg; }
  void set_fifo_rollover(bool fifo_rollover) { this->fifo_rollover_ = fifo_rollover; }
  void set_fifo_threshold(uint8_t fifo_threshold) { this->fifo_threshold_ = fifo_threshold; }

  void set_sample_rate(MAX30105_SAMPLE_RATE sample_rate) { this->sample_rate_ = sample_rate; }
  void set_resolution(MAX30105_RESOLUTION resolution) { this->resolution_ = resolution; }
  void set_current(uint8_t red_current, uint8_t ir_current, uint8_t green_current, uint8_t pilot_current) {
    this->red_current_ = red_current;
    this->ir_current_ = ir_current;
    this->green_current_ = green_current;
    this->pilot_current_ = pilot_current;
  }
  void set_interrupts(bool fifo_almost_full, bool data_ready, bool alc_overflow, bool prox_int, bool temp_ready) {
    this->fifo_almost_full_ = fifo_almost_full;
    this->data_ready_ = data_ready;
    this->alc_overflow_ = alc_overflow;
    this->prox_int_ = prox_int;
    this->temp_ready_ = temp_ready;
  }
  void set_proximity_threshold(uint8_t threshold) { this->proximity_threshold_ = threshold; }
  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  // automation call
  void reset();
  void shutdown();
  void wakeup();
  void set_proximity_threshold_reg(uint8_t threshold);
  void set_mode_reg(MAX30105_MODE mode);
  void set_led_current_reg(uint8_t red_current, uint8_t ir_current, uint8_t green_current, uint8_t pilot_current);
  void enable_interrupts(bool fifo_almost_full, bool data_ready, bool alc_overflow, bool prox_int, bool temp_ready);
  void simulate_interrupt();

 protected:
  MAX30105_MODE mode_;
  MAX30105_ADC_RANGE adc_range_;
  MAX30105_SAMPLE_AVERAGING sample_avg_;
  bool fifo_rollover_;
  uint8_t fifo_threshold_;
  MAX30105_SAMPLE_RATE sample_rate_;
  MAX30105_RESOLUTION resolution_;

  uint8_t red_current_;
  uint8_t ir_current_;
  uint8_t green_current_;
  uint8_t pilot_current_;
  uint8_t active_leds_;

  bool fifo_almost_full_;  // 中断配置
  bool data_ready_;
  bool alc_overflow_;
  bool prox_int_;
  bool temp_ready_;
  uint8_t proximity_threshold_;  // 接近阈值
  bool interrupt_{false};        // 是否发生中断
  bool need_clear_{false};       // 是否需要清除中断传感器
#ifdef USE_SENSOR
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *led1_sensor_{nullptr};
  sensor::Sensor *led2_sensor_{nullptr};
  sensor::Sensor *led3_sensor_{nullptr};
  sensor::Sensor *led4_sensor_{nullptr};
  sensor::Sensor *fifo_overflow_counter_sensor_{nullptr};
  sensor::Sensor *wr_ptr_sensor_{nullptr};
  sensor::Sensor *rd_ptr_sensor_{nullptr};
#endif
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *power_ready_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *target_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *alc_overflow_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *data_ready_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fifo_full_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *temperature_ready_binary_sensor_{nullptr};
#endif
  InternalGPIOPin *interrupt_pin_{nullptr};

  bool verify_part_id();

  static void irq(MAX30105Component *c);
  void set_bit(uint8_t addr, uint8_t bit);
  void clear_bit(uint8_t addr, uint8_t bit);
  void clear_fifo();

  void set_multi_led_slots_reg(std::vector<uint8_t> &slots);
  void read_temperature();
  void read_fifo();

  void read_overflow_counter();
  // trigers
  friend class PowerReadyTrigger;
  friend class FifoAlmostFullTrigger;
  friend class DataReadyTrigger;
  friend class ALCOverflowTrigger;
  friend class ProximityInterruptTrigger;
  friend class TemperatureReadyTrigger;

  CallbackManager<void()> on_power_ready_callback_;
  CallbackManager<void()> on_fifo_almost_full_callback_;
  CallbackManager<void()> on_data_ready_callback_;
  CallbackManager<void()> on_alc_overflow_callback_;
  CallbackManager<void()> on_prox_int_callback_;
  CallbackManager<void(float)> on_temp_ready_callback_;

  template<typename F> void add_on_power_ready_callback(F&& callback) {
    this->on_power_ready_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_fifo_almost_full_callback(F&& callback) {
    this->on_fifo_almost_full_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_data_ready_callback(F&& callback) {
    this->on_data_ready_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_alc_overflow_callback(F&& callback) {
    this->on_alc_overflow_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_prox_int_callback(F&& callback) {
    this->on_prox_int_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_temp_ready_callback(F&& callback) {
    this->on_temp_ready_callback_.add(std::forward<F>(callback));
  }
};  // class MAX30105Component

class PowerReadyTrigger : public Trigger<> {
 public:
  explicit PowerReadyTrigger(MAX30105Component *parent) {
    parent->add_on_power_ready_callback(std::bind(&PowerReadyTrigger::trigger, this));
  }
};

class FifoAlmostFullTrigger : public Trigger<> {
 public:
  explicit FifoAlmostFullTrigger(MAX30105Component *parent) {
    parent->add_on_fifo_almost_full_callback(std::bind(&FifoAlmostFullTrigger::trigger, this));
  }
};

class DataReadyTrigger : public Trigger<> {
 public:
  explicit DataReadyTrigger(MAX30105Component *parent) {
    parent->add_on_data_ready_callback(std::bind(&DataReadyTrigger::trigger, this));
  }
};

class ALCOverflowTrigger : public Trigger<> {
 public:
  explicit ALCOverflowTrigger(MAX30105Component *parent) {
    parent->add_on_alc_overflow_callback(std::bind(&ALCOverflowTrigger::trigger, this));
  }
};

class ProximityInterruptTrigger : public Trigger<> {
 public:
  explicit ProximityInterruptTrigger(MAX30105Component *parent) {
    parent->add_on_prox_int_callback(std::bind(&ProximityInterruptTrigger::trigger, this));
  }
};

class TemperatureReadyTrigger : public Trigger<float> {
 public:
  explicit TemperatureReadyTrigger(MAX30105Component *parent) {
    parent->add_on_temp_ready_callback(std::bind(&TemperatureReadyTrigger::trigger, this, std::placeholders::_1));
  }
};

template<typename... Ts> class MAX30105ResetAction : public Action<Ts...> {
 public:
  MAX30105ResetAction(MAX30105Component *max30105) : max30105_(max30105) {}
  void play(const Ts &...x) override { this->max30105_->reset(); }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105ShutdownAction : public Action<Ts...> {
 public:
  MAX30105ShutdownAction(MAX30105Component *max30105) : max30105_(max30105) {}
  void play(const Ts &...x) override { this->max30105_->shutdown(); }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105WakeupAction : public Action<Ts...> {
 public:
  MAX30105WakeupAction(MAX30105Component *max30105) : max30105_(max30105) {}
  void play(const Ts &...x) override { this->max30105_->wakeup(); }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105SetProximityThresholdAction : public Action<Ts...> {
 public:
  MAX30105SetProximityThresholdAction(MAX30105Component *max30105) : max30105_(max30105) {}
  TEMPLATABLE_VALUE(uint8_t, threshold)
  void play(const Ts &...x) override { this->max30105_->set_proximity_threshold_reg(this->threshold_.value(x...)); }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105SetModeAction : public Action<Ts...> {
 public:
  MAX30105SetModeAction(MAX30105Component *max30105) : max30105_(max30105) {}
  TEMPLATABLE_VALUE(uint8_t, mode)
  void play(const Ts &...x) override { this->max30105_->set_mode_reg((MAX30105_MODE) this->mode_.value(x...)); }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105SetLedCurrentAction : public Action<Ts...> {
 public:
  MAX30105SetLedCurrentAction(MAX30105Component *max30105) : max30105_(max30105) {}
  TEMPLATABLE_VALUE(uint8_t, red_current)
  TEMPLATABLE_VALUE(uint8_t, ir_current)
  TEMPLATABLE_VALUE(uint8_t, green_current)
  TEMPLATABLE_VALUE(uint8_t, pilot_current)
  void play(const Ts &...x) override {
    this->max30105_->set_led_current_reg(this->red_current_.value(x...), this->ir_current_.value(x...),
                                         this->green_current_.value(x...), this->pilot_current_.value(x...));
  }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105EnableInterruptsAction : public Action<Ts...> {
 public:
  MAX30105EnableInterruptsAction(MAX30105Component *max30105) : max30105_(max30105) {}
  TEMPLATABLE_VALUE(bool, fifo_almost_full)
  TEMPLATABLE_VALUE(bool, data_ready)
  TEMPLATABLE_VALUE(bool, alc_overflow)
  TEMPLATABLE_VALUE(bool, prox_int)
  TEMPLATABLE_VALUE(bool, temp_ready)
  void play(const Ts &...x) override {
    this->max30105_->enable_interrupts(this->fifo_almost_full_.value(x...), this->data_ready_.value(x...),
                                       this->alc_overflow_.value(x...), this->prox_int_.value(x...),
                                       this->temp_ready_.value(x...));
  }

 protected:
  MAX30105Component *max30105_;
};

template<typename... Ts> class MAX30105SimulateInterruptAction : public Action<Ts...> {
 public:
  MAX30105SimulateInterruptAction(MAX30105Component *max30105) : max30105_(max30105) {}
  void play(const Ts &...x) override { this->max30105_->simulate_interrupt(); }

 protected:
  MAX30105Component *max30105_;
};

} // namespace esphome::max30105

