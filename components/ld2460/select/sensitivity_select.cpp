#include "sensitivity_select.h"

namespace esphome::ld2460 {

void SensitivitySelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_sensitivity(this->current_option());
  this->parent_->get_sensitivity();
}

}  // namespace esphome::ld2460
