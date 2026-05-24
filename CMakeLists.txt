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
    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/main/web/shrooms_shell.html"
    "SHELL:--preload-file ${SHROOMS_ASSET_DIR}@/assets"
    "SHELL:--preload-file ${SHROOMS_SHADER_DIR}@/shaders"
    "SHELL:--preload-file ${SHROOMS_CUSTOM_SHADER_DIR}@/shaders"
  )
else()
  find_package(SDL2 REQUIRED)
  target_link_libraries(shrooms PRIVATE SDL2::SDL2)
  target_include_directories(shrooms PRIVATE ${SDL2_INCLUDE_DIRS})
endif()
