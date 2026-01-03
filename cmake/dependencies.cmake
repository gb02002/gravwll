# dependencies.cmake

set(THIRD_PARTY_DIR ${gravwll_root}/third_party)

#=====TBB=====#
find_package(TBB REQUIRED)
if(TBB_FOUND)
  message(STATUS "TBB found, version: ${TBB_VERSION}")
else()
  message(FATAL_ERROR "TBB is missing!")
endif()

#=====GRAPHICS=====#
# VULKAN
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
  message(STATUS "Vulkan found, version: ${Vulkan_VERSION}")
else()
  message(FATAL_ERROR "Vulkan is missing!")
endif()

# SDL3
if(EXISTS ${THIRD_PARTY_DIR}/sdl3/CMakeLists.txt)
  set(SDL_SHARED OFF CACHE BOOL "Build shared library" FORCE)
  set(SDL_STATIC ON CACHE BOOL "Build static library" FORCE)
  set(SDL_TEST OFF CACHE BOOL "Build tests" FORCE)

  add_subdirectory(${THIRD_PARTY_DIR}/sdl3)
  set_target_properties(SDL3-static PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
  # set_target_properties(SDL3-shared PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

  message(STATUS "SDL3 connected")
else()
  message(FATAL_ERROR "SDL3 not found in third_party!")
endif()

# VK-BOOTSTRAP
if(EXISTS ${THIRD_PARTY_DIR}/vk-bootstrap/CMakeLists.txt)
  add_subdirectory(${THIRD_PARTY_DIR}/vk-bootstrap SYSTEM)
  set_target_properties(vk-bootstrap PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
  target_compile_options(vk-bootstrap PRIVATE -Wno-error)

  message(STATUS "vk-bootstrap is included")
else()
  message(FATAL_ERROR "vk-bootstrap is missing in third_party!")
endif()

# VMA (header-only)
if(EXISTS ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include/vk_mem_alloc.h)
  add_library(VulkanMemoryAllocator INTERFACE)
  target_include_directories(VulkanMemoryAllocator SYSTEM INTERFACE
    ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
  )
  target_link_libraries(VulkanMemoryAllocator INTERFACE Vulkan::Vulkan)
  message(STATUS "VMA included (header-only)")
else()
  message(FATAL_ERROR "VMA missing in third_party!")
endif()

# ImGui
if(EXISTS ${THIRD_PARTY_DIR}/imgui/imgui.h)
  add_library(imgui STATIC
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl3.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_vulkan.cpp
  )

  set_target_properties(imgui PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
  target_compile_options(imgui PRIVATE -Wno-error)
  target_include_directories(imgui SYSTEM PUBLIC
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imgui/backends
  )

  target_link_libraries(imgui PUBLIC
    SDL3::SDL3
    Vulkan::Vulkan
  )

  message(STATUS "ImGui connected static")
else()
  message(FATAL_ERROR "ImGui not in third_party!")
endif()

# GLM (header-only)
if(EXISTS ${THIRD_PARTY_DIR}/glm/glm/glm.hpp)

  add_library(glm_sandbox INTERFACE)
  target_include_directories(glm_sandbox SYSTEM INTERFACE
    ${THIRD_PARTY_DIR}/glm
  )

  target_compile_options(glm_sandbox INTERFACE
    -Wno-conversion
    -Wno-sign-conversion
    -Wno-shadow
  )

  set(GLM_TARGET glm_sandbox)
  message(STATUS "GLM included (sandbox header-only)")

else()
  message(FATAL_ERROR "GLM is not in third_party!")
endif()

add_library(gravwll_vulkan_dependencies INTERFACE)

target_link_libraries(gravwll_vulkan_dependencies INTERFACE
    Vulkan::Vulkan
    vk-bootstrap
    VulkanMemoryAllocator
    SDL3::SDL3
    ${GLM_TARGET}
    imgui
    TBB::tbb
)

if(TARGET imgui)
  target_link_libraries(gravwll_vulkan_dependencies INTERFACE imgui)
endif()
