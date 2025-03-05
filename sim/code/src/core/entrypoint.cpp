// #include "core/entrypoint.h"
// #include "core/bodies/particles.h"
// #include "ds/storage/storage.h"
// #include "ds/tree/octree.h"
// #include "gfx/renderer.h"
// #include "utils/genParticles.h"
// #include <atomic>
// #include <iostream>
// #include <memory>
// #include <raylib.h>
// #include <thread>
// #include <vector>
//
// std::atomic<bool> task_finished{false};
//
// void invoke_engine(std::vector<Particle> raw_data,
//                    std::unique_ptr<AROctree> tree) {
//   for (int i = 0; i < raw_data.size(); ++i) {
//     tree->insert(raw_data[i], i);
//   }
//   task_finished = true;
//   return;
// }
//
// int gen_ticks(std::vector<Particle> raw_data, std::unique_ptr<AROctree> tree,
//               MyCamera3D *camera) {
//   AROctree *raw_tree = tree.get();
//   std::thread t1(invoke_engine, raw_data, std::move(tree));
//   t1.detach();
//   SetTargetFPS(60);
//
//   while (!WindowShouldClose()) {
//     CheckKeys(camera);
//
//     BeginDrawing();
//     ClearBackground(BLACK);
//     BeginMode3D(camera->camera);
//
//     StartRenderTree(&camera->camera, raw_tree);
//
//     EndMode3D();
//     EndDrawing();
//     if (task_finished) {
//       std::cout << "Задача во втором потоке завершена.\n";
//       break;
//     }
//   }
//   CloseWindow();
//   return 0;
// }
//
// int headless_ticks(std::vector<Particle> raw_data,
//                    std::unique_ptr<AROctree> tree) {
//   for (int i = 0; i < raw_data.size(); ++i) {
//     tree->insert(raw_data[i], i);
//
//     // std::this_thread::sleep_for(std::chrono::milliseconds(3));
//   }
//   return 0;
// }
//
//  int start_main_logic(std::unique_ptr<Settings> settings) {
//    Storage storage;
//    auto main_tree = std::make_unique<AROctree>(
//        10, 8, MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
//        &storage);
//    std::vector<Particle> raw_data = genRand(settings->N);
//
//    if (settings->headless == true) {
//      headless_ticks(raw_data, std::move(main_tree)); // Передача владения
//    } else {
//      auto camera = InitRenderer();
//      gen_ticks(raw_data, std::move(main_tree), camera.get()); // Камера
//      остаётся
//    }
//    return 0;
//  }
