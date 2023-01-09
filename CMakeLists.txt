cmake_minimum_required(VERSION 3.18)

#project(astro LANGUAGES CXX CUDA)
project(astro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(spdlog REQUIRED)
#find_package(CUDAToolkit REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

message("glew include: ${GLEW_INCLUDE_DIRS}")
message("glew include: ${GLEW_INCLUDE_DIR}")

include_directories(
  "ext/dear_spdlog/src"
  "ext/toml11"
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIR}
  ${CUDAToolkit_INCLUDE_DIRS}
  )

add_subdirectory("ext/dear_spdlog")
add_subdirectory("src")
