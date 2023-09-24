cmake_minimum_required(VERSION 3.20)

project(PolygonClip)


set(CMAKE_CXX_STANDARD 17)

# library
add_subdirectory(lib)

option(PC_BUILD_EXAMPLE "option to build example" OFF)

if (${PC_BUILD_EXAMPLE})

  add_subdirectory(sandbox)

endif(${PC_BUILD_EXAMPLE})

