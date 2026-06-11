#include "reboot_button.h"

namespace esphome::ml307r {

void RebootButton::press_action() { this->parent_->shutdown(this->status_); }

}  // namespace esphome::ml307r
