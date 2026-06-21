#include "output_interval_number.h"

namespace esphome::rs200 {

void OutputIntervalNumber::read_and_publish() {
  // 发送读取雨量状态输出频率的命令, 由 RS200Component::loop 统一接收返回值并 publish_state
  this->parent_->send_command(false, RS200_TAG_OUTPUT_FREQ, 0);
}

void OutputIntervalNumber::control(float value) {
  // 数值单位为 ms, 协议单位为 50ms, 0 表示禁用输出
  this->parent_->send_command(true, RS200_TAG_OUTPUT_FREQ, static_cast<uint16_t>(value) / 50);
  this->read_and_publish();
}

}  // namespace esphome::rs200