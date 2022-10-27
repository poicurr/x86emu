#include <Emulator.hpp>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: px86 filename" << std::endl;
    return 1;
  }
  Emulator emu(1024, 0x0000, 0x7c00);
  std::ifstream file(argv[1], std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "[error] failed to open file" << std::endl;
    return 1;
  }

  file.read(emu.memory, sizeof(uint8_t) * 1024);
}
