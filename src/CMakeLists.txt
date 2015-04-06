# Project setup
cmake_minimum_required(VERSION 2.8)
project(conduit)

# Options for output
set(CMAKE_COLOR_MAKEFILE on)

# Tell CMake to look for CUDA
find_package(CUDA REQUIRED)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3 -arch=sm_20)

# CUDA Compile each part
cuda_compile(BUILDTEST buildtest/buildtest.cu)

# Final executable
cuda_add_executable(conduit main.cpp ${BUILDTEST})
