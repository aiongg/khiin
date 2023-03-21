# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(VCPKG_FEATURE_FLAGS "versions")

if(UNIX)
link_libraries(stdc++fs)
endif()

message(STATUS "Begin ${CMAKE_BUILD_TYPE} Build")

project ("KhiinEngine" CXX)

set(PROTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/proto")

if(WIN32)
    find_package(GTest CONFIG REQUIRED)
endif()
find_package(Protobuf REQUIRED)
find_package(SQLiteCpp REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Protobuf_INCLUDE_DIR})

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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_options(khiin PRIVATE "/MAP")
endif ()

if(WIN32)
  enable_testing()
  add_subdirectory ("tests")
  add_compile_options(/FAcs)
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# Add Analyze with CppCheck target if CppCheck is installed
if(WIN32)
  # Find CppCheck executable
  find_program(CMAKE_CXX_CPPCHECK cppcheck NAMES cppcheck HINTS $ENV{PROGRAMFILES}/cppcheck)
  
  # If CppCheck executable found
  if(CMAKE_CXX_CPPCHECK)
    # Check CppCheck version
    set(CPP_CHECK_CMD ${CMAKE_CXX_CPPCHECK} --version)
    execute_process(COMMAND ${CPP_CHECK_CMD}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE CPP_CHECK_RESULT
      OUTPUT_VARIABLE CPP_CHECK_VERSION
      ERROR_VARIABLE CPP_CHECK_ERROR)
    
    # Check if version could be extracted
    if(CPP_CHECK_RESULT EQUAL 0)
      # Get number of CPU cores
      include(ProcessorCount)
      ProcessorCount(CPU_CORES)

      # Append desired arguments to CppCheck
      list(
        APPEND CMAKE_CXX_CPPCHECK
          
          # Using the below template will allow jumping to any found error from inside Visual Studio output window by double click
          "--template \"${CMAKE_SOURCE_DIR}/{file}({line}): {severity} ({id}): {message}\"" 
          
          # Use all the available CPU cores
          "-j ${CPU_CORES}" 
          
          # Only show found errors
          "--quiet" 
          
          # Desired warning level in CppCheck
          "--enable=style"
          
          # Optional: Specified C++ version
          "--std=c++17"
           
          # Optional: Specified platform
          "--platform=win64"
          
          # Optional: suppression file stored in same directory as the top level CMake script
          # "--suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck_suppressions.txt"
          
          # Optional: Use inline suppressions
          "--inline-suppr"
          
          # Run CppCheck from the working directory, as specified in the add_custom_target command below
          "."
        )
      
      add_custom_target(ANALYZE_CPPCHECK DEPENDS khiin
        COMMAND ${CMAKE_CXX_CPPCHECK}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Static code analysis using ${CPP_CHECK_VERSION}"
      )
    endif()
  endif()
endif()