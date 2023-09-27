# example needs glfw3
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

if(APPLE)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGL_SILENCE_DEPRECATION")
endif()

function(pc_example name)
  set(multiValueArgs FILES)

  cmake_parse_arguments(PC_FLILE_LISTS "" "" ${multiValueArgs} ${ARGN})

  message("target_name -> ${name}")
  message("source lists -> ${PC_FLILE_LISTS_FILES}")

  add_executable(${name} ${PC_FLILE_LISTS_FILES})

  target_link_libraries(${name} PRIVATE polygon-clip glfw $<TARGET_OBJECTS:example> OpenGL::GL)
endfunction(pc_example)

add_library(example OBJECT example.hpp example.cc)

target_link_libraries(example PUBLIC glfw polygon-clip)

pc_example(clip-example FILES clip_example.cc)

pc_example(union-example FILES union_example.cc)
