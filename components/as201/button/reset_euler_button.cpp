#include "reset_euler_button.h"

namespace esphome::as201 {

void ResetEulerButton::press_action() { this->parent_->reset_euler_angle(); }

}  // namespace esphome::as201
