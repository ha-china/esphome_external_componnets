#include "enable_upload_switch.h"

namespace esphome::as201 {

void EnableUploadSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->enable_upload(state);
}

}  // namespace esphome::as201
