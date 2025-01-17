#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern uint24_t himem;
extern uint8_t ACCS[256];
extern uint8_t OPTVAL;
extern uint8_t* TABLE[8];

void debug() {
  printf("HIMEM: %X\r\n", himem);
  // printf("TABLE: %p\r\n", TABLE);
  // for(int i = 0; i < 8; i++) {
  //   printf("  TABLE[%d]: %p\r\n", i, TABLE[i]);
  // }
}

#define ABORT_X(name) void abort_##name(uint24_t af, uint24_t bc, uint24_t de, uint8_t *hl, uint24_t ix) { \
  printf("Abort " #name ".  AF:%X, BC: %X, DE: %X, HL: %p (*HL): %x, ix: %x\r\n", af, bc, de, hl, *hl, ix); \
  abort();\
}

ABORT_X(read)
ABORT_X(write)
ABORT_X(exists)
ABORT_X(dir1)
ABORT_X(x1)


void abort_bdos1(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort BDOS1.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
  abort();
}

void abort_bdos16(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort BDOS16.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
  abort();
}

void abort_osshut(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort OSSHUT.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
  abort();
}

void abort_seshut(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort SESHUT.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
  abort();
}

void abort_stload(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort STLOAD.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
  abort();
}

void abort_setdma(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl) {
  printf("Abort SETDMA.  AF:%X, BC: %X, DE: %X, HL: %X\r\n", af, bc, de, hl);
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
