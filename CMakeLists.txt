cmake_minimum_required(VERSION 3.14.1)

project(c5t-ws-module)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

set (lib "wsServerForkedFromTheldus")
set (repo "https://github.com/current-deps/${lib}")

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${lib}" AND IS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lib}")
  message(STATUS "Using `${lib}` from build dir.")
else()
  execute_process(OUTPUT_QUIET ERROR_QUIET COMMAND git clone ${repo} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/${lib}")
target_compile_definitions(ws PRIVATE MAX_CLIENTS=64)

# test server
if(ENABLE_TEST)
  add_executable(test_server src/test_server.cc)
  add_dependencies(test_server ws)
  target_include_directories(test_server PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${lib})
  target_link_libraries(test_server ws)
endif(ENABLE_TEST)
unset(ENABLE_TEST)

