cmake_minimum_required(VERSION 3.10)

project(format_vector)

add_executable(
    ${PROJECT_NAME}
    format_vector.cpp
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

