#pragma once
#include "ds/tree/octree.h"
#include "gfx/camera.h"
#include <memory>
#include <vector>

void invoke_engine(std::vector<Particle> raw_data,
                   std::unique_ptr<AROctree> tree);
int gen_ticks(std::vector<Particle> raw_data, std::unique_ptr<AROctree> tree,
              MyCamera3D *camera);
int start_main_logic(std::unique_ptr<Settings> settings);

int headless_ticks(std::vector<Particle> raw_data,
                   std::unique_ptr<AROctree> tree);
