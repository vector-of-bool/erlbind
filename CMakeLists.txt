cmake_minimum_required(VERSION 3.0.0)
project(erlbind VERSION 0.0.0)

include(CTest)
enable_testing()

include(FindErlang.cmake)

add_library(erlbind erlbind.cpp)

add_library(my-nifs SHARED nifs.cpp)
target_link_libraries(my-nifs PRIVATE erl_nif)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
