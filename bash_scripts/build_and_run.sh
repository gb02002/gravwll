#!/bin/sh

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
cmake ..

make "-j$(nproc)"
cd ../bin/ || exit
if [ -f "simulation_bin" ]; then
  echo "Запуск simulation_bin..."

  ./simulation_bin
else
  echo "Исполняемый файл simulation_bin не найден."
fi
