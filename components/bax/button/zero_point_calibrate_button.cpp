#include "zero_point_calibrate_button.h"

namespace esphome::bax {

void ZeroPointCalibrateButton::press_action() { this->parent_->zero_point_calibrate(); }

}  // namespace esphome::bax
