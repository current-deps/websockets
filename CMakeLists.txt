cmake_minimum_required(VERSION 3.14.1)

project(c5t-ws-module)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

# Add ws c-server repo
include(ExternalProject)
ExternalProject_Add(libws
  GIT_REPOSITORY "https://github.com/Theldus/wsServer"
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/wsServer
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/wsServer
  CMAKE_CACHE_ARGS
    -DFOO_ENABLE_BAR:BOOL=1
  INSTALL_COMMAND ""
  )

# test server
if(ENABLE_TEST)
  link_directories(${CMAKE_CURRENT_BINARY_DIR}/wsServer)
  add_executable(test_server src/test_server.cc)
  add_dependencies(test_server libws)
  target_include_directories(test_server PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/wsServer)
  target_link_libraries(test_server ${CMAKE_CURRENT_BINARY_DIR}/wsServer/libws.a)
endif(ENABLE_TEST)
unset(ENABLE_TEST)

