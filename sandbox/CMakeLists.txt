# example needs glfw3
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

if(APPLE)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGL_SILENCE_DEPRECATION")
endif()

add_library(example OBJECT example.hpp example.cc)

target_link_libraries(example PUBLIC glfw polygon-clip)

add_executable(clip-example clip_example.cc)

target_link_libraries(clip-example PRIVATE polygon-clip glfw $<TARGET_OBJECTS:example> OpenGL::GL)
