cmake_minimum_required(VERSION 3.12)
project(simulation
    VERSION 0.0.1
    DESCRIPTION "Gravwll sourcecode"
    LANGUAGES CXX
)
message(STATUS "gravwll_codebase: ${gravwll_codebase}")
set(gravwll_source_code_dir ${gravwll_codebase}/code)

add_subdirectory(${gravwll_source_code_dir})

file(GLOB_RECURSE gravwll_sources CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/src/*.cpp)
file(GLOB_RECURSE gravwll_sources1 CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/src/*.cc)
file(GLOB_RECURSE gravwll_headers CONFIGURE_DEPENDS
  ${gravwll_source_code_dir}/include/*.h)

add_library(${PROJECT_NAME}
  ${gravwll_sources} ${gravwll_sources1} ${gravwll_headers})
add_library(gravwll::simulation ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC
  ${gravwll_source_code_dir}/include)

set(CONFIG_PATH "${GRAVWLL_CONFIG_DIR_PATH}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/code/include/ctx/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/code/include/ctx/config.h @ONLY)
target_include_directories(simulation PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/code/include
)
add_executable(simulation_bin ${gravwll_source_code_dir}/main.cc)

target_include_directories(simulation_bin PRIVATE
${gravwll_source_code_dir}/include)
target_link_libraries(simulation_bin PRIVATE gravwll::simulation)
target_link_libraries(
  simulation_bin PRIVATE
  gravwll::simulation gravwll_vulkan_dependencies
)
target_include_directories(simulation_bin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
