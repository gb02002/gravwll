#!/usr/bin/env zsh
set -euo pipefail

BUILD_DIR=build
BUILD_TYPE=RelWithDebInfo

cmake -S . -B ${BUILD_DIR} \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_LINKER_TYPE=MOLD

cmake --build ${BUILD_DIR} --parallel

exec gdb ${BUILD_DIR}/bin/simulation_bin
