#include "bot.hpp"
#include <iostream>
#include <fstream>

int main() {
  std::ifstream f("token");
  char buf[256];
  f.getline(buf, 256, '\n');

  dualventer::Bot bot(buf);
  for(auto& module_path : fs::directory_iterator(fs::path("modules")))
    bot.load_module(module_path);
  bot.run();
  std::cin.get();

  return 0;
}
