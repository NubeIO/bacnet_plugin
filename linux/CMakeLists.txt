cmake_minimum_required(VERSION 3.10)

project(bacnet_plugin)

set(BACNET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../native/bacnet-stack")

include_directories(
    "${BACNET_DIR}/src"
    "${BACNET_DIR}/ports/linux"
)

# 1. Core Sources
file(GLOB BACNET_CORE_SOURCES "${BACNET_DIR}/src/bacnet/*.c")

# 2. Basic Services, Objects, Clients
file(GLOB_RECURSE BACNET_BASIC_SOURCES "${BACNET_DIR}/src/bacnet/basic/*.c")

# Exclude non-IPv4 / conflicting files (matching Windows CMakeLists)
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "basic/bbmd6/.*")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "ucix\.c$")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "device-client\.c$")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "bacnet_device\.c$")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "datetime_mstimer\.c$")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "sc_netport\.c$")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "bsc/.*")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "basic/object/gateway/.*")
list(FILTER BACNET_BASIC_SOURCES EXCLUDE REGEX "basic/npdu/h_routed_npdu\.c$")

# 3. Datalink (IP - V4 only)
set(BACNET_DATALINK_SOURCES
    "${BACNET_DIR}/src/bacnet/datalink/bvlc.c"
    "${BACNET_DIR}/src/bacnet/datalink/cobs.c"
    "${BACNET_DIR}/src/bacnet/datalink/datalink.c"
)

# 4. Port (Linux)
set(BACNET_PORT_SOURCES
    "${BACNET_DIR}/ports/linux/bip-init.c"
    "${BACNET_DIR}/ports/linux/datetime-init.c"
    "${BACNET_DIR}/ports/linux/mstimer-init.c"
)

add_library(bacnet_plugin SHARED
    ${BACNET_CORE_SOURCES}
    ${BACNET_BASIC_SOURCES}
    ${BACNET_DATALINK_SOURCES}
    ${BACNET_PORT_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/src/bacnet_plugin.c"
)

target_compile_definitions(bacnet_plugin PRIVATE
    BACDL_BIP
    BACNET_STACK_STATIC_DEFINE
    PRINT_ENABLED=0
    "exit=bacnet_plugin_exit_handler"
)

target_link_libraries(bacnet_plugin pthread)

set_target_properties(bacnet_plugin PROPERTIES
    OUTPUT_NAME "bacnet_plugin"
)

target_compile_definitions(bacnet_plugin PUBLIC FLUTTER_PLUGIN_IMPL)

# Required: export the .so to the Flutter app bundle
set(bacnet_plugin_bundled_libraries
    "$<TARGET_FILE:bacnet_plugin>"
    PARENT_SCOPE
)
