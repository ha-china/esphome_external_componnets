#pragma once

#include "esphome/components/switch/switch.h"
#include "../rs200.h"

namespace esphome::rs200 {

// 通用的开关, 绑定到一个固定的帧标识上, 数据 0=关闭 1=开启
class ModeSwitch : public switch_::Switch, public Parented<RS200Component> {
  public:
    ModeSwitch(uint8_t tag, bool default_on) : tag_(tag), default_on_(default_on) {}

    void init() {
      // parent_ != nullptr 之后再调用, 也就是 cg.register_parented 之后才能有
      if (this->default_on_) {
        this->parent_->task_queue_.emplace([this]() { this->turn_on(); });
      }
    }

  protected:
    void write_state(bool state) override;

    uint8_t tag_{0};
    bool default_on_{false};
};

}  // namespace esphome::rs200
