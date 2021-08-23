cmake_minimum_required(VERSION 3.12)
project (RLE)

file(GLOB SRCFILES
     "*.h"
     "*.cpp"
)
ADD_EXECUTABLE(rle ${SRCFILES})
set_property(TARGET rle PROPERTY CXX_STANDARD 20)
#target_link_libraries( rle pthread tbb)
#target_include_directories(search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
