cmake_minimum_required(VERSION 3.18)

#project(astro LANGUAGES CXX CUDA)
project(astro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(spdlog CONFIG REQUIRED)
find_package(CUDAToolkit REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
#find_package(devil REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
#find_package(TIFF CONFIG REQUIRED)

message("glew include: ${GLEW_INCLUDE_DIRS}")
message("glew include: ${GLEW_INCLUDE_DIR}")

include_directories(
  "ext/dear_spdlog/src"
  "ext/toml11"
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIR}
  ${CUDAToolkit_INCLUDE_DIRS}
  ${SPDLOG_INCLUDE_DIR}
  ${IMGUI_INCLUDE_DIR}
  ${Eigen3_INCLUDE_DIR}
  )

message("CUDA toolkit: ${CUDAToolkit_INCLUDE_DIRS}")
message("eigen incl: ${Eigen3_INCLUDE_DIR}")
message("eigen incl: ${Eigen3_INCLUDE_DIRS}")

add_subdirectory("ext/dear_spdlog")
add_subdirectory("ext/LibRaw-cmake")
add_subdirectory("ext/nbt")

# the library (bulk of the code)
add_subdirectory("lib")

# the apps
add_subdirectory("cli")
add_subdirectory("viewer")
