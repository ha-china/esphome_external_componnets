#pragma once

#include "esphome/components/button/button.h"
#include "../bax.h"

namespace esphome::bax {

class ZeroPointCalibrateButton : public button::Button, public Parented<BAXComponent> {
public:
  ZeroPointCalibrateButton() = default;

protected:
  void press_action() override;
};

}  // namespace esphome::bax
