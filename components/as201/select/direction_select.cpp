#include "direction_select.h"

namespace esphome::as201 {

void DirectionSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_install_params(this->current_option());
  this->parent_->get_install_params();
}

}  // namespace esphome::as201
