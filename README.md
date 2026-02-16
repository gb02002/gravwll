# gravwll – N‑body Simulation with Fast Multipole Method and Modern Integrators

gravwll is a **single‑node, NUMA‑aware** N‑body simulation framework that combines the Fast Multipole Method (FMM) with high‑order Aarseth integrators (Hermite, IAS15). It is written in modern C++20 with a strong focus on **data‑oriented design (DOD)** and low‑level performance optimizations: cache locality, memory access patterns, and vectorization. A Vulkan‑based visualisation layer is under development for real‑time inspection of particle systems.

## Why?

Most open‑source N‑body codes fall into two camps:

- **Production astrophysics codes** (GADGET, AMUSE, NEMO) are often legacy Fortran/C, MPI/OpenMP‑heavy, designed for clusters, and require significant expertise to build and modify.
- **Educational / toy implementations** rarely go beyond Barnes‑Hut or direct summation, and almost never include modern integrators or cache‑aware FMM.

There is a gap: a **modern, well‑documented, single‑NUMA code** that leverages FMM, advanced integrators, and data‑oriented design—approachable for non‑specialists yet efficient enough for serious experiments.

gravwll aims to fill that gap.

## Personal Why

Two years ago, while escaping the abstraction hell of Python, I stumbled upon SIAM’s **"Top 10 Algorithms of the 20th Century"**. The Fast Multipole Method was the only one I had never heard of—and the Wikipedia page left me more confused than before. Why is Barnes‑Hut still used everywhere? What makes FMM so special? Why near-linear FMM is still utilized only in ~15% of studies?

That's why it went off: from Go to C++, from Wiki to Greengard & Rokhlin, from naive loops to cache‑aware data structures. Along the way I absorbed wisdom from Aarseth (integrators and general n-body history), Pikus (what is performance and why you cook it wrong), Fog (practical advices regarding CPU architecture and separate instructions), etc.

## Features

- **Fast Multipole Method** (adaptive octree, multipole expansions up to order ~10, M2M/M2L/L2L translations)
- **Aarseth integrators**: Hermite (4th‑order) and IAS15 (15th‑order, adaptive)
- **Data‑oriented design**: AoSoA storage, Morton‑ordered nodes for locality (see [morton-order-test](https://github.com/gb02002/morton-order-test))
- **NUMA‑aware memory management** (arena allocators inspired by kernel slab allocators)
- **C++20 concepts** for particle types, expansion orders, and interaction kernels
- **Intel TBB** for parallel tree traversal and interaction lists
- **Google Benchmark** & **perf** integration for continuous performance tracking
- **Vulkan visualisation** (in‑progress) for real‑time debugging and presentation

## Current Status

gravwll is a **long‑term work in progress**. Core components are functional:

- [x] Octree construction with Morton codes
- [WIP] Multipole expansions (spherical harmonics, up to order 8)
- [WIP] Translation operators (M2M, M2L, L2L)
- [WIP] Hermite integrator (4th‑order)
- [WIP] Vulkan frontend
- [ ] Widen integrator pool for runtime pick (incl symplex and their usability)
- [ ] Production‑ready error estimation and adaptive refinement

The code is **not ready for production use**.

## Dev doc

More documentation can be found under docs/ directory. There are rules, build flags and current TODO.

## Building

```bash
git clone --recursive https://github.com/gb02002/gravwll.git
cd gravwll
./bash_scripts/build_and_run.sh
```

Dependencies:

- C++20 compiler (GCC 11+, Clang 14+)
- Intel TBB(not in master yet)
- Vulkan SDK
- glm
- SDL3
- gTEST
- CMake3.10+
- Google Benchmark (optional)

## Usage

Run a basic simulation():

```bash
cp /config/test.config.conf /config/config.conf
./build/bin/simulation_bin
```

Benchmarks are build with separate Makefiles and can be found under benchmarks/ directory.

## Project Structure

```
gravwll/
  ├── assets
  │   ├── datasets
  │   └── shaders
  ├── bash_scripts
  │   ├── build_and_run.sh
  │   ├── debug.sh
  │   └── tests.sh
  ├── benchmarks
  │   └── micro
  ├── cmake
  │   ├── dependencies.cmake
  │   ├── options.cmake
  │   └── warnings.cmake
  ├── CMakeLists.txt
  ├── config
  │   └── test.config.conf
  ├── docs
  │   ├── CONVENTIONS.md
  │   ├── FLAGS.md
  │   ├── personal
  │   └── TODO.md
  ├── README.md
  ├── sim
  │   ├── CMakeLists.txt
  │   └── code
  ├── tests
  │   ├── CMakeLists.txt
  │   ├── googletest
  │   └── unittests
  └── third_party
      ├── glm
      ├── imgui
      ├── sdl3
      ├── vk-bootstrap
      └── VulkanMemoryAllocator
```

## Related Research Repositories

- [**morton-order-test**](https://github.com/gb02002/SFCs_tests) – experiments with Morton order for octree node locality
- [**fmm-python-prototype**](https://github.com/gb02002/fmm-prototype) – Python prototype used to validate FMM expansions
- [**hermite-integrator-prototype**](https://github.com/gb02002/hermite) – simple integrator tests

## Goals (Non‑Exhaustive)

- Provide a **clean, readable, and well‑commented** codebase that can serve as a reference for others learning FMM and high‑performance N‑body techniques.
- Achieve **single‑node performance** competitive with legacy MPI codes for moderate particle counts (<10⁷).
- Build a **Vulkan visualiser** that runs on integrated GPUs, making the code accessible to students and hobbyists without expensive hardware.

## License

Project utilizes GPLv3 License.

---

**Contributions, issues, and questions are welcome.** If you're also exploring FMM, integrators, or data‑oriented design, feel free to open a discussion.
