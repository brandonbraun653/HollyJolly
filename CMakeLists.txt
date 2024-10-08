cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_COMPILER_WORKS)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Import Pico SDK
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk")
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(HollyJolly C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_definitions(
  -D__PROGRAM_START # Required to prevent cmsis_gcc.h from complaining about unused definitions
)
add_compile_options(-Wall
  -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
  -Wno-unused-function # we have some for the docs that aren't called
  -Wno-maybe-uninitialized
  )

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

# Import the PicoDebug library
set(CMSIS_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/cmsis_5)
add_subdirectory(lib/pico-debug)

# Add main Folder
add_subdirectory(src)

