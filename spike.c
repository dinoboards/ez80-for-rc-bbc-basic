#include "ram.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <v99x8.h>

void debug() {
  printf("HIMEM: %p\r\n", HIMEM);
  printf("LOMEM: %p\r\n", LOMEM);
  printf("FREE: %X\r\n", FREE);
  printf("FCB: %p\r\n", CPM_SYS_FCB);
  printf("DISK_BUFFER: %p\r\n", DISK_BUFFER);
  printf("OPTVAL: %p, %x\r\n", &OPTVAL, OPTVAL);
  printf("TRPCNT: %p, %x\r\n", &TRPCNT, TRPCNT);
  printf("RAM_END: %p\r\n", RAM_END);
  printf("end_of_bss: %p\r\n", end_of_bss);
  printf("_heap: %p\r\n", _heap);
  printf("STAVAR: %p, %lX, %lX, %lX\r\n", STAVAR, STAVAR[0], STAVAR[1], STAVAR[2]);
  printf("FREE_FCB_TABLE: %X\r\n", FREE_FCB_TABLE);

  printf("TABLE: %p\r\n", TABLE);
  for (int i = 0; i < 8; i++) {
    printf("  TABLE[%d]: %p\r\n", i, TABLE[i]);
  }
}

void log_info(const char *name,
              uint24_t    af_,
              uint24_t    bc_,
              uint24_t    de_,
              uint24_t    hl_,
              uint24_t    af,
              uint24_t    bc,
              uint24_t    de,
              uint24_t    hl,
              uint24_t    ix,
              uint24_t    iy) {
  printf("log: %s.  AF:%X, BC: %X, DE: %X, HL: %X, AF':%X, BC': %X, DE': %X, "
         "HL': %X, ix: %X, iy: %X\r\n",
         name, af, bc, de, hl, af_, bc_, de_, hl_, ix, iy);
  // printf("FREE_FCB_TABLE: %X\r\n", FREE_FCB_TABLE);
  printf("ACCS(%p): %x, %x, %x\r\n", ACCS, ACCS[0], ACCS[1], ACCS[2]);
  printf("BUFFER(%p): %x, %x, %x\r\n", BUFFER, BUFFER[0], BUFFER[1], BUFFER[2]);
  // for (int i = 0; i < 8; i++) {
  //   printf("  TABLE[%d](%p): %p\r\n", i, &TABLE[i], TABLE[i]);
  // }

  uint8_t *p = (uint8_t *)ix;
  printf("*iX: %x %x\r\n", p[0], p[1]);

  p = (uint8_t *)iy;
  printf("*iy: %x %x\r\n", p[0], p[1]);

  p = (uint8_t *)hl;
  printf("*hl: %x %x\r\n", p[0], p[1]);
}

#define ABORT_X(name)                                                                                                              \
  void abort_##name(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl, uint24_t ix) {                                             \
    printf("Abort " #name ".  AF:%X, BC: %X, DE: %X, HL: %X, ix: %X\r\n", af, bc, de, hl, ix);                                     \
    abort();                                                                                                                       \
  }

#define LOG_X(name)                                                                                                                \
  void log_##name(uint24_t af, uint24_t bc, uint24_t de, uint24_t hl, uint24_t ix) {                                               \
    printf("LOG: " #name ".  AF:%X, BC: %X, DE: %X, HL: %X, ix: %X\r\n", af, bc, de, hl, ix);                                      \
  }

ABORT_X(exists)

extern uint8_t sysfont[];

// void star_zzz() {
//   printf("ready\r\n");

//   int xoffset = 0;
//   int yoffset = 0;
//   for (int c = 0; c < 768; c+=8) {
//     for (int y = 0; y < 8; y++) {
//       for (int x = 0; x < 7; x++) {
//         if (x==3) {
//           x++;
//           // continue;
//         }
//         else
//           pix_on = (sysfont[c + y] & (1 << (7 - x)))

//         if  {
//           printf("X");
//           vdp_cmd_wait_completion();
//           vdp_cmd_pset(x + xoffset, y + yoffset, 4, 0);
//         } else
//           printf(" ");
//       }
//       printf("\r\n----\r\n");
//     }

//     xoffset += 16;
//     if (xoffset >= 256) {
//       xoffset = 0;
//       yoffset += 16;
//     }
//     printf("-----------------\r\n");
//   }
// }
