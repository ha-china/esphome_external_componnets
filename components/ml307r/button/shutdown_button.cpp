#include "shutdown_button.h"

namespace esphome::ml307r {

void ShutdownButton::press_action() { this->parent_->shutdown(this->status_); }

}  // namespace esphome::ml307r
