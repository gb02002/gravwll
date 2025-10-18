#!/bin/sh

# rm -rf build_tests
mkdir -p build_tests
cd build_tests || {
  echo "Couldn't create tests folder"
  exit 1
}
cmake -DENABLE_TESTS=ON ..
make "-j$(nproc)"
ctest --rerun-failed --output-on-failure
