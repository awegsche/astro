cmake_minimum_required(VERSION 3.18)

project(astro LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(spdlog REQUIRED)
find_package(CUDAToolkit REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
  ${CUDAToolkit_INCLUDE_DIRS}
  )

add_subdirectory("src")
