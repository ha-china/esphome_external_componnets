#include "reset_z_axis_button.h"

namespace esphome::as201 {

void ResetZAxisButton::press_action() { this->parent_->reset_z_axis(); }

}  // namespace esphome::as201
