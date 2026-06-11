#pragma once

#include "esphome/components/number/number.h"
#include "../ld2460.h"

namespace esphome::ld2460 {

class DetectDistanceNumber : public number::Number, public Parented<LD2460Component> {
 public:
  DetectDistanceNumber() = default;

 protected:
  void control(float value) override;
};

class DetectStartAngleNumber : public number::Number, public Parented<LD2460Component> {
 public:
  DetectStartAngleNumber() = default;

 protected:
  void control(float value) override;
};

class DetectEndAngleNumber: public number::Number, public Parented<LD2460Component> {
 public:
  DetectEndAngleNumber() = default;

 protected:
  void control(float value) override;
};


}  // namespace esphome::ld2460