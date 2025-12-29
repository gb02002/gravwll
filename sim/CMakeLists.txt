cmake_minimum_required(VERSION 3.12)
project(simulation
    VERSION 0.0.1
    DESCRIPTION "Gravwll sourcecode"
    LANGUAGES CXX
)
message(STATUS "gravwll_codebase: ${gravwll_codebase}")
set(gravwll_source_code_dir ${gravwll_codebase}/code)

file(GLOB_RECURSE gravwll_sources CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/src/*.cpp)
file(GLOB_RECURSE gravwll_sources1 CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/src/*.cc)
file(GLOB_RECURSE gravwll_headers CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/include/*.h)

add_library(simulation
  ${gravwll_sources} ${gravwll_sources1} ${gravwll_headers}
)
add_library(gravwll::simulation ALIAS simulation)

target_include_directories(simulation PUBLIC
  ${gravwll_source_code_dir}/include
  ${CMAKE_CURRENT_BINARY_DIR}/code/include
)

target_link_libraries(simulation PRIVATE
  # gravwll_warnings
  gravwll_vulkan_dependencies
)

if(GRAVWLL_SANITIZE)
  target_link_libraries(simulation PRIVATE gravwll_sanitize)
endif()

if(GRAVWLL_PERF)
  target_link_libraries(simulation PRIVATE gravwll_perf)
endif()

add_executable(simulation_bin
  ${gravwll_source_code_dir}/main.cc
)

target_link_libraries(simulation_bin PRIVATE gravwll::simulation gravwll_warnings)

