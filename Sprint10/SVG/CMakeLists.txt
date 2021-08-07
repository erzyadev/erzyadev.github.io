cmake_minimum_required(VERSION 3.12)
project (svg)

file(GLOB SRCFILES
     "*.h"
     "*.cpp"
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMY_DEBUG")

ADD_EXECUTABLE(svg ${SRCFILES})
set_property(TARGET svg PROPERTY CXX_STANDARD 17)

