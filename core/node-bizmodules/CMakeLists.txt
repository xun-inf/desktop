cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(NodeBizModules LANGUAGES C CXX)
  get_filename_component(BIGOSTREAMER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
if(NOT WIN32)
  message(FATAL_ERROR "node-bizmodules currently requires a Windows Node-API host")
endif()

file(GLOB _biz_node_api_candidates "$ENV{LOCALAPPDATA}/node-gyp/Cache/*/include/node")
find_path(BIGOSTREAMER_NODE_API_INCLUDE node_api.h HINTS ${_biz_node_api_candidates})
if(NOT BIGOSTREAMER_NODE_API_INCLUDE)
  message(FATAL_ERROR "Set BIGOSTREAMER_NODE_API_INCLUDE to the Node.js include/node directory")
endif()

add_library(nodeBizModules MODULE
  src/addon.cpp
  src/bindings/napi_util.cpp
  src/runtime/module.cpp
  src/runtime/module_host.cpp
  src/runtime/module_registry.cpp
  src/modules/test/test_module.cpp)
target_include_directories(nodeBizModules PRIVATE "${BIGOSTREAMER_NODE_API_INCLUDE}")
target_compile_definitions(nodeBizModules PRIVATE NAPI_VERSION=8 NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_features(nodeBizModules PRIVATE cxx_std_11)
set_target_properties(nodeBizModules PROPERTIES
  CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
  PREFIX "" SUFFIX ".node" OUTPUT_NAME "node-bizmodules"
  LIBRARY_OUTPUT_DIRECTORY "${BIGOSTREAMER_ROOT}/build/bin"
  RUNTIME_OUTPUT_DIRECTORY "${BIGOSTREAMER_ROOT}/build/bin")
if(TARGET bigostreamer_options)
  target_link_libraries(nodeBizModules PRIVATE bigostreamer_options)
elseif(MSVC)
  target_compile_options(nodeBizModules PRIVATE /utf-8 /W4)
endif()
