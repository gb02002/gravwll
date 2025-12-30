cmake_minimum_required(VERSION 3.14)
project(gravwll_tests VERSION 0.0.1 DESCRIPTION "Gravwll tests" LANGUAGES CXX)

add_subdirectory(googletest)

file(GLOB_RECURSE GRAVWLL_TEST_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/unittests/*.cc
)

add_executable(gravwll_unittests ${GRAVWLL_TEST_SOURCES})
target_link_libraries(gravwll_unittests PRIVATE gravwll_warnings)

target_link_libraries(gravwll_unittests PRIVATE gravwll::simulation
  GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(gravwll_unittests)

