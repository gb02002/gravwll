#!/usr/bin/env zsh
set -euo pipefail

BUILD_DIR=build_tests
BUILD_TYPE=RelWithDebInfo

cmake -S . -B ${BUILD_DIR} \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_LINKER_TYPE=MOLD \
  -DENABLE_TESTS=ON

cmake --build ${BUILD_DIR} --parallel

cd ${BUILD_DIR}
ctest --rerun-failed --output-on-failure
