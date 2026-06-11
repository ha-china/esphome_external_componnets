#pragma once

#include "esphome/components/switch/switch.h"
#include "../as201.h"

namespace esphome::as201 {

class EnableUploadSwitch : public switch_::Switch, public Parented<AS201Component> {
 public:
  EnableUploadSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::as201
