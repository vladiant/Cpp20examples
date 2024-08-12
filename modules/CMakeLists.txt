cmake_minimum_required(VERSION 3.28)
project(std_module_example CXX)

# Set the version of C++ for the project
set(CMAKE_CXX_STANDARD 20)
# Default to C++ extensions being off. Clang's modules support have trouble
# with extensions right now and it is not required for any other compiler
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(foo)
target_sources(foo
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    foo.cxx
)
add_executable(hello main.cxx)
target_link_libraries(hello PRIVATE foo)
