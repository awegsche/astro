cmake_minimum_required(VERSION 3.18)

if (WIN32)
add_compile_definitions(NOMINMAX)
endif()

add_executable(astro_viewer
  main.cpp
  mainwindow.cpp
  photobatch.cpp
  image.h
  frame.h
  screen.cpp
  screen.h
  )

target_link_libraries(astro_viewer PRIVATE
  GLEW::GLEW
  imgui::imgui
  glfw
  spdlog::spdlog
  libraw::libraw
  dear_spdlogger
  )

if(LINUX)
  message("this is linux")
  target_link_libraries(astro_viewer
    OpenGL
  )
endif()

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
