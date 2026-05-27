#include "engine/drivers.h"
#include "engine/input.h"
#include "engine/platform.h"
#include "shrooms_app.hpp"
#include "systems/render/renderable.hpp"
#include "world/visual_constants.hpp"

#ifdef __EMSCRIPTEN__
#include "engine/platform_emscripten.h"

#include <emscripten/emscripten.h>
#else
#include "engine/platform_sdl.h"
#endif

#ifdef __EMSCRIPTEN__
namespace {

engine::InputQueue* web_input_queue = nullptr;
engine::EmscriptenPlatform* web_platform = nullptr;
bool web_page_audio_active = true;
bool web_page_active = true;

void push_web_key_event(int key_code, bool pressed) {
  if (!web_input_queue) return;
  engine::InputEvent evt{};
  evt.kind = pressed ? engine::InputKind::KeyDown : engine::InputKind::KeyUp;
  evt.key_code = key_code;
  web_input_queue->push(evt);
}

}  // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE void shrooms_push_key_event(int key_code, int pressed) {
  push_web_key_event(key_code, pressed != 0);
}

EMSCRIPTEN_KEEPALIVE void shrooms_set_touchscreen_enabled(int enabled) {
  engine::shrooms::set_touchscreen_enabled(enabled != 0);
}

EMSCRIPTEN_KEEPALIVE void shrooms_set_mobile_layout(int enabled) {
  engine::shrooms::set_mobile_layout(enabled != 0);
}

EMSCRIPTEN_KEEPALIVE int shrooms_action_key_code(int action_index) {
  return engine::shrooms::action_key_code(action_index);
}

EMSCRIPTEN_KEEPALIVE void shrooms_request_audio_unlock() {
  if (!web_platform) return;
  web_platform->request_audio_unlock();
}

EMSCRIPTEN_KEEPALIVE void shrooms_set_page_audio_active(int active) {
  web_page_audio_active = active != 0;
  if (!web_platform) return;
  web_platform->set_audio_active(web_page_audio_active);
}

EMSCRIPTEN_KEEPALIVE void shrooms_set_page_active(int active) {
  web_page_active = active != 0;
  engine::shrooms::set_page_active(web_page_active);
}

EMSCRIPTEN_KEEPALIVE int shrooms_is_gameplay_active() {
  return engine::shrooms::is_gameplay_active() ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE int shrooms_is_shoot_enabled() {
  return engine::shrooms::is_shoot_enabled() ? 1 : 0;
}

}  // extern "C"
#endif

int main() {
  const int view_w = 900;
  const int view_h = 900;
  engine::PlatformConfig config{};
  config.width = view_w;
  config.height = view_h;
  config.title = "Grib Grab";
  config.renderer = engine::RendererKind::WebGL;

#ifdef __EMSCRIPTEN__
  engine::EmscriptenPlatform platform{};
#else
  engine::SdlPlatform platform{};
#endif
  engine::InputQueue input{};
#ifdef __EMSCRIPTEN__
  web_input_queue = &input;
  web_platform = &platform;
  web_platform->set_audio_active(web_page_audio_active);
#endif

  platform.init(config, input);
  auto renderer = platform.create_renderer(config);
  renderer->set_view_size(view_w, view_h);
  renderer->set_clear_color(engine::shrooms::kScreenClearColor);
  render_system::set_view_size(static_cast<float>(view_w), static_cast<float>(view_h));

  engine::shrooms::ShroomsLogic logic{view_w, view_h};
  engine::RealtimeDriver driver{logic, *renderer};
  logic.init();
#ifdef __EMSCRIPTEN__
  engine::shrooms::set_page_active(web_page_active);
#endif

  driver.run_main_loop(platform, input);
  return 0;
}
