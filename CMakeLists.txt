cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(QuickJS)

if(VITA)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()

  set(BUILD_PREFIX "$ENV{VITASDK}/arm-vita-eabi")

  set(VITA_GL_LIB vitaGL CACHE STRING "Name of OpenGL static library")
  set(VITA_VITASHARK_LIB vitashark CACHE STRING "Name of vitashark static library")
  set(VITA_PTHREAD_LIB pthread CACHE STRING "Name of pthread static library")

  message(STATUS "Using these static libraries:")
  message(STATUS "With OpenGL: ${VITA_GL_LIB}")
  message(STATUS "With vitashark: ${VITA_VITASHARK_LIB}")
  message(STATUS "With pthread: ${VITA_PTHREAD_LIB}")

  include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
else()
  if(CMAKE_BUILD_PREFIX)
    set(BUILD_PREFIX "${CMAKE_BUILD_PREFIX}")
  else()
    set(BUILD_PREFIX "/usr/local")
  endif()
endif()

set(SUPPORTS_GLSL_2_1 1 CACHE STRING "Host platform supports GLES 2.1 (1 = Yes, 0 = No)")

if(VITA)
  set(SUPPORTS_GLSL_2_1 0)
endif()

project(quickjs_gl)

include_directories("${BUILD_PREFIX}/include/quickjs")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

if(VITA)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q -O3 -ffast-math -mtune=cortex-a9 -mfpu=neon -Wno-incompatible-pointer-types -Wno-stringop-overflow -Wno-array-bounds -Wno-format-truncation")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-q -O3 -ffast-math -mtune=cortex-a9 -mfpu=neon -Wno-stringop-overflow -Wno-array-bounds -Wno-format-truncation")
endif()
# Default directory for QuickJS
if(LINUX)
  set (CMAKE_INSTALL_PREFIX "/usr/local")
endif()

add_subdirectory(src)
add_subdirectory(example)