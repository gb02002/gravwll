cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDART_REQUIRED True)
set(gravwll_root ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(GNUInstallDirs)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink
          ${CMAKE_BINARY_DIR}/compile_commands.json
          ${CMAKE_SOURCE_DIR}/compile_commands.json
)

add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
)

project(gravwll)

include(${gravwll_root}/cmake/options.cmake)
include(${gravwll_root}/cmake/warnings.cmake)
include(${gravwll_root}/cmake/dependencies.cmake)

if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(${gravwll_codebase})
  add_subdirectory(${gravwll_tests_dir})
else()
  add_subdirectory(${gravwll_codebase})
endif()
