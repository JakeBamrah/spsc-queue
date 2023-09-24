# export compile type dependencies produced in build system
# we need this to provide full information on dependency type (gtest, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# setup config for google test
cmake_minimum_required(VERSION 3.16)
project(spsc-queue)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  unittests
  unittests/readerwriter_queue.cc
  unittests/circular_buffer.cc
)
target_link_libraries(
  unittests atomic
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unittests)
