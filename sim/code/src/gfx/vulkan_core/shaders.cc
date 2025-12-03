// #include "gfx/shader.h"
// #include <cstddef>
// #include <fstream>
// #include <ios>
// #include <stdexcept>
//
// static std::vector<char> load_shader(const std::string &filename) {
//   std::ifstream file(filename, std::ios::ate | std::ios::binary);
//
//   if (!file.is_open()) {
//     std::string err_msg = filename + ": cound not load";
//     throw std::runtime_error(err_msg);
//   }
//
//   size_t file_size = (size_t)file.tellg();
//   std::vector<char> buffer(file_size);
//
//   file.seekg(0);
//   file.read(buffer.data(), file_size);
//   file.close();
//
//   return buffer;
// }
