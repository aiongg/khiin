# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(VCPKG_FEATURE_FLAGS "versions")

message(STATUS "Begin ${CMAKE_BUILD_TYPE} Build")

project ("KhiinEngine" CXX)

set(PROTOBUF_DIR "${PROJECT_SOURCE_DIR}/../proto/proto")

find_package(GTest CONFIG REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package (Boost REQUIRED COMPONENTS log)
include_directories (${Boost_INCLUDE_DIRS})

add_subdirectory ("engine")

target_include_directories(khiin PUBLIC "../third_party" "../proto")

enable_testing()
add_subdirectory ("tests")

add_compile_options(/FAcs)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
