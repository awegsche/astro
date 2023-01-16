cmake_minimum_required(VERSION 3.18)

if (WIN32)
add_compile_definitions(NOMINMAX)
endif()

# --------------------------------------------------------------------------------------------------
# ---- AstroViewer ---------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

# ---- Executable ----------------------------------------------------------------------------------
#
#
message("cmake current dir: ${CMAKE_CURRENT_SOURCE_DIR}/ui")
message("cmake src dir: ${CMAKE_SOURCE_DIR}/ui")

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/image"
  "${CMAKE_CURRENT_SOURCE_DIR}/algorithms"
  )

add_executable(astro_viewer
  main.cpp
  ui/mainwindow.cpp
  image/photobatch.cpp
  image/image.h
  image/frame.h
  ui/screen.cpp
  ui/screen.h
  )


# ---- Linking -------------------------------------------------------------------------------------
#
target_link_libraries(astro_viewer PRIVATE
  GLEW::GLEW
  imgui::imgui
  glfw
  spdlog::spdlog
  libraw::libraw
  dear_spdlogger
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  )

if(LINUX)
  message("this is linux")
  target_link_libraries(astro_viewer
    OpenGL
  )
endif()

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
