#include "engine/drivers.h"
#include "engine/input.h"
#include "engine/platform.h"
#include "engine/platform_sdl.h"
#include "shrooms_museum.hpp"
#include "systems/render/renderable.hpp"
#include "world/visual_constants.hpp"

#include <filesystem>
#include <system_error>

namespace {

void use_executable_directory_as_cwd() {
  std::error_code ec;
  const auto exe_path = std::filesystem::canonical("/proc/self/exe", ec);
  if (ec) return;
  std::filesystem::current_path(exe_path.parent_path(), ec);
}

}  // namespace

int main() {
  use_executable_directory_as_cwd();

  const int view_w = 1200;
  const int view_h = 900;

  engine::PlatformConfig config{};
  config.width = view_w;
  config.height = view_h;
  config.title = "Shrooms Art Museum";
  config.renderer = engine::RendererKind::WebGL;
  config.asset_roots = {
      "/assets/shrooms",
      "assets/shrooms",
      "../assets/shrooms",
      "/assets",
      "assets",
      "../assets",
  };

  engine::SdlPlatform platform{};
  engine::InputQueue input{};

  platform.init(config, input);
  auto renderer = platform.create_renderer(config);
  renderer->set_view_size(view_w, view_h);
  renderer->set_clear_color(engine::shrooms::kScreenClearColor);
  render_system::set_view_size(static_cast<float>(view_w), static_cast<float>(view_h));

  engine::shrooms::MuseumLogic logic{view_w, view_h};
  engine::RealtimeDriver driver{logic, *renderer};
  logic.init();
  driver.run_main_loop(platform, input);
  return 0;
}
