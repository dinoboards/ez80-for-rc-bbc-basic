#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern uint24_t himem;
extern uint8_t ACCS[256];
extern uint8_t OPTVAL;

void debug() {
  printf("HIMEM: %X\r\n", himem);
}

void abort_bdos1() {
  printf("Abort BDOS1\r\n");
  abort();
}

void abort_setdma() {
  printf("Abort SETDMA\r\n");
  abort();
}

void abort_print1() {
  printf("Abort PRINT1\r\n");
  abort();
}

void abort_print4() {
  printf("Abort PRINT4\r\n");
  abort();
}

void abort_printc() {
  printf("Abort PRINT c\r\n");
  abort();
}

void abort_print3() {
  printf("Abort PRINT 3:  OPTVAL: %x, ACCS: %p, ACCS[0]: %d\r\n", OPTVAL, ACCS, ACCS[0]);
  abort();
}
