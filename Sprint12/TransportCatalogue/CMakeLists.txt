cmake_minimum_required(VERSION 3.12)
project (Sprint12Final)

file(GLOB SRCFILES
     "*.h"
     "*.cpp"
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMY_DEBUG")
ADD_EXECUTABLE(catalogue ${SRCFILES})
set_property(TARGET catalogue PROPERTY CXX_STANDARD 20)

add_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)


#target_link_libraries( search pthread tbb)
#target_include_directories(search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
