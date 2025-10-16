cmake_minimum_required(VERSION 3.14)
project(gravwll_tests VERSION 0.0.1 DESCRIPTION "Gravwll tests" LANGUAGES CXX)

add_subdirectory(googletest)

add_executable(gravwll_unittests
    unittests/config/input.cc
    # unittests/tree/insert.cpp
    # unittests/tree/split.cpp
    # unittests/tree/boxes.cpp

    unittests/physics/pairwise.cpp
)

target_include_directories(gravwll_unittests PRIVATE ${gravwll_headers})

target_link_libraries(gravwll_unittests PRIVATE gravwll::simulation GTest::gtest GTest::gtest_main)

# Автоматический запуск тестов
include(GoogleTest)
gtest_discover_tests(gravwll_unittests)

