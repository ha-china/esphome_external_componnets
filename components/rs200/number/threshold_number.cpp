#include "threshold_number.h"

namespace esphome::rs200 {

void ThresholdNumber::read_and_publish() {
  // 发送读取该参数的命令, 由 RS200Component::loop 统一接收返回值并 publish_state
  this->parent_->send_command(false, this->tag_, 0);
}

void ThresholdNumber::control(float value) {
  this->parent_->send_command(true, this->tag_, static_cast<uint16_t>(value));
  this->read_and_publish();
}

}  // namespace esphome::rs200
