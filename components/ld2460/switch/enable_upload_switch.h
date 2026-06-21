#pragma once

#include "esphome/components/switch/switch.h"
#include "../ld2460.h"

namespace esphome::ld2460 {

class EnableUploadSwitch : public switch_::Switch, public Parented<LD2460Component> {
 public:
  EnableUploadSwitch() = default;

  void init() {
    // parent_ != nullptr 之后再调用，也就是cg.register_parented之后才能有
    this->parent_->task_queue_.emplace([this]() { this->turn_on(); });
  }

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::ld2460
