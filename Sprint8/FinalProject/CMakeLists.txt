cmake_minimum_required(VERSION 3.12)
project (Sprint8Final)

file(GLOB SRCFILES
     "*.h"
     "*.cpp"
)
ADD_EXECUTABLE(search ${SRCFILES})
set_property(TARGET search PROPERTY CXX_STANDARD 20)
target_link_libraries( search pthread tbb)
#target_include_directories(search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
