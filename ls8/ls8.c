#include <stdio.h>
#include "cpu.h"

/**
 * Main
 */
int main(int argc, char *argv[])
{
  struct cpu cpu;

  cpu_init(&cpu);

  if (argc != 2)
  {
    fprintf(stderr, "usage: ./ls8 filename\n");
    return 1;
  }

  cpu_load(&cpu, argv[1]);
  cpu_run(&cpu);

  return 0;
}