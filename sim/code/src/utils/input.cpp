// #include "utils/input.h"
// #include "utils/envs.h"
// #include <iostream>
// #include <memory>
// #include <stdexcept>
// #include <string>
//
// std::unique_ptr<Settings> ReadInput(int argc, char *argv[]) {
//   auto envs = Settings{};
//
//   for (int i = 1; i < argc; ++i) {
//     if (std::string(argv[i]) == "--headless") {
//       envs.headless = true;
//     } else if (std::string(argv[i]) == "--N" && i + 1 < argc) {
//       envs.N = std::atoi(argv[++i]); // Получаем следующее значение после --N
//     } else {
//       std::string err_value = argv[i];
//       std::string err_str = "Unknown argument: " + err_value;
//       throw std::runtime_error(err_str);
//     }
//   }
//   std::cout << "Headless mode: " << (envs.headless ? "Enabled" : "Disabled")
//             << "\n";
//   std::cout << "Number of particles: " << envs.N << "\n";
//   return std::make_unique<Settings>(envs);
// }
