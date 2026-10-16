include_guard(GLOBAL)

# Schema generation entry point: core/schemas/generate.py
# (merged from gen_protos.py / gen_services.py).
# Outputs (checked in, idempotently re-run):
#   core/common/protos/*.h                          C++ FlatBuffers protocol headers
#   core/common/protos/schema_traits.h              root-table verify/get traits (ServiceBase)
#   core/node-media-service/src/services/*.h/.cpp   same-name service proxies

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(_bigostreamer_schema_dir "${BIGOSTREAMER_ROOT}/core/schemas")
set(_bigostreamer_protos_dir "${BIGOSTREAMER_ROOT}/core/common/protos")
set(_bigostreamer_services_dir "${BIGOSTREAMER_ROOT}/core/node-media-service/src/services")
set(_bigostreamer_schemas
  "__BSSD__/service.fbs"
  "${_bigostreamer_schema_dir}/native_window_service.fbs"
  "${_bigostreamer_schema_dir}/stream_preview_service.fbs")

add_custom_target(generate-schemas
  COMMAND "${CMAKE_COMMAND}" -E make_directory
    "${_bigostreamer_protos_dir}"
    "${_bigostreamer_services_dir}"
  COMMAND "${Python3_EXECUTABLE}" "${BIGOSTREAMER_ROOT}/core/schemas/generate.py"
  DEPENDS ${_bigostreamer_schemas} "${BIGOSTREAMER_ROOT}/core/schemas/generate.py"
  WORKING_DIRECTORY "${BIGOSTREAMER_ROOT}"
  COMMENT "Generating FlatBuffers protocol headers and service proxies"
  VERBATIM)
