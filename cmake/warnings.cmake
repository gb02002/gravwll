add_library(gravwll_warnings INTERFACE)

#======ERROR_FLAGS=======#

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set_property(TARGET gravwll_warnings PROPERTY
  INTERFACE_COMPILE_OPTIONS "")

  target_compile_options(gravwll_warnings INTERFACE
  $<$<COMPILE_LANGUAGE:CXX>:
    -Wall -Wextra -Wpedantic
    -Wshadow=compatible-local
    -Wconversion -Wsign-conversion -Wdouble-promotion
    -Wnull-dereference -Wnon-virtual-dtor -Wzero-as-null-pointer-constant
    -Wuseless-cast -Wredundant-move -Wclass-memaccess>
    -fdiagnostics-color=always
)
endif()

#======LTO/PGO_RULES=======#

option(GRAVWLL_ENABLE_LTO "Enable LTO" OFF)
if(GRAVWLL_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT IPO_OK)
  if(IPO_OK)
    set_property(TARGET gravwll_warnings PROPERTY
    INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

#======SANITIZE_FLAGS=======#

option(GRAVWLL_ENABLE_SANITIZE "Enable Sanitize" OFF)
if(GRAVWLL_ENABLE_SANITIZE)
  add_library(gravwll_sanitize INTERFACE)
  target_compile_options(gravwll_sanitize INTERFACE
  -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(gravwll_sanitize INTERFACE
  -fsanitize=address,undefined)
endif()

#======PERF_FLAGS=======#
option(GRAVWLL_ENABLE_PERF "Enable PERF" OFF)
if(GRAVWLL_ENABLE_PERF)
  add_library(gravwll_perf INTERFACE)
  target_compile_options(gravwll_perf INTERFACE
  -O3 -march=native -ftree-vectorize
  -fno-math-errno -fno-trapping-math
  -ffp-contract=fast -funsafe-math-optimizations
  -fno-math-errno -fno-trapping-math -fno-signed-zeros
  -fassociative-math)
endif()

#======UNSAFE_FLAGS=======#
option(GRAVWLL_ENABLE_UNSAFE "Enable unsafe" OFF)
if(GRAVWLL_ENABLE_UNSAFE)
  add_library(gravwll_fastmath INTERFACE)
  target_compile_options(gravwll_fastmath INTERFACE
  -ffast-math -ffinite-math-only)
endif()
