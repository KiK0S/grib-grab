#pragma once

#include "ecs/driver.hpp"

namespace engine::shrooms {

class MuseumLogic : public ecs::EcsLogic {
 public:
  MuseumLogic(int view_width, int view_height);

 protected:
  void on_init() override;
  void after_tick(const engine::AppContext&, std::span<const engine::InputEvent>,
                  engine::Frame&) override;

 private:
  int view_width_ = 0;
  int view_height_ = 0;
};

}  // namespace engine::shrooms
