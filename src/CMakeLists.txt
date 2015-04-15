# Project setup
cmake_minimum_required(VERSION 3.1)
project(conduit)

# Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -g")

# Options for output
set(CMAKE_COLOR_MAKEFILE on)

# CUDA
find_package(CUDA REQUIRED)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3 -arch=sm_20)

# OpenCV
find_package(OpenCV REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

# SDL2
include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

# Oculus
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
find_package(Oculus REQUIRED)
include_directories(${Oculus_INCLUDE_DIRS})

# Compile each part
cuda_compile(BUILDTEST buildtest/buildtest.cu)
set(VIDEOREADER videoreader/videoreader.cpp)
set(RENDERER renderer/renderer.cpp)
set(UTIL util/imageutil.cpp)

# Final executable
add_executable(conduit main.cpp
  ${BUILDTEST}
  ${VIDEOREADER}
  ${RENDERER}
  ${UTIL}
  )

target_link_libraries(conduit
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${OpenCV_LIBS}
  ${CUDA_LIBRARIES}
  ${Oculus_LIBRARIES}
  )

target_compile_features(conduit PRIVATE cxx_range_for)
