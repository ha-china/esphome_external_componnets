#include "restart_button.h"

namespace esphome::as201 {

void RestartButton::press_action() { this->parent_->restart(); }

}  // namespace esphome::as201
