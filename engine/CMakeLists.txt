# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
set(VCPKG_FEATURE_FLAGS "versions")

message(STATUS "Begin ${CMAKE_BUILD_TYPE} Build")

project ("KhiinEngine" CXX)

set(PROTOS_DIR "${PROJECT_SOURCE_DIR}/../proto/proto")

find_package(GTest CONFIG REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${PROJECT_SOURCE_DIR} ${PROTOBUF_INCLUDE_DIR})

add_library(khiin STATIC
    "${PROTOS_DIR}/config.pb.h"
    "${PROTOS_DIR}/config.pb.cc"
    "${PROTOS_DIR}/command.pb.h"
    "${PROTOS_DIR}/command.pb.cc"
    "${PROTOS_DIR}/proto.h"
    "Engine.h"
    "Engine.cpp"
)

add_subdirectory("config")
add_subdirectory("data")
add_subdirectory("input")
add_subdirectory("utils")
target_include_directories(khiin PUBLIC "../third_party" "../proto")

# Debug
target_link_options(khiin PRIVATE "/MAP")
# target_include_directories (khiin PUBLIC "engine")

enable_testing()
add_subdirectory ("tests")

add_compile_options(/FAcs)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
