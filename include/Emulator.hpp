#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

enum Register { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT };
const char* registers_name[] = {"EAX", "ECX", "EDX", "EBX",
                                "ESP", "EBP", "ESI", "EDI"};

struct Emulator {
  Emulator(size_t size, uint32_t eip, uint32_t esp) {
    memset(registers, 0, sizeof(registers));
    this->registers[ESP] = esp;
    this->memory = new uint8_t[size];
    this->eip = eip;
    this->eflags = 0;
    init_instructions();
  }
  virtual ~Emulator() { delete memory; }

  void dump_registers() {
    for (int i = 0; i < REGISTERS_COUNT; ++i) {
      printf("%s = %08x\n", registers_name[i], registers[i]);
    }
    printf("EIP = %08x\n", eip);
  }

  uint32_t get_code8(int index) { return memory[eip + index]; }

  int32_t get_sign_code8(int index) { return (int8_t)memory[eip + index]; }

  uint32_t get_code32(int index) {
    uint32_t ret = 0;
    for (int i = 0; i < 4; i++) ret |= get_code8(index + i) << (i * 8);
    return ret;
  }

  int32_t get_sign_code32(int index) { return (int32_t)get_code32(index); }

  void mov_r32_imm32() {
    uint8_t reg = get_code8(0) - 0xB8;
    uint32_t value = get_code32(1);
    registers[reg] = value;
    eip += 5;
  }

  void short_jump() {
    int8_t diff = get_sign_code8(1);
    eip += (diff + 2);
  }

  void near_jump() {
    int32_t diff = get_sign_code32(1);
    eip += (diff + 5);
  }

  using instruction_func_t = std::function<void()>;
  std::vector<instruction_func_t> instructions;
  void init_instructions() {
    instructions.reserve(256);
    for (int i = 0; i < 8; ++i) {
      instructions[0xB8 + i] = [&]() { mov_r32_imm32(); };
    }
    instructions[0xE9] = [&]() { near_jump(); };
    instructions[0xEB] = [&]() { short_jump(); };
  }

  uint32_t registers[REGISTERS_COUNT];
  uint8_t* memory;
  uint32_t eip;
  uint32_t eflags;
};
