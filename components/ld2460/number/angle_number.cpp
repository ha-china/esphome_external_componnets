#include "angle_number.h"

namespace esphome::ld2460 {

void AngleNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_install_params(
      (this->parent_->height_number_ != nullptr && this->parent_->height_number_->has_state())
          ? this->parent_->height_number_->state
          : this->parent_->height_,
      value);
  this->parent_->get_install_params();  // todo read after change
  this->parent_->save_to_flash();
}

}  // namespace esphome::ld2460
