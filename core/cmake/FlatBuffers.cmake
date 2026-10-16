include_guard(GLOBAL)

include(FetchContent)

set(BIGOSTREAMER_FLATBUFFERS_VERSION "25.9.23" CACHE STRING
  "FlatBuffers version shared by C++, flatc and Electron")
set(BIGOSTREAMER_FLATBUFFERS_ROOT "" CACHE PATH
  "Optional FlatBuffers source root containing include/flatbuffers")
set(BIGOSTREAMER_FLATC "" CACHE FILEPATH
  "Optional flatc executable; otherwise CMake downloads the pinned Windows binary")

set(_BIGOSTREAMER_FLATBUFFERS_SOURCE_SHA256
  "9102253214dea6ae10c2ac966ea1ed2155d22202390b532d1dea64935c518ada")
set(_BIGOSTREAMER_FLATC_WINDOWS_SHA256
  "3d6383193ecd274f5de544a6e03464a87f581befb9fc1dda9cf508fa3cce3127")

file(READ "${BIGOSTREAMER_ROOT}/electron/package.json" _bigostreamer_package_json)
string(JSON _bigostreamer_npm_flatbuffers_version
  GET "${_bigostreamer_package_json}" dependencies flatbuffers)
if(NOT _bigostreamer_npm_flatbuffers_version STREQUAL BIGOSTREAMER_FLATBUFFERS_VERSION)
  message(FATAL_ERROR
    "FlatBuffers version mismatch: CMake=${BIGOSTREAMER_FLATBUFFERS_VERSION}, "
    "electron/package.json=${_bigostreamer_npm_flatbuffers_version}")
endif()

function(bigostreamer_add_flatbuffers_headers)
  if(TARGET FlatBuffers::Headers)
    return()
  endif()

  if(BIGOSTREAMER_FLATBUFFERS_ROOT)
    get_filename_component(_flatbuffers_root
      "${BIGOSTREAMER_FLATBUFFERS_ROOT}" ABSOLUTE BASE_DIR "${BIGOSTREAMER_ROOT}")
  else()
    FetchContent_Declare(bigostreamer_fb
      URL
        "https://github.com/google/flatbuffers/archive/refs/tags/v${BIGOSTREAMER_FLATBUFFERS_VERSION}.tar.gz"
      URL_HASH "SHA256=${_BIGOSTREAMER_FLATBUFFERS_SOURCE_SHA256}"
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      SOURCE_SUBDIR _bigostreamer_no_cmake_project)
    FetchContent_MakeAvailable(bigostreamer_fb)
    set(_flatbuffers_root "${bigostreamer_fb_SOURCE_DIR}")
  endif()

  set(_flatbuffers_header "${_flatbuffers_root}/include/flatbuffers/flatbuffers.h")
  if(NOT EXISTS "${_flatbuffers_header}")
    message(FATAL_ERROR
      "FlatBuffers headers not found: ${_flatbuffers_header}. "
      "Set BIGOSTREAMER_FLATBUFFERS_ROOT to a matching source tree if building offline.")
  endif()

  set(_flatbuffers_base_header "${_flatbuffers_root}/include/flatbuffers/base.h")
  file(READ "${_flatbuffers_base_header}" _flatbuffers_base)
  string(REPLACE "." ";" _flatbuffers_version_parts
    "${BIGOSTREAMER_FLATBUFFERS_VERSION}")
  list(GET _flatbuffers_version_parts 0 _flatbuffers_version_major)
  list(GET _flatbuffers_version_parts 1 _flatbuffers_version_minor)
  list(GET _flatbuffers_version_parts 2 _flatbuffers_version_revision)
  foreach(_version_part MAJOR MINOR REVISION)
    string(TOLOWER "${_version_part}" _version_part_lower)
    string(FIND "${_flatbuffers_base}"
      "#define FLATBUFFERS_VERSION_${_version_part} ${_flatbuffers_version_${_version_part_lower}}"
      _version_match)
    if(_version_match EQUAL -1)
      message(FATAL_ERROR
        "FlatBuffers headers in ${_flatbuffers_root} do not match "
        "version ${BIGOSTREAMER_FLATBUFFERS_VERSION}")
    endif()
  endforeach()

  add_library(bigostreamer_flatbuffers_headers INTERFACE)
  add_library(FlatBuffers::Headers ALIAS bigostreamer_flatbuffers_headers)
  target_include_directories(bigostreamer_flatbuffers_headers SYSTEM INTERFACE
    "${_flatbuffers_root}/include")
endfunction()

function(bigostreamer_add_flatc)
  if(TARGET FlatBuffers::flatc)
    return()
  endif()

  if(BIGOSTREAMER_FLATC)
    get_filename_component(_flatc
      "${BIGOSTREAMER_FLATC}" ABSOLUTE BASE_DIR "${BIGOSTREAMER_ROOT}")
  elseif(WIN32)
    FetchContent_Declare(bigostreamer_flatc
      URL
        "https://github.com/google/flatbuffers/releases/download/v${BIGOSTREAMER_FLATBUFFERS_VERSION}/Windows.flatc.binary.zip"
      URL_HASH "SHA256=${_BIGOSTREAMER_FLATC_WINDOWS_SHA256}"
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      SOURCE_SUBDIR _bigostreamer_no_cmake_project)
    FetchContent_MakeAvailable(bigostreamer_flatc)
    set(_flatc "${bigostreamer_flatc_SOURCE_DIR}/flatc.exe")
  else()
    find_program(_flatc NAMES flatc)
  endif()

  if(NOT _flatc OR NOT EXISTS "${_flatc}")
    message(FATAL_ERROR
      "flatc not found. Set BIGOSTREAMER_FLATC to a ${BIGOSTREAMER_FLATBUFFERS_VERSION} executable.")
  endif()

  execute_process(
    COMMAND "${_flatc}" --version
    RESULT_VARIABLE _flatc_version_result
    OUTPUT_VARIABLE _flatc_version_output
    ERROR_VARIABLE _flatc_version_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  string(FIND "${_flatc_version_output}${_flatc_version_error}"
    "${BIGOSTREAMER_FLATBUFFERS_VERSION}" _flatc_version_match)
  if(NOT _flatc_version_result EQUAL 0 OR _flatc_version_match EQUAL -1)
    message(FATAL_ERROR
      "flatc ${_flatc} does not report version ${BIGOSTREAMER_FLATBUFFERS_VERSION}")
  endif()

  add_executable(bigostreamer_flatc IMPORTED GLOBAL)
  set_target_properties(bigostreamer_flatc PROPERTIES IMPORTED_LOCATION "${_flatc}")
  add_executable(FlatBuffers::flatc ALIAS bigostreamer_flatc)
endfunction()
