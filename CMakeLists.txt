cmake_minimum_required(VERSION 3.20)

project(shrooms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHROOMS_ASAN "Enable AddressSanitizer" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(SHROOMS_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(ENGINE_PLATFORM "web")
  message(STATUS "Configuring for Web (Emscripten)")
else()
  set(ENGINE_PLATFORM "native")
  message(STATUS "Configuring for native platform")
endif()

include(FetchContent)

FetchContent_Declare(
  stb_image
  URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
  DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})

# Engine headers include both glm/... and glm/glm/... forms.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/glm)

add_subdirectory(3rd-party/engine/libs/core)
add_subdirectory(3rd-party/engine/libs/ui)
add_subdirectory(3rd-party/engine/libs/render)
add_subdirectory(3rd-party/engine/libs/platform)

if(ENGINE_PLATFORM STREQUAL "web")
  # Third-party engine currently adds both platform backends to one target.
  # For web builds we still use Emscripten's SDL2 port, but must drop the native SDL backend.
  get_target_property(engine_platform_sources engine_platform SOURCES)
  list(FILTER engine_platform_sources EXCLUDE REGEX "platform_sdl\\.cpp$")
  set_property(TARGET engine_platform PROPERTY SOURCES "${engine_platform_sources}")
endif()

add_executable(shrooms
  src/main/main.cpp
  src/main/shrooms_app.cpp
)

target_link_libraries(shrooms
  PRIVATE
    engine_core
    engine_render
    engine_webgl
    engine_platform
)

target_include_directories(shrooms PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/libs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main
)

target_compile_features(shrooms PRIVATE cxx_std_20)

set(SHROOMS_ENGINE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/assets")
set(SHROOMS_PROJECT_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(SHROOMS_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/shaders")
set(SHROOMS_CUSTOM_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/main/custom_shaders")

if(EXISTS "${SHROOMS_PROJECT_ASSET_DIR}")
  set(SHROOMS_ASSET_DIR "${SHROOMS_PROJECT_ASSET_DIR}")
else()
  set(SHROOMS_ASSET_DIR "${SHROOMS_ENGINE_ASSET_DIR}")
endif()

if(NOT EXISTS "${SHROOMS_ASSET_DIR}")
  message(FATAL_ERROR "Missing assets directory: ${SHROOMS_ASSET_DIR}")
endif()

set(SHROOMS_WEB_PRELOAD_FILES)

macro(shrooms_add_web_preload source_path virtual_path)
  if(NOT EXISTS "${source_path}")
    message(FATAL_ERROR "Missing web preload file: ${source_path}")
  endif()
  list(APPEND SHROOMS_WEB_PRELOAD_FILES "SHELL:--preload-file ${source_path}@${virtual_path}")
endmacro()

macro(shrooms_add_web_asset relative_path)
  shrooms_add_web_preload("${SHROOMS_ASSET_DIR}/${relative_path}" "/assets/${relative_path}")
endmacro()

macro(shrooms_add_web_shader filename)
  if(EXISTS "${SHROOMS_CUSTOM_SHADER_DIR}/${filename}")
    set(_shrooms_shader_source "${SHROOMS_CUSTOM_SHADER_DIR}/${filename}")
  elseif(EXISTS "${SHROOMS_SHADER_DIR}/${filename}")
    set(_shrooms_shader_source "${SHROOMS_SHADER_DIR}/${filename}")
  else()
    message(FATAL_ERROR "Missing web shader preload file: ${filename}")
  endif()
  shrooms_add_web_preload("${_shrooms_shader_source}" "/shaders/${filename}")
endmacro()

# Runtime preload manifest. License/credits files stay in source control but
# are intentionally left out of the downloaded web package.
set(SHROOMS_WEB_ASSET_FILES
  Vera.ttf
  levels.data
  mushrooms.data
  shrooms/audio/bgm/69_forest_night.wav
  shrooms/audio/sfx/familiar_return.wav
  shrooms/audio/sfx/familiar_start.wav
  shrooms/audio/sfx/mushroom_catch1.wav
  shrooms/audio/sfx/mushroom_catch2.wav
  shrooms/audio/sfx/mushroom_fall.wav
  shrooms/audio/sfx/mushroom_shot.wav
  shrooms/background.svg
  shrooms/bat_face.svg
  shrooms/borovik.svg
  shrooms/borovik_small.svg
  shrooms/bottom_1.svg
  shrooms/bottom_2.svg
  shrooms/config.toml
  shrooms/digits_1.svg
  shrooms/digits_2.svg
  shrooms/digits_3.svg
  shrooms/emoji_crown.svg
  shrooms/emoji_fly.svg
  shrooms/emoji_frog.svg
  shrooms/emoji_hedgehog.svg
  shrooms/emoji_house.svg
  shrooms/emoji_infinity.svg
  shrooms/emoji_lock.svg
  shrooms/emoji_strawberry.svg
  shrooms/emoji_tree.svg
  shrooms/explosion.png
  shrooms/face_mini_1.svg
  shrooms/face_mini_2.svg
  shrooms/familiar_attack.svg
  shrooms/famiriar.svg
  shrooms/heart.svg
  shrooms/icon.svg
  shrooms/in_game_pause.svg
  shrooms/level_1_ezh.svg
  shrooms/level_2_eli.svg
  shrooms/level_3_izba.svg
  shrooms/level_4_lyaguha.svg
  shrooms/level_5_mol.svg
  shrooms/level_6_tzar.svg
  shrooms/level_7_yagoda.svg
  shrooms/lisi4ka.svg
  shrooms/lisi4ka_small.svg
  shrooms/menu_face.svg
  shrooms/menu_pause.svg
  shrooms/menu_scoreboard.svg
  shrooms/mukhomor.svg
  shrooms/mukhomor_small.svg
  shrooms/tilemap.png
  shrooms/witch.svg
  shrooms/witch_left_1.svg
  shrooms/witch_left_2.svg
  shrooms/witch_right_1.svg
  shrooms/witch_right_2.svg
)

foreach(_shrooms_asset IN LISTS SHROOMS_WEB_ASSET_FILES)
  shrooms_add_web_asset("${_shrooms_asset}")
endforeach()

set(SHROOMS_WEB_SHADER_FILES
  alpha_mask_2d_f.glsl
  alpha_mask_2d_v.glsl
  glow_blur_2d_f.glsl
  glow_blur_2d_v.glsl
  glow_divide_2d_f.glsl
  glow_divide_2d_v.glsl
  glow_mask_2d_f.glsl
  glow_mask_2d_v.glsl
  implicit_alpha_mask_2d_f.glsl
  implicit_alpha_mask_2d_v.glsl
  implicit_warp_sprite_2d_f.glsl
  implicit_warp_sprite_2d_v.glsl
  implicit_warp_sprite_gray_2d_f.glsl
  implicit_warp_sprite_gray_2d_v.glsl
  mesh_color_3d_f.glsl
  mesh_color_3d_v.glsl
  mesh_texture_3d_f.glsl
  mesh_texture_3d_v.glsl
  shrooms_composite_2d_f.glsl
  shrooms_composite_2d_v.glsl
  texture_alpha_tint_2d_f.glsl
  texture_alpha_tint_2d_v.glsl
  ui_default_2d_f.glsl
  ui_default_2d_v.glsl
  ui_texture_2d_f.glsl
  ui_texture_2d_v.glsl
)

foreach(_shrooms_shader IN LISTS SHROOMS_WEB_SHADER_FILES)
  shrooms_add_web_shader("${_shrooms_shader}")
endforeach()

if(ENGINE_PLATFORM STREQUAL "web")
  add_custom_command(TARGET shrooms POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
    $<TARGET_FILE_DIR:shrooms>/assets
    COMMAND ${CMAKE_COMMAND} -E rm -rf
    $<TARGET_FILE_DIR:shrooms>/shaders
  )
else()
  add_custom_command(TARGET shrooms POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
    $<TARGET_FILE_DIR:shrooms>/assets
    COMMAND ${CMAKE_COMMAND} -E rm -rf
    $<TARGET_FILE_DIR:shrooms>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHROOMS_ASSET_DIR}
    $<TARGET_FILE_DIR:shrooms>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHROOMS_SHADER_DIR}
    $<TARGET_FILE_DIR:shrooms>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHROOMS_CUSTOM_SHADER_DIR}
    $<TARGET_FILE_DIR:shrooms>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHROOMS_SHADER_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHROOMS_CUSTOM_SHADER_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  )
endif()

if(ENGINE_PLATFORM STREQUAL "web")
  set_target_properties(shrooms PROPERTIES SUFFIX ".html")
  set_property(TARGET shrooms APPEND PROPERTY LINK_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/web/shrooms_shell.html
  )
  target_link_options(shrooms PRIVATE
    "-sWASM=1"
    # The engine decodes assets from the preloaded filesystem itself.
    # Emscripten preload plugins add browser-side image/audio decode
    # dependencies before main(), which can stall startup on WAV files.
    "-sEXPORTED_RUNTIME_METHODS=['FS']"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sINITIAL_MEMORY=134217728"
    "-sMAXIMUM_MEMORY=1073741824"
    "-sMIN_WEBGL_VERSION=2"
    "-sMAX_WEBGL_VERSION=2"
    "-sUSE_WEBGL2=1"
    "-sFULL_ES3=1"
    "-sUSE_SDL=2"
    "SHELL:--use-preload-cache"
    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/main/web/shrooms_shell.html"
    ${SHROOMS_WEB_PRELOAD_FILES}
  )
else()
  find_package(SDL2 REQUIRED)
  target_link_libraries(shrooms PRIVATE SDL2::SDL2)
  target_include_directories(shrooms PRIVATE ${SDL2_INCLUDE_DIRS})
endif()
