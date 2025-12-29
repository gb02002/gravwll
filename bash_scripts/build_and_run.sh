#!/usr/bin/env zsh
set -euo pipefail

BUILD_DIR=build
BUILD_TYPE=RelWithDebInfo

select_compiler() {
    if [[ $# -gt 0 ]]; then
        case $1 in
            c|clang|1) 
                echo "Using Clang"
                export CC=clang CXX=clang++
                ;;
            g|gcc|2)
                echo "Using GCC"
                export CC=gcc CXX=g++
                ;;
            *)
                echo "Usage: $0 [c/clang/1|g/gcc/2]"
                echo "  c/clang/1 - use Clang (default)"
                echo "  g/gcc/2   - use GCC"
                exit 1
                ;;
        esac
    else
        echo "Select compiler:"
        echo "  1) Clang (c)"
        echo "  2) GCC (g)"
        echo -n "Your choice [1]: "

        read -r -k1 choice
        echo

        case $choice in
            2|g|G) 
                echo "Using GCC"
                export CC=gcc CXX=g++
                ;;
            *) 
                echo "Using Clang (default)"
                export CC=clang CXX=clang++
                ;;
        esac
    fi
}
select_compiler "$@"

cmake -S . -B ${BUILD_DIR} \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_LINKER_TYPE=MOLD \
  -DCMAKE_C_COMPILER=${CC} \
  -DCMAKE_CXX_COMPILER=${CXX}

cmake --build ${BUILD_DIR} --parallel
cmake --install ${BUILD_DIR}

exec ${BUILD_DIR}/bin/simulation_bin
