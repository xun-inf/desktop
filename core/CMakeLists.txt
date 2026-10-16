cmake_minimum_required(VERSION 3.25)

option(BIGOSTREAMER_SCHEMA_ONLY "Configure only the schema generation target" OFF)
if(BIGOSTREAMER_SCHEMA_ONLY)
  project(bigostreamer VERSION 1.0.0 LANGUAGES NONE)
else()
  project(bigostreamer VERSION 1.0.0 LANGUAGES C CXX)
endif()

get_filename_component(BIGOSTREAMER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FlatBuffers)
include(Spdlog)

if(BIGOSTREAMER_SCHEMA_ONLY)
  include(Schemas)
  return()
endif()

bigostreamer_add_flatbuffers_headers()

# Windows resource compile: .rc -> PE resource, used by media-service icon.
if(WIN32)
  enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

bigostreamer_add_spdlog()

# Applies only to this project own targets; does not affect FetchContent third-party projects.
add_library(bigostreamer_options INTERFACE)
target_compile_definitions(bigostreamer_options INTERFACE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
if (MSVC)
  target_compile_options(bigostreamer_options INTERFACE /utf-8 /W4)
endif()

add_subdirectory(utils)
add_subdirectory(media-service)
if(WIN32)
  # Integration fixtures (scripts/biz-tests, scripts/native-tests) were removed with the refactor;
  # keep the option for re-enabling them later, but do not default to ON.
  option(BIGOSTREAMER_BUILD_INTEGRATION_TESTS "Build native addon integration fixtures" OFF)
  add_subdirectory(node-bizmodules)
  add_subdirectory(node-media-service)
endif()

# Unified output directory: executables go to build/bin/<Config> at repo root
# (multi-config generators append the config name; single-config like Ninja use build/bin directly).
get_filename_component(BIGOSTREAMER_OUTPUT_DIR "${BIGOSTREAMER_ROOT}/build/bin" ABSOLUTE)

set_target_properties(mediaService PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${BIGOSTREAMER_OUTPUT_DIR}
  PDB_OUTPUT_DIRECTORY ${BIGOSTREAMER_OUTPUT_DIR})
