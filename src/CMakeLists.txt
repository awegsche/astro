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

set(HEADERS
  image/image.h
  image/light.h
  image/dark.h
  image/frame.h
  image/image_path.h
  )

set(SOURCES
  main.cpp
  image/photobatch.cpp
  image/image_path.cpp
  image/image.cpp
  ui/screen.cpp
  ui/mainwindow.cpp
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
