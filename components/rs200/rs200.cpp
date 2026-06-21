#include "rs200.h"
#include "esphome/core/log.h"

namespace esphome::rs200 {
static const char *const TAG = "rs200";

uint8_t rs200_crc8(const uint8_t *dat, uint8_t size) {
  uint8_t crc = 0xFF, i;
  for (uint8_t b = 0; b < size; b++) {
    crc ^= (dat[b]);
    for (i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

void RS200Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RS200...");
  // 读取模块当前的所有可配置参数, 让 number 实体在启动时显示真实值
  this->task_queue_.emplace([this]() { this->read_all_parameters(); });
  // 按设定周期请求模块上报一次温度
  if (this->temperature_update_interval_ > 0) {
    this->set_interval("temperature", this->temperature_update_interval_,
                       [this]() { this->task_queue_.emplace([this]() {
                         this->send_command(false, RS200_TAG_TEMPERATURE, 0);
                       }); });
  }
}

void RS200Component::dump_config() {
  ESP_LOGCONFIG(TAG, "RS200:");
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "light", this->light_sensor_);
#endif
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "version", this->version_text_sensor_);
  LOG_TEXT_SENSOR("  ", "rain", this->rain_text_sensor_);
  LOG_TEXT_SENSOR("  ", "system_status", this->system_status_text_sensor_);
#endif
#ifdef USE_NUMBER
  LOG_NUMBER("  ", "output_interval", this->output_interval_number_);
  LOG_NUMBER("  ", "v1", this->v1_number_);
  LOG_NUMBER("  ", "v2", this->v2_number_);
  LOG_NUMBER("  ", "v3", this->v3_number_);
  LOG_NUMBER("  ", "s1", this->s1_number_);
  LOG_NUMBER("  ", "s2", this->s2_number_);
  LOG_NUMBER("  ", "s3", this->s3_number_);
  LOG_NUMBER("  ", "n1", this->n1_number_);
  LOG_NUMBER("  ", "n2", this->n2_number_);
  LOG_NUMBER("  ", "n3", this->n3_number_);
  LOG_NUMBER("  ", "optical_calibration", this->optical_calibration_number_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH("  ", "sleep_mode", this->sleep_mode_switch_);
  LOG_SWITCH("  ", "realtime_rain_mode", this->realtime_rain_mode_switch_);
  LOG_SWITCH("  ", "ambient_light_mode", this->ambient_light_mode_switch_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "calibrate_button", this->calibrate_button_);
#endif
  this->check_uart_settings(115200);
}

const char *RS200Component::rain_state_str_(uint16_t data) {
  switch (data) {
    case 0:
      return "no rain";
    case 1:
      return "small rain";
    case 2:
      return "medium rain";
    case 3:
      return "heavy rain";
    default:
      return "unknown";
  }
}

void RS200Component::handle_frame_(uint8_t frame_tag, uint16_t frame_data) {
  // 帧标识的 bit7 表示读写属性, 对于从 RS200 接收到的数据帧无意义, 这里只看低 7 位
  switch (frame_tag) {
    case RS200_TAG_VERSION: {
      // 帧数据[15:8]主版本号, [7:0]副本号
#ifdef USE_TEXT_SENSOR
      if (this->version_text_sensor_ != nullptr) {
        this->version_text_sensor_->publish_state(
            std::to_string(static_cast<uint8_t>((frame_data >> 8) & 0xFF)) + "." +
            std::to_string(static_cast<uint8_t>(frame_data & 0xFF)));
      }
#endif
      break;
    }
    case RS200_TAG_RAIN: {
      // 雨量状态: 0=无雨 1=小雨 2=中雨 3=大雨
#ifdef USE_TEXT_SENSOR
      if (this->rain_text_sensor_ != nullptr) {
        this->rain_text_sensor_->publish_state(this->rain_state_str_(frame_data));
      }
#endif
      break;
    }
    case RS200_TAG_SYSTEM_STATUS: {
      // 系统状态, 见用户手册 表 3.1 / 数据手册 表 2.8
      const char *status = "unknown";
      switch (frame_data) {
        case 0:
          ESP_LOGI(TAG, "System status: ok");
          status = "ok";
          break;
        case 1:
          status = "internal communication error";  // RS200 内部芯片间通信异常
          break;
        case 2:
          status = "LEDA damage";  // 读出的雨量值一直无效, 可能 LEDA 损坏
          break;
        case 3:
          status = "LEDB damage";  // 读出的雨量值一直为 0, 可能 LEDB 损坏
          break;
        case 4:
          status = "optical calibration poor";  // 校准后无水雨量值与中间值 1023 之差绝对值过大
          break;
        case 5:
          status = "parameter config failed";  // 向 Flash 写参数失败, 或输入参数超出规定范围
          break;
        case 6:
          status = "uart checksum error";  // RS200 收到的主机指令校验错误, 该指令不会被执行
          break;
        case 7:
          status = "low voltage warning";  // 模块电压低于或等于 2.8V
          break;
        default:
          break;
      }
      if (frame_data != 0) {
        ESP_LOGW(TAG, "System status: %s (%u)", status, frame_data);
      }
#ifdef USE_TEXT_SENSOR
      if (this->system_status_text_sensor_ != nullptr) {
        this->system_status_text_sensor_->publish_state(status);
      }
#endif
      break;
    }
    case RS200_TAG_OPTICAL: {
      // 光学系统校准值(读回执) / 校准执行结果回送, 均通过 tag3 推送
#ifdef USE_NUMBER
      if (this->optical_calibration_number_ != nullptr) {
        this->optical_calibration_number_->publish_state(frame_data);
      }
#endif
      break;
    }
    case RS200_TAG_REALTIME_RAIN: {
      // RS200向主机发送原始雨量数值
#ifdef USE_SENSOR
      if (this->raw_rain_sensor_ != nullptr) {
        this->raw_rain_sensor_->publish_state(frame_data);
      }
#endif
      break;
    }
    case RS200_TAG_OUTPUT_FREQ: {
      // 雨量状态输出频率: 每单位代表 50ms, 0 表示禁用输出
#ifdef USE_NUMBER
      if (this->output_interval_number_ != nullptr) {
        this->output_interval_number_->publish_state(frame_data * 50);
      }
#endif
      break;
    }
    case RS200_TAG_V1:
    case RS200_TAG_V2:
    case RS200_TAG_V3:
    case RS200_TAG_S1:
    case RS200_TAG_S2:
    case RS200_TAG_S3:
    case RS200_TAG_N1:
    case RS200_TAG_N2:
    case RS200_TAG_N3: {
      // 阈值参数回执
#ifdef USE_NUMBER
      number::Number *num = nullptr;
      switch (frame_tag) {
        case RS200_TAG_V1:
          num = this->v1_number_;
          break;
        case RS200_TAG_V2:
          num = this->v2_number_;
          break;
        case RS200_TAG_V3:
          num = this->v3_number_;
          break;
        case RS200_TAG_S1:
          num = this->s1_number_;
          break;
        case RS200_TAG_S2:
          num = this->s2_number_;
          break;
        case RS200_TAG_S3:
          num = this->s3_number_;
          break;
        case RS200_TAG_N1:
          num = this->n1_number_;
          break;
        case RS200_TAG_N2:
          num = this->n2_number_;
          break;
        case RS200_TAG_N3:
          num = this->n3_number_;
          break;
        default:
          break;
      }
      if (num != nullptr) {
        num->publish_state(frame_data);
      }
#endif
      break;
    }
    case RS200_TAG_LIGHT: {
      // 环境光反馈值, 范围 0~1024, 数值越小光越强
#ifdef USE_SENSOR
      if (this->light_sensor_ != nullptr) {
        this->light_sensor_->publish_state(frame_data);
      }
#endif
      break;
    }
    case RS200_TAG_TEMPERATURE: {
      // 模块温度(片内温度传感器), 线性关系, 数值即为温度℃
#ifdef USE_SENSOR
      if (this->temperature_sensor_ != nullptr) {
        this->temperature_sensor_->publish_state((static_cast<float>(frame_data) - 605.36) / (-1.5596)); // yhsc(用户手册)图1.2  温度测量线性分析
      }
#endif
      break;
    }
    case RS200_TAG_BOOT: {
      // RS200 上电/退出睡眠时主动发送的 Boot 启动指令(data=0x0002), 后续会跟随固件版本帧
      ESP_LOGI(TAG, "RS200 booted");
      break;
    }
    default:
      ESP_LOGW(TAG, "Unknown frame tag: %u, data: %u", frame_tag, frame_data);
      break;
  }
}

void RS200Component::loop() {
  // 注意: 全部 this->read()/this->available() 只能在这个方法里发生, 保持串口数据完整性
  // 先执行排队的任务(发送指令), 保证收发有序
  if (!this->task_queue_.empty()) {
    auto task = std::move(this->task_queue_.front());
    if (task) {
      task();
    }
    this->task_queue_.pop();
  }

  // 把串口收到的字节全部读入缓冲
  while (this->available()) {
    this->buffer_.push_back(static_cast<uint8_t>(this->read()));
  }

  // 循环解析缓冲里所有完整帧
  while (this->buffer_.size() >= 5) {
    // 定位帧头 0x3A
    size_t start = 0;
    while (start < this->buffer_.size() && this->buffer_[start] != 0x3A) {
      start++;
    }
    if (start > 0) {
      // 丢弃帧头之前的无效字节
      this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + start);
    }
    if (this->buffer_.size() < 5) {
      return;  // 还没收满一帧
    }
    // CRC 校验: 帧标识 + 帧数据(2 字节)
    uint8_t crc = rs200_crc8(this->buffer_.data() + 1, 3);
    if (this->buffer_[4] != crc) {
      ESP_LOGW(TAG, "CRC mismatch, discarding one byte. data: %02X %02X %02X %02X, crc: %02X (calc %02X)",
               this->buffer_[0], this->buffer_[1], this->buffer_[2], this->buffer_[3], this->buffer_[4], crc);
      // 只丢弃帧头字节, 让下一轮重新对齐帧头, 避免丢掉后续有效数据
      this->buffer_.erase(this->buffer_.begin());
      continue;
    }
    uint8_t frame_tag = this->buffer_[1] & 0x7F;
    uint16_t frame_data = ((uint16_t) this->buffer_[2]) | (((uint16_t) this->buffer_[3]) << 8);
    this->handle_frame_(frame_tag, frame_data);
    // 处理完后, 移除这一帧(5 字节)
    this->buffer_.erase(this->buffer_.begin(), this->buffer_.begin() + 5);
  }
}

void RS200Component::read_all_parameters() {
  this->send_command(false, RS200_TAG_VERSION, 0);
  this->send_command(false, RS200_TAG_OPTICAL, 0);  // 读取光学系统校准值
  this->send_command(false, RS200_TAG_OUTPUT_FREQ, 0);
  this->send_command(false, RS200_TAG_V1, 0);
  this->send_command(false, RS200_TAG_V2, 0);
  this->send_command(false, RS200_TAG_V3, 0);
  this->send_command(false, RS200_TAG_S1, 0);
  this->send_command(false, RS200_TAG_S2, 0);
  this->send_command(false, RS200_TAG_S3, 0);
  this->send_command(false, RS200_TAG_N1, 0);
  this->send_command(false, RS200_TAG_N2, 0);
  this->send_command(false, RS200_TAG_N3, 0);
}

void RS200Component::send_command(bool write, uint8_t tag, uint16_t data) {
  std::vector<uint8_t> frame(5);
  frame[0] = 0x3A;
  frame[1] = (write ? 0x80 : 0x00) | (tag & 0x7F);
  frame[2] = (data & 0xFF);
  frame[3] = (data >> 8) & 0xFF;
  frame[4] = rs200_crc8(frame.data() + 1, 3);
  this->write_array(frame.data(), frame.size());
}

}  // namespace esphome::rs200
