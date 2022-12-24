#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <modrm.hpp>
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

  uint32_t get_register32(int index) { return registers[index]; }
  void set_register32(int index, uint32_t value) { registers[index] = value; }

  uint8_t get_register8(int index) {
    if (index < 4) {
      return registers[index] & 0xff;
    } else {
      return (registers[index - 4] >> 8) & 0xff;
    }
  }

  void set_register8(int index, uint8_t value) {
    if (index < 4) {
      uint32_t r = registers[index] & 0xffffff00;
      registers[index] = r | (uint32_t)value;
    } else {
      uint32_t r = registers[index - 4] & 0xffff00ff;
      registers[index - 4] = r | ((int32_t)value << 8);
    }
  }

  uint32_t get_memory8(uint32_t address) { return memory[address]; }
  uint32_t get_memory32(uint32_t address) {
    uint32_t ret = 0;
    for (int i = 0; i < 4; ++i) ret |= get_memory8(address + i) << (8 * i);
    return ret;
  }

  void set_rm32(ModRM* modrm, uint32_t value) {
    if (modrm->mod == 3) {
      set_register32(modrm->rm, value);
    } else {
      uint32_t address = calc_memory_address(modrm);
      set_memory32(address, value);
    }
  }

  uint32_t get_rm32(ModRM* modrm) {
    if (modrm->mod == 3) {
      return get_register32(modrm->rm);
    } else {
      uint32_t address = calc_memory_address(modrm);
      return get_memory32(address);
    }
  }

  void set_rm8(ModRM* modrm, uint8_t value) {
    if (modrm->mod == 3) {
      set_register8(modrm->rm, value);
    } else {
      uint32_t address = calc_memory_address(modrm);
      set_memory8(address, value);
    }
  }

  uint8_t get_rm8(ModRM* modrm) {
    if (modrm->mod == 3) {
      return get_register8(modrm->rm);
    } else {
      uint32_t address = calc_memory_address(modrm);
      return get_memory8(address);
    }
  }

  void set_r8(ModRM* modrm, uint8_t value) {
    set_register8(modrm->reg_index, value);
  }

  void set_r32(ModRM* modrm, uint32_t value) {
    set_register32(modrm->reg_index, value);
  }

  uint8_t get_r8(ModRM* modrm) { return get_register8(modrm->reg_index); }

  uint32_t get_r32(ModRM* modrm) { return get_register32(modrm->reg_index); }

  void set_memory8(uint32_t address, uint32_t value) {
    memory[address] = value & 0xFF;
  }

  void set_memory32(uint32_t address, uint32_t value) {
    for (int i = 0; i < 4; ++i) set_memory8(address + i, value >> (i * 8));
  }

  uint32_t calc_memory_address(ModRM* modrm) {
    if (modrm->mod == 0) {
      if (modrm->rm == 4) {
        printf("not implemented");
        exit(0);
      } else if (modrm->rm == 5) {
        return modrm->disp32;
      } else {
        return get_register32(modrm->rm);
      }
    } else if (modrm->mod == 1) {
      if (modrm->rm == 4) {
        printf("not implemented");
        exit(0);
      } else {
        return get_register32(modrm->rm) + modrm->disp8;
      }
    } else if (modrm->mod == 2) {
      if (modrm->rm == 4) {
        printf("not implemented");
        exit(0);
      } else {
        return get_register32(modrm->rm) + modrm->disp32;
      }
    } else {
      printf("not implemented");
      exit(0);
    }
  }

  void mov_r32_imm32() {
    uint8_t reg = get_code8(0) - 0xB8;
    uint32_t value = get_code32(1);
    registers[reg] = value;
    eip += 5;
  }

  void mov_rm32_imm32() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);
    uint32_t value = get_code32(0);
    eip += 4;
    set_rm32(&modrm, value);
  }

  void mov_rm32_r32() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);
    uint32_t r32 = get_r32(&modrm);
    set_rm32(&modrm, r32);
  }

  void mov_r32_rm32() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);
    uint32_t rm32 = get_rm32(&modrm);
    set_r32(&modrm, rm32);
  }

  void add_rm32_r32() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);
    uint32_t r32 = get_r32(&modrm);
    uint32_t rm32 = get_rm32(&modrm);
    set_rm32(&modrm, rm32 + r32);
  }

  void sub_rm32_imm8(ModRM* modrm) {
    uint32_t rm32 = get_rm32(modrm);
    uint32_t imm8 = (int32_t)get_sign_code8(0);
    eip += 1;
    set_rm32(modrm, rm32 - imm8);
  }

  void code_83() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);

    switch (modrm.opecode) {
      case 5:
        sub_rm32_imm8(&modrm);
        break;
      default:
        printf("not implemented: 83 /%d\n", modrm.opecode);
        exit(1);
    }
  }

  void inc_rm32(ModRM* modrm) {
    uint32_t value = get_rm32(modrm);
    set_rm32(modrm, value + 1);
  }

  void code_ff() {
    eip += 1;
    ModRM modrm;
    parse_modrm(&modrm);

    switch (modrm.opecode) {
      case 0:
        inc_rm32(&modrm);
        break;
      default:
        printf("not implemented: FF /%d\n", modrm.opecode);
        exit(1);
    }
  }

  void short_jump() {
    int8_t diff = get_sign_code8(1);
    eip += (diff + 2);
  }

  void near_jump() {
    int32_t diff = get_sign_code32(1);
    eip += (diff + 5);
  }

  void parse_modrm(ModRM* modrm) {
    memset(modrm, 0, sizeof(ModRM));
    uint8_t code = get_code8(0);
    modrm->mod = ((code & 0xC0) >> 6);
    modrm->opecode = ((code & 0x38) >> 3);
    modrm->rm = code & 0x07;

    eip += 1;

    if (modrm->mod != 3 && modrm->rm == 4) {
      modrm->sib = get_code8(0);
      eip += 1;
    }

    if ((modrm->mod == 0 && modrm->rm == 5) || modrm->mod == 2) {
      modrm->disp32 = get_sign_code32(0);
      eip += 4;
    } else if (modrm->mod == 1) {
      modrm->disp8 = get_sign_code8(0);
      eip += 1;
    }
  }

  using instruction_func_t = std::function<void()>;
  std::vector<instruction_func_t> instructions;
  void init_instructions() {
    instructions.reserve(256);
    for (int i = 0; i < 8; ++i) {
      instructions[0xB8 + i] = [&]() { mov_r32_imm32(); };
    }
    instructions[0x01] = [&]() { add_rm32_r32(); };
    instructions[0x83] = [&]() { code_83(); };
    instructions[0x89] = [&]() { mov_rm32_r32(); };
    instructions[0x8B] = [&]() { mov_r32_rm32(); };
    instructions[0xC7] = [&]() { mov_rm32_imm32(); };
    instructions[0xFF] = [&]() { code_ff(); };
    instructions[0xE9] = [&]() { near_jump(); };
    instructions[0xEB] = [&]() { short_jump(); };
  }

  uint32_t registers[REGISTERS_COUNT];
  uint8_t* memory;
  uint32_t eip;
  uint32_t eflags;
};
