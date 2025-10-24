# find_package(raylib REQUIRED)
# find_package(Vulkan REQUIRED)
#
# if(Vulkan_FOUND)
#   message(STATUS "Vulkan найден")
#
#   # Добавляем vk-bootstrap и VMA
#   add_subdirectory(../../third_party/vk-bootstrap)
#   add_subdirectory(../../third_party/VulkanMemoryAllocator)
#
#   # Основной исполняемый файл
#   add_executable(YourProject main.cpp)
#   target_link_libraries(YourProject PRIVATE
#       raylib
#       vk-bootstrap
#       VulkanMemoryAllocator
#       Vulkan::Vulkan
#   )
#   target_include_directories(YourProject PRIVATE
#       ${Vulkan_INCLUDE_DIR}
#   )
#   target_compile_features(YourProject PRIVATE cxx_std_17)
#
# endif()
