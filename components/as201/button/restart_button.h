#pragma once

#include "esphome/components/button/button.h"
#include "../as201.h"

namespace esphome::as201 {

class RestartButton : public button::Button, public Parented<AS201Component> {
 public:
  RestartButton() = default;

 protected:
  void press_action() override;
};

}  // namespace esphome::as201
