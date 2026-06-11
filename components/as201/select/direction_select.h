#pragma once

#include "esphome/components/select/select.h"
#include "../as201.h"

namespace esphome::as201 {

class DirectionSelect : public select::Select, public Parented<AS201Component> {
 public:
  DirectionSelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace esphome::as201
