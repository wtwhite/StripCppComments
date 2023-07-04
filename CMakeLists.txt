# CMakeList.txt : CMake project for StripCppComments
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

project("StripCppComments")

# Add source to this project's executable.
add_library(CommentStripper OBJECT "CommentStripper.cpp" "CommentStripper.h")
add_executable(StripCppComments "main.cpp")
target_link_libraries(StripCppComments CommentStripper)

# Testing
enable_testing()

# Download GoogleTest from GitHub as a local dep
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(Tests tests.cpp)
target_link_libraries(Tests CommentStripper gtest_main)
include(GoogleTest)
gtest_discover_tests(Tests)
