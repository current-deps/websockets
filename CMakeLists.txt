cmake_minimum_required(VERSION 3.14.1)

project(c5t-ws-module)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

set (lib "wsServer")

# Note: this `repo` is totally unused!
# set (repo "https://github.com/current-deps/${lib}")

# Note: this assumes the library is clond under `./wsServer` -- and `pls install` does exactly this.
add_subdirectory("${lib}")
target_compile_definitions(ws PRIVATE MAX_CLIENTS=16384)

# test server
if(ENABLE_TEST)
  add_executable(test_server src/test_server.cc)
  add_dependencies(test_server ws)
  target_link_libraries(test_server ws)
endif(ENABLE_TEST)
unset(ENABLE_TEST)
