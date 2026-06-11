#include "baud_rate_select.h"

namespace esphome::ld2460 {

void BaudRateSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_baud_rate(this->current_option());
}

}  // namespace esphome::ld2460
