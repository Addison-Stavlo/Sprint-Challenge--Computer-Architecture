#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *file)
{
  FILE *fp;
  fp = fopen(file, "r");

  if (fp == NULL)
  {
    fprintf(stderr, "comp: error opening file\n");
    exit(2);
  }

  int address = 0;
  char line[256];

  while (fgets(line, 256, fp) != NULL)
  {
    char *endptr;
    unsigned int val = strtoul(line, &endptr, 2);
    if (endptr == line)
    {
      // no digits found.. goto next read line
      continue;
    }
    cpu->ram[address++] = val;
  }

  fclose(fp);
}

unsigned int cpu_ram_read(struct cpu *cpu, unsigned int pc)
{
  return cpu->ram[pc];
}

void cpu_ram_write(struct cpu *cpu, unsigned int instruction, unsigned int pc)
{
  cpu->ram[pc] = instruction;
}
/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    cpu->reg[regA] = cpu->reg[regA] * cpu->reg[regB];
    break;
  case ALU_ADD:
    cpu->reg[regA] = cpu->reg[regA] + cpu->reg[regB];
    break;
    // TODO: implement more ALU ops
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  int FLAG = 0b00000000;

  while (running)
  {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    int IR = cpu_ram_read(cpu, cpu->pc); // instruction register
    // 2. Figure out how many operands this next instruction requires
    int numOps = IR >> 6;

    // 3. Get the appropriate value(s) of the operands following this instruction
    int ops[4];
    for (int i = 0; i < numOps; i++)
    {
      ops[i] = cpu_ram_read(cpu, cpu->pc + 1 + i);
    }

    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.

    switch (IR)
    {
    case LDI:
      //sets value of register to an int
      cpu->reg[ops[0]] = ops[1];
      break;

    case PRN:
      printf("%d\n", cpu->reg[ops[0]]);
      break;

    case HLT:
      running = 0;
      break;

    case MUL:
      alu(cpu, ALU_MUL, ops[0], ops[1]);
      break;

    case ADD:
      alu(cpu, ALU_ADD, ops[0], ops[1]);
      break;
    case PUSH:
      // reg[7] is stack pointer
      // start of stack at RAM[0xF3]
      if (cpu->reg[7] == 0)
      {
        cpu->reg[7] = 0xF4;
      }
      cpu->reg[7]--;
      cpu->ram[cpu->reg[7]] = cpu->reg[ops[0]];

      break;

    case POP:
      if (cpu->reg[7] == 0 || cpu->reg[7] == 0xF4)
      {
        fprintf(stderr, "error: no items in stack!\n");
        break;
      }
      cpu->reg[ops[0]] = cpu->ram[cpu->reg[7]];
      cpu->reg[7]++;
      break;

    case CALL:
      // NOT DRY - modified copy/paste'd PUSH code
      if (cpu->reg[7] == 0)
      {
        cpu->reg[7] = 0xF4;
      }
      cpu->reg[7]--;
      cpu->ram[cpu->reg[7]] = cpu->pc + 2;

      cpu->pc = cpu->reg[ops[0]] - 2;
      break;

    case RET:
      // NOT DRY - some modified copy/paste'd POP code
      cpu->pc = cpu->ram[cpu->reg[7]];
      cpu->reg[7]++;
      cpu->pc -= 1 + numOps;
      break;

    case CMP:
      // compare and set flag
      if (cpu->reg[ops[0]] == cpu->reg[ops[1]])
      {
        FLAG = 0b00000000;
      }
      else if (cpu->reg[ops[0]] > cpu->reg[ops[1]])
      {
        FLAG = 0b00000010;
      }
      else
      {
        FLAG = 0b00000100;
      }
      break;

    case JEQ:
      // 0 -> equal values flag
      if (FLAG == 0)
      {
        cpu->pc = cpu->reg[ops[0]];
        cpu->pc -= 1 + numOps;
      }
      break;

    case JNE:
      if (FLAG != 0)
      {
        cpu->pc = cpu->reg[ops[0]];
        cpu->pc -= 1 + numOps;
      }
      break;

    case JMP:
      cpu->pc = cpu->reg[ops[0]];
      cpu->pc -= 1 + numOps;
      break;

    default:
      printf("Unknown instruction at %d: %d\n", cpu->pc, IR);
      exit(1);
    }

    // 6. Move the PC to the next instruction.
    cpu->pc += 1 + numOps;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->pc = 0;
  memset(cpu->reg, 0, 8 * (sizeof(char)));
  memset(cpu->ram, 0, 128 * sizeof(char));
}
