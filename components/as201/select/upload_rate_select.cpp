#include "upload_rate_select.h"

namespace esphome::as201 {

void UploadRateSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_upload_rate(this->current_option());
}

}  // namespace esphome::as201
