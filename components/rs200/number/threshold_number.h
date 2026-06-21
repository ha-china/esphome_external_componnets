#pragma once

#include "esphome/components/number/number.h"
#include "../rs200.h"

namespace esphome::rs200 {

// 通用的阈值参数 number, 绑定到一个固定的帧标识上
class ThresholdNumber : public number::Number, public Parented<RS200Component> {
  public:
    ThresholdNumber(uint8_t tag): tag_(tag) {}

    void read_and_publish();

  protected:
    void control(float value) override;

    uint8_t tag_{0};
};

}  // namespace esphome::rs200
