cmake_minimum_required(VERSION 3.18)

if (WIN32)
add_compile_definitions(NOMINMAX)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------------------------------------
# ---- AstroViewer ---------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

# ---- Executable ----------------------------------------------------------------------------------
#
#

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ${ASTRO_LIB_INCLUDE_DIRS}
  ui
  )

set(HEADERS
  ui/mainwindow.h
  ui/screen.h
  ${ASTRO_LIB_HEADERS}
  )

set(SOURCES
  main.cpp
  ui/mainwindow.cpp
  ui/screen.cpp
  ${ASTRO_LIB_SOURCES}

  )

add_executable(astro_viewer
  ${HEADERS}
  ${SOURCES}
  )


# ---- Linking -------------------------------------------------------------------------------------
#
set(AV_LIBS "")

list(APPEND AV_LIBS
  GLEW::GLEW
  imgui::imgui
  glfw
  spdlog::spdlog
  libraw::libraw
  dear_spdlogger
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  nbtlib
  )

if(LINUX)
  message("this is linux")
  list(APPEND AV_LIBS
    OpenGL
  )
endif()

target_link_libraries(astro_viewer PRIVATE ${AV_LIBS})
# ---- Resources -----------------------------------------------------------------------------------
#
set(copiedShaders "")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/screen.vert
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/shaders/screen.vert
    ${CMAKE_CURRENT_BINARY_DIR}/screen.vert
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/screen.vert
    VERBATIM
  )
list(APPEND copiedShaders ${CMAKE_CURRENT_BINARY_DIR}/screen.vert)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/screen.frag
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/shaders/screen.frag
    ${CMAKE_CURRENT_BINARY_DIR}/screen.frag
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/screen.frag
    COMMENT "copying file"
    VERBATIM
  )
list(APPEND copiedShaders ${CMAKE_CURRENT_BINARY_DIR}/screen.frag)

add_custom_target(
  copy_shaders ALL
  DEPENDS ${copiedShaders}
  )
