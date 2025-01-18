#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern uint24_t himem;
extern uint8_t CPM_SYS_FCB[36];
extern uint8_t DISK_BUFFER[128];
extern uint8_t ACCS[256];
extern uint8_t OPTVAL;
extern uint8_t* TABLE[8];
extern uint8_t TRPCNT;
extern uint8_t RAM_END[];
// extern uint8_t *USER;
extern uint8_t end_of_bss[];
extern uint8_t _heap[];
extern uint32_t STAVAR[27];

uint24_t debug() {
  printf("HIMEM: %X\r\n", himem);
  printf("FCB: %p\r\n", CPM_SYS_FCB);
  printf("DISK_BUFFER: %p\r\n", DISK_BUFFER);
  printf("OPTVAL: %p, %x\r\n", &OPTVAL, OPTVAL);
  printf("TRPCNT: %p, %x\r\n", &TRPCNT, TRPCNT);
  printf("RAM_END: %p\r\n", RAM_END);
  // printf("USER: %p, %p\r\n", &USER, *USER);
  printf("end_of_bss: %p\r\n", end_of_bss);
  printf("_heap: %p\r\n", _heap);
  printf("STAVAR: %p, %lX, %lX, %lX\r\n", STAVAR, STAVAR[0], STAVAR[1], STAVAR[2]);

  uint24_t heap_aligned = ((uint24_t)_heap + 255) & ~255;
  printf("heap_aligned: %x\r\n", heap_aligned);


  printf("TABLE: %p\r\n", TABLE);
  for(int i = 0; i < 8; i++) {
    printf("  TABLE[%d]: %p\r\n", i, TABLE[i]);
  }

  return heap_aligned;
}

void log_info(const char* name, uint24_t af, uint24_t bc, uint24_t de, uint24_t hl, uint24_t ix) { \
  printf("log: %s.  AF:%X, BC: %X, DE: %X, HL: %X, ix: %X\r\n", name, af, bc, de, hl, ix); \
}

#define ABORT_X(name) void abort_##name(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl, uint24_t ix) { \
  printf("Abort " #name ".  AF:%X, BC: %X, DE: %X, HL: %X, ix: %X\r\n", af, bc, de, hl, ix); \
  abort();\
}

#define LOG_X(name) void log_##name(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl, uint24_t ix) { \
  printf("LOG: " #name ".  AF:%X, BC: %X, DE: %X, HL: %X, ix: %X\r\n", af, bc, de, hl, ix); \
}

ABORT_X(read)
ABORT_X(exists)
ABORT_X(dir1)

LOG_X(incsec)
LOG_X(read)
LOG_X(read1)
LOG_X(read2)
LOG_X(read3)

