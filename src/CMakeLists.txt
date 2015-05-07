# Project setup
cmake_minimum_required(VERSION 3.1)
project(conduit)

# Flags
if (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -g -Wno-deprecated")
endif (WIN32)

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
if (WIN32)
  find_package(SDL2 REQUIRED)
else (WIN32)
  include(FindPkgConfig)
  pkg_search_module(SDL2 REQUIRED sdl2)
endif (WIN32)
include_directories(${SDL2_INCLUDE_DIRS})

# Oculus
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
find_package(Oculus REQUIRED)
include_directories(${Oculus_INCLUDE_DIRS})

# Compile each part
cuda_compile(BUILDTEST buildtest/buildtest.cu)
set(VIDEOREADER videoreader/videoreader.cpp)
set(RENDERER renderer/renderer.cpp)
set(UTIL util/imageutil.cpp util/cylinderwarp.cpp)
set(RENDERTEST rendertest/rendertest.cpp)
set(OCULUS2 oculus2/oculus2.cpp)
set(OPTIMIZER optimizer/optimizer.cpp)

find_package (Threads)

LINK_DIRECTORIES( ${LINK_DIRECTORIES} /usr/local/lib/)

# Final executable
add_executable(conduit main.cpp
  ${RENDERTEST}
  ${BUILDTEST}
  ${VIDEOREADER}
  ${RENDERER}
  ${UTIL}
  ${OCULUS2}
  ${OPTIMIZER}
  )

target_link_libraries(conduit
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${OpenCV_LIBS}
  ${CUDA_LIBRARIES}
  ${Oculus_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

if (WIN32)
  link_directories(
    ${OPENGL_LIBRARIES}/../
    ${GLUT_LIBRARY}/../
    ${GLEW_LIBRARIES}/../
    ${SDL2_LIBRARIES}/../
    ${OpenCV_LIBS}/../
    ${CUDA_LIBRARIES}/../
    ${Oculus_LIBRARIES}/../
	C:/sdl2/lib/x64
	C:/OculusSDK/LibOVR/Lib/Windows/x64/Release/VS2013
    )

  include_directories(
    ${OpenGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
    ${OPENCV_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIR}
    ${Oculus_INCLUDE_DIRS}
	C:/sdl2/include
	C:/OculusSDK/LibOVR/Include
	C:/glm
    )
endif(WIN32)

target_compile_features(conduit PRIVATE cxx_range_for)
