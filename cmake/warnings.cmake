add_library(gravwll_warnings INTERFACE)

#======ERROR_RULES=======#

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set_property(TARGET gravwll_warnings PROPERTY
  INTERFACE_COMPILE_OPTIONS "")

  target_compile_options(gravwll_warnings INTERFACE
  $<$<COMPILE_LANGUAGE:CXX>:
    -Wall -Wextra -Werror -pedantic-errors -Wshadow=compatible-local -Wconversion
  >
  -fdiagnostics-color=always
)
endif()

#======LTO/PGO_RULES=======#

option(GRAVWLL_ENABLE_LTO "Enable LTO" ON)
if(GRAVWLL_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT IPO_OK)
  if(IPO_OK)
    set_property(TARGET gravwll_warnings PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

