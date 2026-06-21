#pragma once

#include "esphome/components/button/button.h"
#include "../rs200.h"

namespace esphome::rs200 {

// 触发 RS200 执行一次光学系统自校准 (帧标识 3, 写, 数据 0)
class CalibrateButton : public button::Button, public Parented<RS200Component> {
 public:
  CalibrateButton() = default;

 protected:
  void press_action() override;
};

}  // namespace esphome::rs200
