#include "as201.h"
#include <bitset>
#include "esphome/core/log.h"

namespace esphome {
namespace as201 {

static const char *const TAG = "as201";

static uint8_t AS201_CMD_HEAD[2] = {0xFA, 0xFB};
static uint8_t AS201_CMD_TAIL[2] = {0xFC, 0xFD};

void AS201Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AS201...");
  this->version();
  this->get_install_params();
#ifdef USE_SELECT
  if (this->baud_rate_select_ != nullptr) {
    this->baud_rate_select_->publish_state(std::to_string(this->parent_->get_baud_rate()));
  }
#endif
  this->get_config();
}

void AS201Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS201:");
#ifdef USE_SENSOR
  LOG_SENSOR(" ", "Accel X", this->accel_x_sensor_);
  LOG_SENSOR(" ", "Accel Y", this->accel_y_sensor_);
  LOG_SENSOR(" ", "Accel Z", this->accel_z_sensor_);
  LOG_SENSOR(" ", "Gyro X", this->gyro_x_sensor_);
  LOG_SENSOR(" ", "Gyro Y", this->gyro_y_sensor_);
  LOG_SENSOR(" ", "Gyro Z", this->gyro_z_sensor_);
  LOG_SENSOR(" ", "Angle X", this->angle_x_sensor_);
  LOG_SENSOR(" ", "Angle Y", this->angle_y_sensor_);
  LOG_SENSOR(" ", "Angle Z", this->angle_z_sensor_);
  LOG_SENSOR(" ", "Field Strength X", this->field_strength_x_sensor_);
  LOG_SENSOR(" ", "Field Strength Y", this->field_strength_y_sensor_);
  LOG_SENSOR(" ", "Field Strength Z", this->field_strength_z_sensor_);
  LOG_SENSOR(" ", "Q0", this->q0_sensor_);
  LOG_SENSOR(" ", "Q1", this->q1_sensor_);
  LOG_SENSOR(" ", "Q2", this->q2_sensor_);
  LOG_SENSOR(" ", "Q3", this->q3_sensor_);
  LOG_SENSOR(" ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR(" ", "Pressure", this->pressure_sensor_);
  LOG_SENSOR(" ", "Height", this->height_sensor_);
#endif
}

void AS201Component::loop() {
  uint8_t peeked;
  while (this->available()) {
    if (!this->head_found && this->peek_byte(&peeked) && peeked != AS201_CMD_HEAD[0]) {
      this->read();
    } else {
      this->head_found = true;  // fixme 重复设置head_found
    }
    if (this->head_found) {
      this->receive_buffer.push_back((uint8_t) this->read());
    }
  }
  if (this->receive_buffer.size() < 5) {  // head + len + tail
    return;                               // Not enough data to process
  }
  uint8_t data_size = this->receive_buffer[2];
  if (this->receive_buffer.size() < (5 + data_size)) {  // head + len + tail
    return;                                             // Not enough data to process
  }
  if (memcpy(this->receive_buffer.data(), AS201_CMD_HEAD, 2) != 0) {
    this->head_found = false;
    this->receive_buffer.clear();
    return;
  }
  if (memcpy(this->receive_buffer.data() + data_size + 3, AS201_CMD_TAIL, 2) != 0) {
    this->head_found = false;
    this->receive_buffer.clear();
    return;
  }
  uint8_t cmd = this->receive_buffer[3];
  uint8_t checksum = this->receive_buffer[data_size + 2];
  uint8_t calculated_checksum = 0;
  for (uint8_t i = 0; i < data_size-1; i++) {
    calculated_checksum += this->receive_buffer[3 + i];
  }
  if (calculated_checksum != checksum) {
    ESP_LOGW(TAG, "Checksum error: expected 0x%02X, got 0x%02X", checksum, calculated_checksum);
    this->head_found = false;
    this->receive_buffer.clear();
    return;
  }
  switch (cmd) {
    case 0x00: {
      this->parse_data();
      break;
    }
    case 0x10: {  // get version resp
#ifdef USE_TEXT_SENSOR
      uint8_t minor = this->receive_buffer[4];
      uint8_t major = this->receive_buffer[5];
      uint8_t hour = this->receive_buffer[6];
      uint8_t day = this->receive_buffer[7];
      uint8_t month = this->receive_buffer[8];
      uint16_t year = (uint16_t) this->receive_buffer[9];
      if (this->version_text_sensor_ != nullptr) {
        this->version_text_sensor_->publish_state(std::to_string(major) + "." + std::to_string(minor) + " " +
                                                  std::to_string(year) + "-" + std::to_string(month) + "-" +
                                                  std::to_string(day) + " " + std::to_string(hour) + ":00");
      }
#endif
      break;
    }
    case 0x11: {  // factory reset resp
      ESP_LOGD(TAG, "Factory reset successful");
      break;
    }
    case 0x12: {  // z axis to zero resp
      ESP_LOGD(TAG, "Z axis set to zero successful");
      break;
    }
    case 0x13: {  // euler angle reset resp
      ESP_LOGD(TAG, "reset euler angle successful");
      break;
    }
    case 0x14: {  // get install direction resp
#ifdef USE_SELECT
      if (this->direction_select_ != nullptr) {
        if (this->receive_buffer[4]) {  // 垂直
          this->direction_select_->publish_state("vertical");
        } else {  // 水平
          this->direction_select_->publish_state("horizontal");
        }
      }
#endif
      break;
    }
    case 0x15: {  // set install direction resp
      ESP_LOGD(TAG, "Set install direction successful");
      break;
    }
    case 0x16: {  // set upload rate resp
      ESP_LOGD(TAG, "Set upload rate successful");
      break;
    }
    case 0x17: {  // factory reset resp
      ESP_LOGD(TAG, "restart successful");
      break;
    }
    case 0x18: {  // set baud rate resp
      ESP_LOGD(TAG, "Set new baud rate successful");
      this->parent_->load_settings();
      break;
    }
    case 0x19: {  // get config resp
      uint8_t sub = this->receive_buffer[4];
      uint8_t rate = this->receive_buffer[5];
      uint8_t baud = this->receive_buffer[6];
      uint8_t up = this->receive_buffer[7];
#ifdef USE_SELECT
      if (this->upload_rate_select_ != nullptr) {
        switch (rate) {
          case 0x01:
            this->upload_rate_select_->publish_state("0.1Hz");
            break;
          case 0x02:
            this->upload_rate_select_->publish_state("0.5Hz");
            break;
          case 0x03:
            this->upload_rate_select_->publish_state("1Hz");
            break;
          case 0x04:
            this->upload_rate_select_->publish_state("2Hz");
            break;
          case 0x05:
            this->upload_rate_select_->publish_state("5Hz");
            break;
          case 0x06:
            this->upload_rate_select_->publish_state("10Hz");
            break;
          case 0x07:
            this->upload_rate_select_->publish_state("20Hz");
            break;
          default:
            this->upload_rate_select_->publish_state("20Hz");
        }
      }
#endif
#ifdef USE_SWITCH
      if (this->enable_upload_switch_ != nullptr) {
        if (up == 0x01) {
          this->enable_upload_switch_->publish_state(true);
        } else {
          this->enable_upload_switch_->publish_state(false);
        }
      }
#endif
      break;
    }
    case 0x1A: {  // enable upload resp
      ESP_LOGD(TAG, "toggle upload successful");
      break;
    }
    case 0x1B: {
      this->parse_data();
      break;
    }
    case 0x1C: {  // error resp
      if (this->receive_buffer[4]) {
        ESP_LOGI(TAG, "calibrate done");
      } else {
        ESP_LOGW(TAG, "calibrate failed");
      }
      break;
    }
    case 0x1D: {
      ESP_LOGI(TAG, "begin calibrate magnetic field");
      break;
    }
    case 0x1E: {
      ESP_LOGI(TAG, "finish calibrate magnetic field");
      break;
    }
  }
  this->receive_buffer.erase(this->receive_buffer.begin(), this->receive_buffer.begin() + data_size + 5);
}

void AS201Component::parse_data() {
  uint8_t sensor_type = this->receive_buffer[4];
  std::bitset<8> sensor_type_bits(sensor_type);
#ifdef USE_TEXT_SENSOR
  if (this->type_text_sensor_ != nullptr) {
    if (!sensor_type_bits[0] && !sensor_type_bits[1]) {
      this->type_text_sensor_->publish_state("10 axis");
    } else if (!sensor_type_bits[0] && sensor_type_bits[1]) {
      this->type_text_sensor_->publish_state("9 axis");
    } else if (sensor_type_bits[0] && !sensor_type_bits[1]) {
      this->type_text_sensor_->publish_state("6 axis");
    } else {
      this->type_text_sensor_->publish_state("unknown");
    }
  }
  if (this->accuracy_text_sensor_ != nullptr) {
    if (!sensor_type_bits[2] && !sensor_type_bits[3]) {
      this->accuracy_text_sensor_->publish_state("not calibrated or have magnet field interference");
    } else if (!sensor_type_bits[2] && sensor_type_bits[3]) {
      this->accuracy_text_sensor_->publish_state("have magnet field interference");
    } else if (sensor_type_bits[2] && !sensor_type_bits[3]) {
      this->accuracy_text_sensor_->publish_state("have magnet field interference");
    } else {
      this->accuracy_text_sensor_->publish_state("ready");
    }
  }
#endif
#ifdef USE_SENSOR
  if (this->accel_x_sensor_ != nullptr) {
    uint16_t accel_x = (uint16_t) this->receive_buffer[5] | ((uint16_t) this->receive_buffer[6]) << 8;
    this->accel_x_sensor_->publish_state((float) accel_x * 0.00478515625);
  }
  if (this->accel_y_sensor_ != nullptr) {
    uint16_t accel_y = (uint16_t) this->receive_buffer[7] | ((uint16_t) this->receive_buffer[8]) << 8;
    this->accel_y_sensor_->publish_state((float) accel_y * 0.00478515625);
  }
  if (this->accel_z_sensor_ != nullptr) {
    uint16_t accel_z = (uint16_t) this->receive_buffer[9] | ((uint16_t) this->receive_buffer[10]) << 8;
    this->accel_z_sensor_->publish_state((float) accel_z * 0.00478515625);
  }
  if (this->gyro_x_sensor_ != nullptr) {
    uint16_t gyro_x = (uint16_t) this->receive_buffer[11] | ((uint16_t) this->receive_buffer[12]) << 8;
    this->gyro_x_sensor_->publish_state((float) gyro_x * 0.0625);
  }
  if (this->gyro_y_sensor_ != nullptr) {
    uint16_t gyro_y = (uint16_t) this->receive_buffer[13] | ((uint16_t) this->receive_buffer[14]) << 8;
    this->gyro_y_sensor_->publish_state((float) gyro_y * 0.0625);
  }
  if (this->gyro_z_sensor_ != nullptr) {
    uint16_t gyro_z = (uint16_t) this->receive_buffer[15] | ((uint16_t) this->receive_buffer[16]) << 8;
    this->gyro_z_sensor_->publish_state((float) gyro_z * 0.0625);
  }
  if (this->angle_x_sensor_ != nullptr) {
    uint16_t angle_x = (uint16_t) this->receive_buffer[17] | ((uint16_t) this->receive_buffer[18]) << 8;
    this->angle_x_sensor_->publish_state((float) angle_x * 0.0054931640625);
  }
  if (this->angle_y_sensor_ != nullptr) {
    uint16_t angle_y = (uint16_t) this->receive_buffer[19] | ((uint16_t) this->receive_buffer[20]) << 8;
    this->angle_y_sensor_->publish_state((float) angle_y * 0.0054931640625);
  }
  if (this->angle_z_sensor_ != nullptr) {
    uint16_t angle_z = (uint16_t) this->receive_buffer[21] | ((uint16_t) this->receive_buffer[22]) << 8;
    this->angle_z_sensor_->publish_state((float) angle_z * 0.0054931640625);
  }
  if (this->field_strength_x_sensor_ != nullptr) {
    uint16_t field_strength_x = (uint16_t) this->receive_buffer[23] | ((uint16_t) this->receive_buffer[24]) << 8;
    this->field_strength_x_sensor_->publish_state((float) field_strength_x * 0.006103515625);
  }
  if (this->field_strength_y_sensor_ != nullptr) {
    uint16_t field_strength_y = (uint16_t) this->receive_buffer[25] | ((uint16_t) this->receive_buffer[26]) << 8;
    this->field_strength_y_sensor_->publish_state((float) field_strength_y * 0.006103515625);
  }
  if (this->field_strength_z_sensor_ != nullptr) {
    uint16_t field_strength_z = (uint16_t) this->receive_buffer[27] | ((uint16_t) this->receive_buffer[28]) << 8;
    this->field_strength_z_sensor_->publish_state((float) field_strength_z * 0.006103515625);
  }

  if (this->q0_sensor_ != nullptr) {
    uint16_t q0 = (uint16_t) this->receive_buffer[29] | ((uint16_t) this->receive_buffer[30]) << 8;
    this->q0_sensor_->publish_state((float) q0 * 0.000030517578125);
  }
  if (this->q1_sensor_ != nullptr) {
    uint16_t q1 = (uint16_t) this->receive_buffer[31] | ((uint16_t) this->receive_buffer[32]) << 8;
    this->q1_sensor_->publish_state((float) q1 * 0.000030517578125);
  }
  if (this->q2_sensor_ != nullptr) {
    uint16_t q2 = (uint16_t) this->receive_buffer[33] | ((uint16_t) this->receive_buffer[34]) << 8;
    this->q2_sensor_->publish_state((float) q2 * 0.000030517578125);
  }
  if (this->q3_sensor_ != nullptr) {
    uint16_t q3 = (uint16_t) this->receive_buffer[35] | ((uint16_t) this->receive_buffer[36]) << 8;
    this->q3_sensor_->publish_state((float) q3 * 0.000030517578125);
  }

  if (this->temperature_sensor_ != nullptr) {
    int16_t temperature = (int16_t) this->receive_buffer[37] | ((int16_t) this->receive_buffer[38]) << 8;
    this->temperature_sensor_->publish_state((float) temperature * 0.01);
  }
  if (this->pressure_sensor_ != nullptr) {
    int32_t pressure = (int32_t) this->receive_buffer[39] | ((int32_t) this->receive_buffer[40]) << 8 |
                       ((int32_t) this->receive_buffer[41]) << 16 | ((int32_t) this->receive_buffer[42]) << 24;
    this->pressure_sensor_->publish_state((float) pressure * 0.0002384185791);
  }
  if (this->height_sensor_ != nullptr) {
    int32_t height = (int32_t) this->receive_buffer[43] | ((int32_t) this->receive_buffer[44]) << 8 |
                     ((int32_t) this->receive_buffer[45]) << 16 | ((int32_t) this->receive_buffer[46]) << 24;
    this->height_sensor_->publish_state((float) height * 0.0010728836);
  }
#endif
}

void AS201Component::send_command(uint8_t command, const uint8_t *data, uint16_t data_size) {
  this->write_array(AS201_CMD_HEAD, 2);
  this->write_byte((uint8_t) (data_size + 2));  // len = cmd + data + checksum
  this->write_byte(command);
  this->write_array(data, data_size);
  uint8_t checksum = command;
  for (uint8_t i = 0; i < data_size; i++) {
    checksum += data[i];
  }
  this->write_byte(checksum);
  this->write_array(AS201_CMD_TAIL, 2);
  this->flush();
}

void AS201Component::version() {
  uint8_t data = 0x01;
  this->send_command(0x10, &data, 1);
}

void AS201Component::factory_reset() {
  ESP_LOGD(TAG, "Factory Reset");
  uint8_t data = 0x01;
  this->send_command(0x11, &data, 1);
}

void AS201Component::restart() {
  ESP_LOGD(TAG, "Restarting AS201");
  uint8_t data = 0x01;
  this->send_command(0x17, &data, 1);
}

void AS201Component::reset_z_axis() {
  uint8_t data = 0x01;
  this->send_command(0x12, &data, 1);
}

void AS201Component::reset_euler_angle() {
  uint8_t data = 0x01;
  this->send_command(0x13, &data, 1);
}

void AS201Component::get_install_params() {
  uint8_t data = 0x01;
  this->send_command(0x14, &data, 1);
}

void AS201Component::set_install_params(const std::string &direction) {
  uint8_t data = direction == "vertical" ? 0x01 : 0x00;
  this->send_command(0x15, &data, 1);
}

void AS201Component::set_upload_rate(const std::string &rate) {
  uint8_t data;
  if (rate == "0.1Hz") {
    data = 0x01;
  } else if (rate == "0.5Hz") {
    data = 0x02;
  } else if (rate == "1Hz") {
    data = 0x03;
  } else if (rate == "2Hz") {
    data = 0x04;
  } else if (rate == "5Hz") {
    data = 0x05;
  } else if (rate == "10Hz") {
    data = 0x06;
  } else if (rate == "20Hz") {
    data = 0x07;
  } else {
    data = 0x07;
  }
  this->send_command(0x16, &data, 1);
}

void AS201Component::set_baud_rate(const std::string &baud_rate) {
  uint8_t data = 0x00;  // default 9600
  if (baud_rate == "4800") {
    data = 0x01;  // unsupported, set to 9600
  } else if (baud_rate == "9600") {
    data = 0x02;
  } else if (baud_rate == "19200") {
    data = 0x03;
  } else if (baud_rate == "38400") {
    data = 0x04;
  } else if (baud_rate == "57600") {
    data = 0x05;
  } else if (baud_rate == "115200") {
    data = 0x06;
  } else if (baud_rate == "230400") {
    data = 0x07;
  } else if (baud_rate == "460800") {
    data = 0x08;
  } else if (baud_rate == "500000") {
    data = 0x09;
  } else if (baud_rate == "921600") {
    data = 0x0A;
  }
  this->send_command(0x18, &data, 1);
  uint32_t new_baud_rate = stoi(baud_rate);
  if (this->parent_->get_baud_rate() != new_baud_rate) {
    this->parent_->set_baud_rate(new_baud_rate);
  }
  this->set_timeout(200, [this]() { this->restart(); });
}

void AS201Component::get_config() {
  uint8_t data = 0x01;
  this->send_command(0x19, &data, 1);
}

void AS201Component::enable_upload(bool enable) {
  uint8_t data = enable ? 0x01 : 0x00;
  this->send_command(0x1A, &data, 1);
}

void AS201Component::get_data_once() {
  uint8_t data = 0x01;
  this->send_command(0x1B, &data, 1);
}

void AS201Component::calibrate_accel() {
  uint8_t data = 0x01;
  this->send_command(0x1C, &data, 1);
}

void AS201Component::start_calibrate_magnetic_field() {
  uint8_t data = 0x01;
  this->send_command(0x1D, &data, 1);
}

void AS201Component::finish_calibrate_magnetic_field() {
  uint8_t data = 0x01;
  this->send_command(0x1E, &data, 1);
}

void AS201Component::set_subscribe_flag(bool accel, bool gyro, bool angle,
                                        bool field_strength, bool quaternion,
                                        bool temperature, bool pressure, bool height) {
  std::bitset<8> flag_bits = 0;
  flag_bits[0] = accel;
  flag_bits[1] = gyro;
  flag_bits[2] = angle;
  flag_bits[3] = field_strength;
  flag_bits[4] = quaternion;
  flag_bits[5] = temperature;
  flag_bits[6] = pressure;
  flag_bits[7] = height;
  uint8_t data = (uint8_t) flag_bits.to_ulong();
  this->send_command(0x1F, &data, 1);
}


}  // namespace as201
}  // namespace esphome