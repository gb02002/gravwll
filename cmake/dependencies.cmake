# dependencies.cmake

set(THIRD_PARTY_DIR ${gravwll_root}/third_party)

# VULKAN
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
  message(STATUS "Vulkan найден, версия: ${Vulkan_VERSION}")
else()
  message(FATAL_ERROR "Vulkan не найден!")
endif()

# SDL3
if(EXISTS ${THIRD_PARTY_DIR}/sdl3/CMakeLists.txt)
  set(SDL_SHARED OFF CACHE BOOL "Build shared library" FORCE)
  set(SDL_STATIC ON CACHE BOOL "Build static library" FORCE)
  set(SDL_TEST OFF CACHE BOOL "Build tests" FORCE)

  add_subdirectory(${THIRD_PARTY_DIR}/sdl3)
  message(STATUS "SDL3 подключен")
else()
  message(FATAL_ERROR "SDL3 не найден в third_party!")
endif()

# RAYLIB
# find_package(raylib REQUIRED)
# if(raylib_FOUND)
#   message(STATUS "raylib найден")
# else()
#   message(FATAL_ERROR "raylib не найден!")
# endif()

# VK-BOOTSTRAP
if(EXISTS ${THIRD_PARTY_DIR}/vk-bootstrap/CMakeLists.txt)
  add_subdirectory(${THIRD_PARTY_DIR}/vk-bootstrap)

  target_include_directories(vk-bootstrap
  PUBLIC ${THIRD_PARTY_DIR}/vk-bootstrap/src)
  message(STATUS "vk-bootstrap подключен")
else()
  message(FATAL_ERROR "vk-bootstrap не найден в third_party!")
endif()

# VMA
if(EXISTS ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include/vk_mem_alloc.h)
  add_library(VulkanMemoryAllocator INTERFACE)

  target_include_directories(VulkanMemoryAllocator INTERFACE
        ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
    )

  target_link_libraries(VulkanMemoryAllocator INTERFACE Vulkan::Vulkan)

  message(STATUS "VMA подключен (header-only)")
else()
  message(FATAL_ERROR "VMA не найден в third_party!")
endif()

# ImGui header-only
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
if(EXISTS ${THIRD_PARTY_DIR}/imgui/imgui.h)
  include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
endif()

# if(EXISTS ${THIRD_PARTY_DIR}/imgui/imgui.h)
#   add_library(imgui
#     ${THIRD_PARTY_DIR}/imgui/imgui.cpp
#     ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
#     ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
#     ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
#     ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
#
#     # Бэкенды которые нам нужны
#     ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl3.cpp
#     ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_vulkan.cpp
#   )
#   target_include_directories(imgui PUBLIC ${THIRD_PARTY_DIR}/imgui ${THIRD_PARTY_DIR}/imgui/backend)
#   target_link_libraries(imgui SDL3 gravwll_vulkan_dependencies)
#   target_compile_features(imgui PRIVATE cxx_std_11)
#   message(STATUS "ImGui подключен (header-only)")
# else()
#   message(FATAL_ERROR "ImGui не найден в third_party!")
# endif()

add_library(gravwll_vulkan_dependencies INTERFACE)
target_link_libraries(gravwll_vulkan_dependencies INTERFACE
    Vulkan::Vulkan
    vk-bootstrap
    VulkanMemoryAllocator
    SDL3
)

target_include_directories(gravwll_vulkan_dependencies INTERFACE
    ${THIRD_PARTY_DIR}/vk-bootstrap/src
    ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/src
)
