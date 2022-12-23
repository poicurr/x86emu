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

  FILE* binary = fopen(argv[1], "rb");
  if (binary == NULL) {
    printf("%s: failed to open file\n", argv[1]);
    return 1;
  }

  fread(emu.memory + 0x7c00, 1, 0x200, binary);
  fclose(binary);

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
