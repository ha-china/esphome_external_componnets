#include "as762x.h"
#include <bitset>
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::as762x {

static const char *const TAG = "as762x";
static const uint8_t REG_STATUS = 0x00;  // read-only
static const uint8_t REG_WRITE = 0x01;   // write-only
static const uint8_t REG_READ = 0x02;    // read-only

static float bytes_to_float(uint32_t myLong) {
  float myFloat;
  memcpy(&myFloat, &myLong, 4);  // Copy uint8_ts into a float
  return (myFloat);
}

void IRAM_ATTR AS762XComponent::irq(AS762XComponent *c) { c->interrupt_ = true; }

void AS762XComponent::loop() {
  if (this->interrupt_) {
    this->on_data_ready_.call();
    this->interrupt_ = false;
  }
}

void AS762XComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  // Set the default configuration
  uint8_t version = this->read_virtual_register(AS726x_HW_VERSION);
  if (version != SENSORTYPE_AS7261 && version != SENSORTYPE_AS7262 && version != SENSORTYPE_AS7263) {
    this->mark_failed();
    return;
  }
  this->set_control_reg(false, this->interrupt_output_, this->gain_, this->conversion_type_);
  //  this->enable_interrupt(this->interrupt_output_);
  //  this->set_gain_reg(this->gain_);
  //  this->set_conversion_type_reg(this->conversion_type_);
  this->set_integration_time_reg(this->integration_time_);
  this->set_led_reg(this->led_drv_current_, this->led_drv_, this->led_ind_current_, this->led_ind_);
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    this->interrupt_pin_->setup();
    this->interrupt_pin_->attach_interrupt(AS762XComponent::irq, this, gpio::INTERRUPT_FALLING_EDGE);
  }
}

void AS762XComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AS762X:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Channel 1 Sensor", this->channel_1_sensor_);
  LOG_SENSOR("  ", "Channel 2 Sensor", this->channel_2_sensor_);
  LOG_SENSOR("  ", "Channel 3 Sensor", this->channel_3_sensor_);
  LOG_SENSOR("  ", "Channel 4 Sensor", this->channel_4_sensor_);
  LOG_SENSOR("  ", "Channel 5 Sensor", this->channel_5_sensor_);
  LOG_SENSOR("  ", "Channel 6 Sensor", this->channel_6_sensor_);
  LOG_SENSOR("  ", "Calibrated X Sensor", this->calibrated_x_sensor_);
  LOG_SENSOR("  ", "Calibrated Y Sensor", this->calibrated_y_sensor_);
  LOG_SENSOR("  ", "Calibrated Z Sensor", this->calibrated_z_sensor_);
  LOG_SENSOR("  ", "Calibrated X1931 Sensor", this->calibrated_x1931_sensor_);
  LOG_SENSOR("  ", "Calibrated Y1931 Sensor", this->calibrated_y1931_sensor_);
  LOG_SENSOR("  ", "Calibrated UPRI Sensor", this->calibrated_upri_sensor_);
  LOG_SENSOR("  ", "Calibrated VPRI Sensor", this->calibrated_vpri_sensor_);
  LOG_SENSOR("  ", "Calibrated U Sensor", this->calibrated_u_sensor_);
  LOG_SENSOR("  ", "Calibrated V Sensor", this->calibrated_v_sensor_);
  LOG_SENSOR("  ", "Calibrated DUV Sensor", this->calibrated_duv_sensor_);
  LOG_SENSOR("  ", "Calibrated Lux Sensor", this->calibrated_lux_sensor_);
  LOG_SENSOR("  ", "Calibrated CCT Sensor", this->calibrated_cct_sensor_);
}

void AS762XComponent::update() {
  if (this->conversion_type_ == AS762X_CONVERSION_TYPE_3) {
    this->clear_data_available();
    this->set_conversion_type_reg(AS762X_CONVERSION_TYPE_3);
    uint32_t timeout = millis() + AS762X_TIMEOUT;
    while (!this->data_available()) {
      // wait for data to be available
      delay(POLLING_DELAY);
      if (millis() > timeout) {
        ESP_LOGW(TAG, "Timeout waiting for data to be available");
        this->status_set_warning();
        return;
      }
    }
  }
  if (this->channel_1_sensor_ != nullptr) {
    uint16_t x = this->get_x();
    this->channel_1_sensor_->publish_state(x);
  }
  if (this->channel_2_sensor_ != nullptr) {
    uint16_t y = this->get_y();
    this->channel_2_sensor_->publish_state(y);
  }
  if (this->channel_3_sensor_ != nullptr) {
    uint16_t z = this->get_z();
    this->channel_3_sensor_->publish_state(z);
  }
  if (this->channel_4_sensor_ != nullptr) {
    uint16_t nir = this->get_nir();
    this->channel_4_sensor_->publish_state(nir);
  }
  if (this->channel_5_sensor_ != nullptr) {
    uint16_t dark = this->get_dark();
    this->channel_5_sensor_->publish_state(dark);
  }
  if (this->channel_6_sensor_ != nullptr) {
    uint16_t clear = this->get_clear();
    this->channel_6_sensor_->publish_state(clear);
  }
  if (this->calibrated_x_sensor_ != nullptr) {
    float calibrated_x = this->get_calibrated_x();
    this->calibrated_x_sensor_->publish_state(calibrated_x);
  }
  if (this->calibrated_y_sensor_ != nullptr) {
    float calibrated_y = this->get_calibrated_y();
    this->calibrated_y_sensor_->publish_state(calibrated_y);
  }
  if (this->calibrated_z_sensor_ != nullptr) {
    float calibrated_z = this->get_calibrated_z();
    this->calibrated_z_sensor_->publish_state(calibrated_z);
  }
  if (this->calibrated_x1931_sensor_ != nullptr) {
    float calibrated_x1931 = this->get_calibrated_x1931();
    this->calibrated_x1931_sensor_->publish_state(calibrated_x1931);
  }
  if (this->calibrated_y1931_sensor_ != nullptr) {
    float calibrated_y1931 = this->get_calibrated_y1931();
    this->calibrated_y1931_sensor_->publish_state(calibrated_y1931);
  }
  if (this->calibrated_upri_sensor_ != nullptr) {
    float calibrated_upri = this->get_calibrated_upri();
    this->calibrated_upri_sensor_->publish_state(calibrated_upri);
  }
  if (this->calibrated_vpri_sensor_ != nullptr) {
    float calibrated_vpri = this->get_calibrated_vpri();
    this->calibrated_vpri_sensor_->publish_state(calibrated_vpri);
  }
  if (this->calibrated_u_sensor_ != nullptr) {
    float calibrated_u = this->get_calibrated_u();
    this->calibrated_u_sensor_->publish_state(calibrated_u);
  }
  if (this->calibrated_v_sensor_ != nullptr) {
    float calibrated_v = this->get_calibrated_v();
    this->calibrated_v_sensor_->publish_state(calibrated_v);
  }
  if (this->calibrated_duv_sensor_ != nullptr) {
    float calibrated_duv = this->get_calibrated_duv();
    this->calibrated_duv_sensor_->publish_state(calibrated_duv);
  }
  if (this->calibrated_lux_sensor_ != nullptr) {
    float calibrated_lux = this->get_calibrated_lux();
    this->calibrated_lux_sensor_->publish_state(calibrated_lux);
  }
  if (this->calibrated_cct_sensor_ != nullptr) {
    float calibrated_cct = this->get_calibrated_cct();
    this->calibrated_cct_sensor_->publish_state(calibrated_cct);
  }
  if (this->temperature_sensor_ != nullptr) {
    uint8_t temp = this->get_temperature();
    this->temperature_sensor_->publish_state(temp);
  }
  this->status_clear_warning();
}

float AS762XComponent::get_calibrated_x() {
  return this->get_calibrated_value(AS7261_X_CAL);
}

float AS762XComponent::get_calibrated_y() {
  return this->get_calibrated_value(AS7261_Y_CAL);
}

float AS762XComponent::get_calibrated_z() {
  return this->get_calibrated_value(AS7261_Z_CAL);
}

float AS762XComponent::get_calibrated_x1931() {
  return this->get_calibrated_value(AS7261_X1931_CAL);
}

float AS762XComponent::get_calibrated_y1931() {
  return this->get_calibrated_value(AS7261_Y1931_CAL);
}

float AS762XComponent::get_calibrated_upri() {
  return this->get_calibrated_value(AS7261_UPRI_CAL);
}

float AS762XComponent::get_calibrated_vpri() {
  return this->get_calibrated_value(AS7261_VPRI_CAL);
}

float AS762XComponent::get_calibrated_u() {
  return this->get_calibrated_value(AS7261_U_CAL);
}

float AS762XComponent::get_calibrated_v() {
  return this->get_calibrated_value(AS7261_V_CAL);
}

float AS762XComponent::get_calibrated_duv() {
  return this->get_calibrated_value(AS7261_DUV_CAL);
}

float AS762XComponent::get_calibrated_lux() {
  return this->get_calibrated_value(AS7261_LUX_CAL);
}

float AS762XComponent::get_calibrated_cct() {
  return this->get_calibrated_value(AS7261_CCT_CAL);
}
// normal
uint16_t AS762XComponent::get_x() {
  return this->get_channel(AS7261_X);
}

uint16_t AS762XComponent::get_y() {
  return this->get_channel(AS7261_Y);
}

uint16_t AS762XComponent::get_z() {
  return this->get_channel(AS7261_Z);
}

uint16_t AS762XComponent::get_nir() {
  return this->get_channel(AS7261_NIR);
}

uint16_t AS762XComponent::get_dark() {
  return this->get_channel(AS7261_DARK);
}

uint16_t AS762XComponent::get_clear() {
  return this->get_channel(AS7261_CLEAR);
}


// A the 16-bit value stored in a given channel registerReturns
uint16_t AS762XComponent::get_channel(uint8_t addr) {
  uint16_t color_data = ((uint16_t) this->read_virtual_register(addr)) << 8;  // High uint8_t
  color_data |= (uint16_t) this->read_virtual_register(addr + 1);             // Low uint8_t
  return color_data;
}

void AS762XComponent::set_led_reg(AS762X_LED_DRV_CURRENT drv_current, bool led_drv, AS762X_LED_IND_CURRENT ind_current,
                                  bool led_ind) {
  std::bitset<8> config;
  switch (drv_current) {
    case AS762X_LED_DRV_CURRENT_12_5MA:
      config[5] = false;  // 12.5mA
      config[4] = false;  // 12.5mA
      break;
    case AS762X_LED_DRV_CURRENT_25MA:
      config[5] = false;  // 25mA
      config[4] = true;   // 25mA
      break;
    case AS762X_LED_DRV_CURRENT_50MA:
      config[5] = true;   // 50mA
      config[4] = false;  // 50mA
      break;
    case AS762X_LED_DRV_CURRENT_100MA:
      config[5] = true;  // 100mA
      config[4] = true;  // 100mA
      break;
  }
  if (led_drv) {
    config[3] = true;  // Enable LED driver
  } else {
    config[3] = false;  // Disable LED driver
  }
  switch (ind_current) {
    case AS762X_LED_IND_CURRENT_1MA:
      config[2] = false;  // 1mA
      config[1] = false;  // 1mA
      break;
    case AS762X_LED_IND_CURRENT_2MA:
      config[2] = false;  // 2mA
      config[1] = true;   // 2mA
      break;
    case AS762X_LED_IND_CURRENT_4MA:
      config[2] = true;   // 4mA
      config[1] = false;  // 4mA
      break;
    case AS762X_LED_IND_CURRENT_8MA:
      config[2] = true;  // 8mA
      config[1] = true;  // 8mA
      break;
  }
  if (led_ind) {
    config[0] = true;  // Enable LED indicator
  } else {
    config[0] = false;  // Disable LED indicator
  }
  this->write_virtual_register(AS726x_LED_CONTROL, config.to_ulong());
}

// Returns the temperature in C
// Pretty inaccurate: +/-8.5C
uint8_t AS762XComponent::get_temperature() { return this->read_virtual_register(AS726x_DEVICE_TEMP); }

// Sets the integration value
// Give this function a uint8_t from 0 to 255.
// Time will be 2.8ms * [integration value]
void AS762XComponent::set_integration_time_reg(uint8_t time) { this->write_virtual_register(AS726x_INT_T, time); }

uint8_t AS762XComponent::get_integration_time_reg(uint8_t time) { return this->read_virtual_register(AS726x_INT_T); }

bool AS762XComponent::data_available() {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  return config[1];
}

void AS762XComponent::clear_data_available() {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  config[1] = false;  // Clear the data available bit
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

void AS762XComponent::set_control_reg(bool reset, bool interrupt, AS762X_GAIN gain,
                                      AS762X_CONVERSION_TYPE conversion_type) {
  std::bitset<8> config;
  config[7] = reset;      // Set the reset bit
  config[6] = interrupt;  // Set the interrupt bit
  switch (gain) {
    case AS762X_GAIN_1X:
      config[5] = false;  // Gain 1x
      config[4] = false;  // Gain 1x
      break;
    case AS762X_GAIN_3_7X:
      config[5] = false;  // Gain 3.7x
      config[4] = true;   // Gain 3.7x
      break;
    case AS762X_GAIN_16X:
      config[5] = true;   // Gain 16x
      config[4] = false;  // Gain 16x
      break;
    case AS762X_GAIN_64X:
      config[5] = true;  // Gain 64x
      config[4] = true;  // Gain 64x
      break;
  }
  switch (conversion_type) {
    case AS762X_CONVERSION_TYPE_0:
      config[3] = false;  // Mode 0: X, Y, Z and NIR
      config[2] = false;
      break;
    case AS762X_CONVERSION_TYPE_1:
      config[3] = false;  // Mode 1: X, Y, D and C
      config[2] = true;
      break;
    case AS762X_CONVERSION_TYPE_2:
      config[3] = true;  // Mode 2: X, Y, Z, NIR, D and C
      config[2] = false;
      break;
    case AS762X_CONVERSION_TYPE_3:
      config[3] = true;  // Mode 3: One-Shot operation
      config[2] = true;
      break;
  }
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

AS762X_CONVERSION_TYPE AS762XComponent::get_conversion_type_reg() {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  if (!config[3] && !config[2]) {
    return AS762X_CONVERSION_TYPE_0;  // Mode 0: X, Y, Z and NIR
  } else if (!config[3] && config[2]) {
    return AS762X_CONVERSION_TYPE_1;  // Mode 1: X, Y, D and C
  } else if (config[3] && !config[2]) {
    return AS762X_CONVERSION_TYPE_2;  // Mode 2: X, Y, Z, NIR, D and C
  } else {
    return AS762X_CONVERSION_TYPE_3;  // Mode 3: One-Shot operation
  }
}

void AS762XComponent::set_conversion_type_reg(AS762X_CONVERSION_TYPE conversion_type) {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  switch (conversion_type) {
    case AS762X_CONVERSION_TYPE_0:
      config[3] = false;  // Mode 0: X, Y, Z and NIR
      config[2] = false;
      break;
    case AS762X_CONVERSION_TYPE_1:
      config[3] = false;  // Mode 1: X, Y, D and C
      config[2] = true;
      break;
    case AS762X_CONVERSION_TYPE_2:
      config[3] = true;  // Mode 2: X, Y, Z, NIR, D and C
      config[2] = false;
      break;
    case AS762X_CONVERSION_TYPE_3:
      config[3] = true;  // Mode 3: One-Shot operation
      config[2] = true;
      break;
  }
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

AS762X_GAIN AS762XComponent::get_gain_reg() {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  if (!config[5] && !config[4]) {
    return AS762X_GAIN_1X;  // Gain 1x
  } else if (!config[5] && config[4]) {
    return AS762X_GAIN_3_7X;  // Gain 3.7x
  } else if (config[5] && !config[4]) {
    return AS762X_GAIN_16X;  // Gain 16x
  } else {
    return AS762X_GAIN_64X;  // Gain 64x
  }
}

void AS762XComponent::set_gain_reg(AS762X_GAIN gain) {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  switch (gain) {
    case AS762X_GAIN_1X:
      config[5] = false;  // Gain 1x
      config[4] = false;  // Gain 1x
      break;
    case AS762X_GAIN_3_7X:
      config[5] = false;  // Gain 3.7x
      config[4] = true;   // Gain 3.7x
      break;
    case AS762X_GAIN_16X:
      config[5] = true;   // Gain 16x
      config[4] = false;  // Gain 16x
      break;
    case AS762X_GAIN_64X:
      config[5] = true;  // Gain 64x
      config[4] = true;  // Gain 64x
      break;
  }
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

// Does a soft reset
// Give sensor at least 1000ms to reset
void AS762XComponent::reset() {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  config[7] = true;  // Set the reset bit
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

void AS762XComponent::enable_interrupt(bool enable) {
  std::bitset<8> config = this->read_virtual_register(AS726x_CONTROL_SETUP);
  config[6] = enable;  // Set the reset bit
  this->write_virtual_register(AS726x_CONTROL_SETUP, config.to_ulong());
}

// Given an address, read four uint8_ts and return the floating point calibrated value
float AS762XComponent::get_calibrated_value(uint8_t addr) {
  uint8_t b0, b1, b2, b3;
  b0 = this->read_virtual_register(addr + 0);
  b1 = this->read_virtual_register(addr + 1);
  b2 = this->read_virtual_register(addr + 2);
  b3 = this->read_virtual_register(addr + 3);

  // Channel calibrated values are stored big-endian
  uint32_t calBytes = 0;
  calBytes |= ((uint32_t) b0 << 24);
  calBytes |= ((uint32_t) b1 << 16);
  calBytes |= ((uint32_t) b2 << 8);
  calBytes |= ((uint32_t) b3);

  return bytes_to_float(calBytes);
}

void AS762XComponent::get_status(bool *can_read, bool *can_write) {
  std::bitset<8> status = this->reg(REG_STATUS).get();
  if (status[1]) {
    *can_write = false;
  } else {
    *can_write = true;
  }
  if (status[0]) {
    *can_read = true;
  } else {
    *can_read = false;
  }
}

uint8_t AS762XComponent::read_virtual_register(uint8_t addr) {
  bool can_read, can_write;
  this->get_status(&can_read, &can_write);
  if (can_read) {
    uint8_t incoming = this->reg(REG_READ).get();  // Read the uint8_t but do nothing with it
  }
  uint8_t status;
  uint8_t retries = 0;

  while (true) {
    //    status = this->reg(REG_STATUS).get();
    //    if (status == 0xFF) {
    //      return status;
    //    } todo is this needed? the datasheet didn't do this
    //    https://github.com/sparkfun/SparkFun_AS726X_Arduino_Library/blob/master/src/AS726X.cpp#L386
    this->get_status(&can_read, &can_write);
    if (can_write) {
      break;
    }
    delay(POLLING_DELAY);
    if (retries++ > MAX_RETRIES) {
      return 0xFF;
    }
  }
  this->reg(REG_WRITE) = addr;
  retries = 0;
  while (true) {
    this->get_status(&can_read, &can_write);
    if (can_read) {
      break;
    }
    delay(POLLING_DELAY);
    if (retries++ > MAX_RETRIES) {
      return 0xFF;
    }
  }
  uint8_t incoming = this->reg(REG_READ).get();
  return incoming;
}

uint8_t AS762XComponent::write_virtual_register(uint8_t addr, uint8_t data) {
  uint8_t status;
  uint8_t retries = 0;
  bool can_read, can_write;
  while (true) {
    this->get_status(&can_read, &can_write);
    if (can_write) {
      break;
    }
    delay(POLLING_DELAY);
    if (retries++ > MAX_RETRIES) {
      return 1;
    }
  }
  this->reg(REG_WRITE) = addr | 0x80;  // Set the write bit
  retries = 0;
  while (true) {
    this->get_status(&can_read, &can_write);
    if (can_write) {
      break;
    }
    delay(POLLING_DELAY);
    if (retries++ > MAX_RETRIES) {
      return 1;
    }
  }
  this->reg(REG_WRITE) = data;
  return 0;  // ok
}

} // namespace esphome::as762x
