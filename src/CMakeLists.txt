# Project setup
cmake_minimum_required(VERSION 2.8)
project(conduit)

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

# Compile each part
cuda_compile(BUILDTEST buildtest/buildtest.cu)

# Final executable
add_executable(conduit main.cpp ${BUILDTEST})
target_link_libraries(conduit
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  ${OpenCV_LIBS}
  ${CUDA_LIBRARIES}
  )
