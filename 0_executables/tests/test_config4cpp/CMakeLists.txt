cmake_minimum_required(VERSION 2.8)

project(test_config4cpp)

add_library(FooConfiguration "FooConfiguration.cpp")
target_link_libraries(FooConfiguration config4HaptiComm)

add_executable(${PROJECT_NAME} "main.cpp")

link_directories(${CMAKE_BINARY_DIR})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC config4HaptiComm FooConfiguration)
