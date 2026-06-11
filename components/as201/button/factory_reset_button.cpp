#include "factory_reset_button.h"

namespace esphome::as201 {

void FactoryResetButton::press_action() { this->parent_->factory_reset(); }

}  // namespace esphome::as201
