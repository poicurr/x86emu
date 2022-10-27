#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

static const size_t REGISTER_COUNT = 24;
static const size_t ESP = 0;

struct Emulator {
  Emulator(size_t size, uint32_t eip, uint32_t esp) {
    memset(registers, 0, sizeof(registers));
    memory = new char[size];
    eip = eip;
    registers[ESP] = esp;
  }

  virtual ~Emulator() { delete memory; }

  uint32_t registers[REGISTER_COUNT];
  uint32_t eflags;
  char* memory;
  uint32_t eip;
};
