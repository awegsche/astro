cmake_minimum_required(VERSION 3.18)

if (WIN32)
add_compile_definitions(NOMINMAX)
endif()

# --------------------------------------------------------------------------------------------------
# ---- Astro Library -------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

set(ASTRO_LIB_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/image
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms
  PARENT_SCOPE
)

set(ASTRO_LIB_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/star_detection.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/dark.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/frame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/image_path.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/light.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image/photobatch.h
  PARENT_SCOPE
)
  
set(ASTRO_LIB_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/image/image_path.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/image/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/image/photobatch.cpp
  PARENT_SCOPE
)

