#include "calibrate_button.h"

namespace esphome::rs200 {

void CalibrateButton::press_action() {
  // 命令 RS200 执行光学系统校准, 执行后模块会通过帧标识 3 回送校准结果/校准值
  this->parent_->send_command(true, RS200_TAG_OPTICAL, 0);
  this->parent_->set_timeout("read_optical_calibration", 5000, [this]() {
    this->parent_->send_command(false, RS200_TAG_OPTICAL, 0);
  });
}

}  // namespace esphome::rs200
