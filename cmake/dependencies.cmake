# dependencies.cmake

set(THIRD_PARTY_DIR ${gravwll_root}/third_party)

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
  message(STATUS "SDL3 connected")
else()
  message(FATAL_ERROR "SDL3 not found in third_party!")
endif()

# VK-BOOTSTRAP
if(EXISTS ${THIRD_PARTY_DIR}/vk-bootstrap/CMakeLists.txt)
  add_subdirectory(${THIRD_PARTY_DIR}/vk-bootstrap)
  message(STATUS "vk-bootstrap is included")
else()
  message(FATAL_ERROR "vk-bootstrap is missing in third_party!")
endif()

# VMA (header-only)
if(EXISTS ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include/vk_mem_alloc.h)
  add_library(VulkanMemoryAllocator INTERFACE)
  target_include_directories(VulkanMemoryAllocator INTERFACE
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

  target_include_directories(imgui PUBLIC
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
if(EXISTS ${THIRD_PARTY_DIR}/glm/CMakeLists.txt)
  option(GLM_TEST_ENABLE OFF)
  add_subdirectory(${THIRD_PARTY_DIR}/glm)
  set(GLM_TARGET glm::glm)
  message(STATUS "GLM included via CMake")
elseif(EXISTS ${THIRD_PARTY_DIR}/glm/glm/glm.hpp)
  # Иначе создаем интерфейсный таргет
  add_library(glm INTERFACE)
  target_include_directories(glm INTERFACE ${THIRD_PARTY_DIR}/glm)
  set(GLM_TARGET glm)
  message(STATUS "GLM included (header-only, interface)")
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
)

if(TARGET imgui)
  target_link_libraries(gravwll_vulkan_dependencies INTERFACE imgui)
endif()
