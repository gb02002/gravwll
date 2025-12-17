#!/bin/sh
set -e

# Must read how to check if proj needs rebuild
# rm bin/simulation_bin
if [ ! -d "build" ]; then
  mkdir build || {
    echo "Couldn't create build folder"
    exit 1
  }
fi

cd build || {
  echo "Couldn't enter build folder"
  exit 1
}
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_FLAGS=-print-prog-name=ld -DCMAKE_CXX_FLAGS="-fuse-ld=mold" ..

make -j$(($(nproc) - 1))
cd ../bin/ || exit
if [ -f "simulation_bin" ]; then
  echo "Запуск simulation_bin..."

  ./simulation_bin
else
  echo "Исполняемый файл simulation_bin не найден."
fi
