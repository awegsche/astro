cmake_minimum_required(VERSION 3.18)

add_executable(astro_viewer
  main.cpp
  mainwindow.cpp
  photobatch.cpp
  image.h
  frame.h
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
