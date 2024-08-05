cmake_minimum_required(VERSION 3.14.1)

project(c5t-ws-module)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

set (lib "wsServerForkedFromTheldus")
set (repo "https://github.com/current-deps/${lib}")

add_subdirectory("${lib}")
target_compile_definitions(ws PRIVATE MAX_CLIENTS=16384)
add_library(websockets INTERFACE)
target_include_directories(websockets INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_dependencies(websockets ws)

# test server
if(ENABLE_TEST)
  add_executable(test_server src/test_server.cc)
  add_dependencies(test_server ws)
  target_link_libraries(test_server ws)
endif(ENABLE_TEST)
unset(ENABLE_TEST)

