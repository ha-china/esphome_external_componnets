#include "mode_select.h"

namespace esphome::ld2460 {

void ModeSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_mode(this->current_option());
  this->parent_->get_mode();
}

}  // namespace esphome::ld2460
