cmake_minimum_required(VERSION 3.18)

add_executable(astro_viewer
  main.cpp
  mainwindow.cpp
  photobatch.cpp
  )

target_link_libraries(astro_viewer
  GLEW::GLEW
  glfw
  dear_spdlogger
  spdlog::spdlog
  )
