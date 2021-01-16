cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(project_name stream_view)
project(stream_view)

find_package(GTest CONFIG REQUIRED)

###############################
#only tests are need to build as project.
###############################
include_directories(include/)
add_executable(${project_name}
    tests/stream_view.cpp
    )
add_executable(generated_code
    tests/generated_code.cpp
    )
target_link_libraries(${project_name}
    PUBLIC
    GTest::gtest
    GTest::gtest_main
)

enable_testing()
