#include <string.h>

#include <Emulator.hpp>
#include <fstream>
#include <iostream>

static const size_t MEMORY_SIZE = 1024 * 1024;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: px86 filename" << std::endl;
    return 1;
  }
  Emulator emu(MEMORY_SIZE, 0x7c00, 0x7c00);

  std::ifstream ifs(argv[1], std::ios::binary);
  if (!ifs.is_open()) {
    printf("failed to open file: %s\n", argv[1]);
    return 1;
  }
  auto binary = std::string{};
  char buffer[1024];
  while (!ifs.eof()) {
    ifs.read(buffer, 1024);
    binary.append(buffer, ifs.gcount());
  }
  memcpy(emu.memory + 0x7c00, binary.data(), binary.size());

  while (emu.eip < MEMORY_SIZE) {
    uint8_t code = emu.get_code8(0);
    printf("EIP = %X, Code = %02X\n", emu.eip, code);
    emu.instructions[code]();
    if (emu.eip == 0x00) {
      printf("\n\nend of program.\n\n");
      break;
    }
  }

  emu.dump_registers();
}
