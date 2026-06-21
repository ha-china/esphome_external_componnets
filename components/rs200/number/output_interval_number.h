#pragma once

#include "esphome/components/number/number.h"
#include "../rs200.h"

namespace esphome::rs200 {

class OutputIntervalNumber : public number::Number, public Parented<RS200Component> {
  public:
    OutputIntervalNumber() = default;
    // void init() {
    //   this->parent_->task_queue_.emplace([this]() { this->read_and_publish(); });
    // }
    void read_and_publish();

  protected:
    void control(float value) override;
};

}  // namespace esphome::rs200