
add_library(polygon-clip
  include/polygon_clip.hpp
  src/polygon_clip.cc
  src/polygon_clip_math.cc
  src/polygon_clip_math.hpp
  src/polygon_clip_priv.cc
  src/polygon_clip_priv.hpp
)

target_include_directories(polygon-clip PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

target_include_directories(polygon-clip
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)
