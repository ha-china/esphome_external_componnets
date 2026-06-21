#include "mode_switch.h"

namespace esphome::rs200 {

void ModeSwitch::write_state(bool state) {
  this->parent_->send_command(true, this->tag_, state ? 1 : 0);
  this->publish_state(state);
}

}  // namespace esphome::rs200
