cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_library(RT_LIB rt)

add_executable(parent parent.c)
if(RT_LIB)
    target_link_libraries(parent ${RT_LIB})
endif()

add_executable(child child.c)
if(RT_LIB)
    target_link_libraries(child ${RT_LIB})
endif()