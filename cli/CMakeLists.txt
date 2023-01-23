cmake_minimum_required(VERSION 3.18)

if (WIN32)
add_compile_definitions(NOMINMAX)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(
  "astro includes: ${ASTRO_LIB_INCLUDE_DIRS}"
  )

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ${ASTRO_LIB_INCLUDE_DIRS}
  )

set(HEADERS
  ${ASTRO_LIB_HEADERS}
  )

set(SOURCES
  main.cpp
  ${ASTRO_LIB_SOURCES}

  )

add_executable(astro_cli
  ${HEADERS}
  ${SOURCES}
  )


# ---- Linking -------------------------------------------------------------------------------------
#
set(AV_LIBS "")

list(APPEND AV_LIBS
  spdlog::spdlog
  libraw::libraw
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  nbtlib
  Eigen3::Eigen
  )

target_link_libraries(astro_cli PRIVATE ${AV_LIBS})
