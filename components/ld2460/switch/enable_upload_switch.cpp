#include "enable_upload_switch.h"

namespace esphome::ld2460 {

void EnableUploadSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->enable_upload(state);
}

}  // namespace esphome::ld2460
