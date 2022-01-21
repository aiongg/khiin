# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project ("KhiinEngine" CXX)

# find_package(ICU)
# Boost v1.75.0
find_package (Boost REQUIRED COMPONENTS log unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

find_package(SQLiteCpp)

add_subdirectory ("khiin_engine")

target_include_directories(khiin_engine PUBLIC "../third_party")

enable_testing()
add_subdirectory ("tests")

add_compile_options(/FAcs)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
