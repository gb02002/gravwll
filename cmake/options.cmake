#======OS_NAME=======#

cmake_host_system_information(RESULT gravwll_platform QUERY OS_NAME)
string(TOUPPER ${gravwll_platform} gravwll_upper_platform)
string(TOLOWER ${gravwll_platform} gravwll_lower_platform)

#======Options=======#

option(ENABLE_TESTS "Enable unittesting" OFF)
option(GRAVWLL_SANITIZE "Enable sanitize" OFF)
option(GRAVWLL_PERF "Perf build" OFF)
option(GRAVWLL_ENABLE_LTO "Include lto" OFF)

#======Directories=======#

set(gravwll_codebase ${gravwll_root}/sim CACHE PATH "Path to the codebase")
set(gravwll_config ${gravwll_root}/config CACHE PATH "Path to config directory")
set(gravwll_tests_dir ${gravwll_root}/tests CACHE PATH "Path to tests")
set(gravwll_assets_dir ${gravwll_root}/assets CACHE PATH "Path to assets")
set(gravwll_shaders_dir ${gravwll_assets_dir}/shaders CACHE PATH "Path to assets")

#======Configurations=======#

list(APPEND CMAKE_MODULE_PATH
  ${gravwll_config}/cmake
  ${gravwll_codebase}/cmake
  ${gravwll_tests_dir}/cmake
)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDART_REQUIRED ON)

set(GRAVWLL_CONFIG_DIR_PATH "${gravwll_config}")
set(GRAVWLL_SHADER_DIR_PATH "${gravwll_shaders_dir}")
message(STATUS "This is is GRAVWLL_CONFIG_DIR_PATH: ${GRAVWLL_CONFIG_DIR_PATH}")
message(STATUS "This is is GRAVWLL_SHADER_DIR_PATH: ${GRAVWLL_SHADER_DIR_PATH}")
include_directories(${CMAKE_BINARY_DIR})

#======BUILD_TYPES=======#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

#======CACHE=======#

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()
