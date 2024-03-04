cmake_minimum_required(VERSION 3.10)

project(ConstexprVector)

add_executable(
    ${PROJECT_NAME}
    constexpr_vector.cpp
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

