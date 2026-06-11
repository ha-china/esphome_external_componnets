#include "max30105.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::max30105 {

static const char *const TAG = "max30105";
// 寄存器地址常量 (根据数据手册定义)
static const uint8_t REG_INTR_STATUS_1 = 0x00;
static const uint8_t REG_INTR_STATUS_2 = 0x01;
static const uint8_t REG_INTR_ENABLE_1 = 0x02;
static const uint8_t REG_INTR_ENABLE_2 = 0x03;
static const uint8_t REG_FIFO_WR_PTR = 0x04;
static const uint8_t REG_OVF_COUNTER = 0x05;
static const uint8_t REG_FIFO_RD_PTR = 0x06;
static const uint8_t REG_FIFO_DATA = 0x07;
static const uint8_t REG_FIFO_CONFIG = 0x08;
static const uint8_t REG_MODE_CONFIG = 0x09;
static const uint8_t REG_SPO2_CONFIG = 0x0A;
static const uint8_t REG_LED1_PA = 0x0C;
static const uint8_t REG_LED2_PA = 0x0D;
static const uint8_t REG_LED3_PA = 0x0E;
static const uint8_t REG_PILOT_PA = 0x10;
static const uint8_t REG_MULTI_LED_CTRL1 = 0x11;
static const uint8_t REG_MULTI_LED_CTRL2 = 0x12;
static const uint8_t REG_TEMP_INT = 0x1F;
static const uint8_t REG_TEMP_FRAC = 0x20;
static const uint8_t REG_TEMP_CONFIG = 0x21;
static const uint8_t REG_PROX_INT_THRESH = 0x30;
static const uint8_t REG_REV_ID = 0xFE;
static const uint8_t REG_PART_ID = 0xFF;

static const uint8_t EXPECTED_PART_ID = 0x15;

void IRAM_ATTR MAX30105Component::irq(MAX30105Component *c) { c->interrupt_ = true; }

void MAX30105Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  if (!this->verify_part_id()) {
    this->mark_failed();
    return;
  }
  this->reset();
  this->clear_fifo();
  this->reg(REG_FIFO_CONFIG) =
      ((this->sample_avg_ << 5) | (static_cast<uint8_t>(this->fifo_rollover_) << 4) | (this->fifo_threshold_));

  this->set_mode_reg(this->mode_);

  uint8_t spo2_config = (this->adc_range_ << 5) | (this->sample_rate_ << 2) | this->resolution_;
  this->reg(REG_SPO2_CONFIG) = spo2_config;

  this->set_led_current_reg(this->red_current_, this->ir_current_, this->green_current_, this->pilot_current_);

  std::vector<uint8_t> data;
  if (this->mode_ == MAX30105_MODE_SPO2_HR) {
    data.push_back(MAX30105_SLOT_RED);
    data.push_back(MAX30105_SLOT_IR);
  } else if (this->mode_ == MAX30105_MODE_HR_ONLY) {
    data.push_back(MAX30105_SLOT_RED);
  } else if (this->mode_ == MAX30105_MODE_MULTI_LED) {
    data.push_back(MAX30105_SLOT_RED);
    data.push_back(MAX30105_SLOT_IR);
    data.push_back(MAX30105_SLOT_GREEN);
  }  // todo MAX30105_SLOT_RED_PILOT?
  this->set_multi_led_slots_reg(data);

  this->enable_interrupts(this->fifo_almost_full_, this->data_ready_, this->alc_overflow_, this->prox_int_,
                          this->temp_ready_);

  this->set_proximity_threshold_reg(this->proximity_threshold_);

  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    this->interrupt_pin_->setup();
    this->interrupt_pin_->attach_interrupt(MAX30105Component::irq, this, gpio::INTERRUPT_FALLING_EDGE);
  }
  this->simulate_interrupt(); // 貌似不读一次寄存器的话，后续中断不会发生
}

void MAX30105Component::update() {
#ifdef USE_SENSOR
  this->read_temperature();
  this->read_fifo();
  this->read_overflow_counter();
#endif
}

void MAX30105Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MAX30105:\n"
                     "  REV ID: %d\n", this->reg(REG_REV_ID).get());
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Mode: %d", this->mode_);
  ESP_LOGCONFIG(TAG, "  ADC Range: %d", this->adc_range_);
  ESP_LOGCONFIG(TAG, "  Sample Averaging: %d", this->sample_avg_);
  ESP_LOGCONFIG(TAG, "  FIFO Rollover: %s", this->fifo_rollover_ ? "Enabled" : "Disabled");
  ESP_LOGCONFIG(TAG, "  FIFO Threshold: %d", this->fifo_threshold_);
  ESP_LOGCONFIG(TAG, "  Sample Rate: %d", this->sample_rate_);
  ESP_LOGCONFIG(TAG, "  Resolution: %d", this->resolution_);
  ESP_LOGCONFIG(TAG, "  Red Current amp: %d", this->red_current_);
  ESP_LOGCONFIG(TAG, "  IR Current amp: %d", this->ir_current_);
  ESP_LOGCONFIG(TAG, "  Green Current amp: %d", this->green_current_);
  ESP_LOGCONFIG(TAG, "  Pilot Current amp: %d", this->pilot_current_);
#ifdef USE_SENSOR
  LOG_SENSOR("    ", "Temperature Sensor", this->temperature_sensor_);
  LOG_SENSOR("    ", "LED1 Sensor", this->led1_sensor_);
  LOG_SENSOR("    ", "LED2 Sensor", this->led2_sensor_);
  LOG_SENSOR("    ", "LED3 Sensor", this->led3_sensor_);
  LOG_SENSOR("    ", "LED4 Sensor", this->led4_sensor_);
  LOG_SENSOR("    ", "FIFO Overflow Counter Sensor", this->fifo_overflow_counter_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("    ", "Power Ready Binary Sensor", this->power_ready_binary_sensor_);
  LOG_BINARY_SENSOR("    ", "Target Binary Sensor", this->target_binary_sensor_);
  LOG_BINARY_SENSOR("    ", "ALC Overflow Binary Sensor", this->alc_overflow_binary_sensor_);
  LOG_BINARY_SENSOR("    ", "Data Ready Binary Sensor", this->data_ready_binary_sensor_);
  LOG_BINARY_SENSOR("    ", "FIFO Full Binary Sensor", this->fifo_full_binary_sensor_);
  LOG_BINARY_SENSOR("    ", "Temperature Ready Binary Sensor", this->temperature_ready_binary_sensor_);
#endif
}

void MAX30105Component::read_fifo() {
  uint8_t wr_ptr = this->reg(REG_FIFO_WR_PTR).get();  // 获取FIFO写指针
  uint8_t rd_ptr = this->reg(REG_FIFO_RD_PTR).get();  // 获取FIFO读指针
  if (this->wr_ptr_sensor_ != nullptr) {
    this->wr_ptr_sensor_->publish_state(wr_ptr);  // 发布写指针状态
  }
  if (this->rd_ptr_sensor_ != nullptr) {
    this->rd_ptr_sensor_->publish_state(rd_ptr);  // 发布读指针状态
  }
//  uint8_t num_samples = 32 + wr_ptr - rd_ptr if wr_ptr < rd_ptr else wr_ptr - rd_ptr;
  uint8_t num_samples = 0;
  if (wr_ptr < rd_ptr) {
    num_samples = 32 + wr_ptr - rd_ptr;
  } else {
    num_samples = wr_ptr - rd_ptr;
  }
  num_samples %= 32;
  if (num_samples == 0) {
    if (this->reg(REG_OVF_COUNTER).get() > 0) {
      wr_ptr += 1;
      if (wr_ptr > 31) {
        wr_ptr = 0;  // 如果写指针超过31，重置为0
      }
      this->reg(REG_FIFO_WR_PTR) = wr_ptr;
      num_samples=1;
    } else {
      return;  // 如果没有数据，直接返回
    }
  }
  uint8_t bytes_per_sample = this->active_leds_ * 3;  // 每个样本的字节数 = 激活LED数 * 3
//  std::vector<uint8_t> data;
//  for (int i = 0; i < num_samples * bytes_per_sample; i++) {
//    data.push_back(this->reg(REG_FIFO_DATA).get());
//  }
  this->write(&REG_FIFO_DATA, 1); // burst read
  uint8_t *data = new uint8_t[num_samples * bytes_per_sample];  // 最多32个样本，每个样本12个字节
  this->read(data, num_samples * bytes_per_sample);

  MAX30105_RESOLUTION res = (MAX30105_RESOLUTION)(this->reg(REG_SPO2_CONFIG).get() & 0x03);  // 获取分辨率
  uint8_t bit;
  if (res == MAX30105_RESOLUTION_15_BIT) {
    bit = 3;
  } else if (res == MAX30105_RESOLUTION_16_BIT) {
    bit = 2;
  } else if (res == MAX30105_RESOLUTION_17_BIT) {
    bit = 1;
  } else {
    bit = 0;
  }
  int index = ((int) num_samples - 1) * ((int) bytes_per_sample);  // 只要最后一个sample
  uint32_t sample = 0;
  for (int i = 0; i < this->active_leds_; i++) {
    sample = (((uint32_t) data[index + i * 3]) << 16) | (((uint32_t) data[index + i * 3 + 1]) << 8) |
             (((uint32_t) data[index + i * 3 + 2]));  // 每个LED占3个字节
    if (i == 0 && this->led1_sensor_ != nullptr) {
      this->led1_sensor_->publish_state(sample >> bit);  // 发布LED1数据
    } else if (i == 1 && this->led2_sensor_ != nullptr) {
      this->led2_sensor_->publish_state(sample >> bit);  // 发布LED2数据
    } else if (i == 2 && this->led3_sensor_ != nullptr) {
      this->led3_sensor_->publish_state(sample >> bit);  // 发布LED3数据
    } else if (i == 3 && this->led4_sensor_ != nullptr) {
      this->led4_sensor_->publish_state(sample >> bit);  // 发布LED4数据
    }
  }
  delete[] data;
  //  std::vector<std::vector<uint32_t>> samples;
}

void MAX30105Component::read_overflow_counter() {
#ifdef USE_SENSOR
  uint8_t overflow_count = 0;
  if (this->fifo_overflow_counter_sensor_ != nullptr) {
    overflow_count = this->reg(REG_OVF_COUNTER).get();  // 获取溢出计数
    this->fifo_overflow_counter_sensor_->publish_state(overflow_count);
  }
#endif
}


void MAX30105Component::set_proximity_threshold_reg(uint8_t threshold) { this->reg(REG_PROX_INT_THRESH) = threshold; }

void MAX30105Component::read_temperature() { this->set_bit(REG_TEMP_CONFIG, 0); }

void MAX30105Component::enable_interrupts(bool fifo_almost_full, bool data_ready, bool alc_overflow, bool prox_int,
                                          bool temp_ready) {
  // 启用中断
  uint8_t intr_en1 = 0, intr_en2 = 0;
  if (fifo_almost_full) {
    intr_en1 |= MAX30105_INTERRUPT_FIFO_FULL;
  }
  if (data_ready) {
    intr_en1 |= MAX30105_INTERRUPT_DATA_RDY;
  }
  if (alc_overflow) {
    intr_en1 |= MAX30105_INTERRUPT_ALC_OVF;
  }
  if (prox_int) {
    intr_en1 |= MAX30105_INTERRUPT_PROXIMITY;
  }
  if (temp_ready) {
    intr_en2 |= MAX30105_INTERRUPT_TEMP_RDY;
  }
  this->reg(REG_INTR_ENABLE_1) = intr_en1;
  this->reg(REG_INTR_ENABLE_2) = intr_en2;  // 设置第二个寄存器
}

void MAX30105Component::set_multi_led_slots_reg(std::vector<uint8_t>& slots) {
  size_t size = slots.size();
  this->active_leds_ = (uint8_t) size;
  uint8_t reg11 = 0, reg12 = 0;
  if (size > 0) {
    reg11 |= slots[0];
  }
  if (size > 1) {
    reg11 |= (slots[1] << 4);
  }
  if (size > 2) {
    reg12 |= slots[2];
  }
  if (size > 3) {
    reg12 |= (slots[3] << 4);
  }
  this->reg(REG_MULTI_LED_CTRL1) = reg11;  // 设置第一个寄存器
  this->reg(REG_MULTI_LED_CTRL2) = reg12;  // 设置第二个寄存器
}

void MAX30105Component::set_led_current_reg(uint8_t red_current, uint8_t ir_current, uint8_t green_current,
                                            uint8_t pilot_current) {
  // 设置LED电流寄存器
  this->reg(REG_LED1_PA) = red_current;
  this->reg(REG_LED2_PA) = ir_current;
  this->reg(REG_LED3_PA) = green_current;
  this->reg(REG_PILOT_PA) = pilot_current;
}

void MAX30105Component::set_mode_reg(MAX30105_MODE mode) {
  uint8_t current = this->reg(REG_MODE_CONFIG).get();
  uint8_t new_config = (current & 0xF8) | mode;
  this->reg(REG_MODE_CONFIG) = new_config;
}

bool MAX30105Component::verify_part_id() {
  uint8_t part_id = this->reg(REG_PART_ID).get();  // 读取REV_ID寄存器
  if (part_id != EXPECTED_PART_ID) {
    ESP_LOGE(TAG, "Part ID mismatch! Expected: 0x%02X, Got: 0x%02X", EXPECTED_PART_ID, part_id);
    return false;  // 如果ID不匹配，抛出错误
  }
  return true;  // 如果ID匹配，返回true
}

void MAX30105Component::reset() {
  this->set_bit(REG_MODE_CONFIG, 6);  // 设置MODE_CONFIG寄存器的第6位为1以复位
}

void MAX30105Component::set_bit(uint8_t addr, uint8_t bit) {
  uint8_t value = this->reg(addr).get();
  value |= (1 << bit);     // 设置指定的位
  this->reg(addr) = value;  // 写回寄存器
}

void MAX30105Component::clear_bit(uint8_t addr, uint8_t bit) {
  uint8_t value = this->reg(addr).get();
  this->reg(addr) = value & ~(1 << bit);  // 写回寄存器
}

void MAX30105Component::shutdown() {
  this->set_bit(REG_MODE_CONFIG, 7);  // 设置MODE_CONFIG寄存器的第7位为1以进入省电模式
}

void MAX30105Component::wakeup() {
  this->clear_bit(REG_MODE_CONFIG, 7);  // 清除MODE_CONFIG寄存器的第7位以唤醒设备
}

void MAX30105Component::clear_fifo() {
  this->reg(REG_FIFO_WR_PTR) = 0;  // 清除FIFO写指针
  this->reg(REG_OVF_COUNTER) = 0;  // 清除溢出计数器
  this->reg(REG_FIFO_RD_PTR) = 0;  // 清除FIFO读指针
}

void MAX30105Component::loop() {
  if (this->interrupt_) {
    uint8_t status1 = this->reg(REG_INTR_STATUS_1).get();  // 读取中断状态寄存器以清除中断
    uint8_t status2 = this->reg(REG_INTR_STATUS_2).get();  // 读取中断状态寄存器以清除中断
    if (status1 & MAX30105_INTERRUPT_PWR_RDY) {
#ifdef USE_BINARY_SENSOR
      if (this->power_ready_binary_sensor_ != nullptr) {
        this->power_ready_binary_sensor_->publish_state(true);
      }
#endif
      this->on_power_ready_callback_.call();
    }
    if (status1 & MAX30105_INTERRUPT_PROXIMITY) {
#ifdef USE_BINARY_SENSOR
      if (this->target_binary_sensor_ != nullptr) {
        this->target_binary_sensor_->publish_state(true);
      }
#endif
      this->on_prox_int_callback_.call();
    }
    if (status1 & MAX30105_INTERRUPT_ALC_OVF) {
#ifdef USE_BINARY_SENSOR
      if (this->alc_overflow_binary_sensor_ != nullptr) {
        this->alc_overflow_binary_sensor_->publish_state(true);
      }
#endif
      this->on_alc_overflow_callback_.call();
    }
    if (status1 & MAX30105_INTERRUPT_DATA_RDY) {
#ifdef USE_BINARY_SENSOR
      if (this->data_ready_binary_sensor_ != nullptr) {
        this->data_ready_binary_sensor_->publish_state(true);
      }
#endif
      this->on_data_ready_callback_.call();
    }
    if (status1 & MAX30105_INTERRUPT_FIFO_FULL) {
#ifdef USE_BINARY_SENSOR
      if (this->fifo_full_binary_sensor_ != nullptr) {
        this->fifo_full_binary_sensor_->publish_state(true);
      }
#endif
      this->read_overflow_counter();
      this->on_fifo_almost_full_callback_.call();
    }
    if (status2 & MAX30105_INTERRUPT_TEMP_RDY) {
#ifdef USE_BINARY_SENSOR
      if (this->temperature_ready_binary_sensor_ != nullptr) {
        this->temperature_ready_binary_sensor_->publish_state(true);
      }
#endif
      int32_t temp_int = (int32_t)this->reg(REG_TEMP_INT).get();
      uint8_t temp_frac = this->reg(REG_TEMP_FRAC).get(); // & 0x0F;  // 只保留低4位
      if (temp_int > 127) {
        temp_int -= 256;
      }
      float temperature = (float) temp_int + ((float) temp_frac) * 0.0625;
#ifdef USE_SENSOR
      if (this->temperature_sensor_ != nullptr) {
        this->temperature_sensor_->publish_state(temperature);
      }
#endif
      this->on_temp_ready_callback_.call(temperature);
    }
    this->interrupt_ = false;
    this->need_clear_ = true;
    return;
  }
  if (this->need_clear_) {
#ifdef USE_BINARY_SENSOR
    if (this->power_ready_binary_sensor_ != nullptr) {
      this->power_ready_binary_sensor_->publish_state(false);
    }
    if (this->target_binary_sensor_ != nullptr) {
      this->target_binary_sensor_->publish_state(false);
    }
    if (this->alc_overflow_binary_sensor_ != nullptr) {
      this->alc_overflow_binary_sensor_->publish_state(false);
    }
    if (this->data_ready_binary_sensor_ != nullptr) {
      this->data_ready_binary_sensor_->publish_state(false);
    }
    if (this->fifo_full_binary_sensor_ != nullptr) {
      this->fifo_full_binary_sensor_->publish_state(false);
    }
    if (this->temperature_ready_binary_sensor_ != nullptr) {
      this->temperature_ready_binary_sensor_->publish_state(false);
    }
#endif
    this->need_clear_ = false;
  }
}

void MAX30105Component::simulate_interrupt() {
  // 模拟中断
  this->interrupt_ = true;
}

} // namespace esphome::max30105
