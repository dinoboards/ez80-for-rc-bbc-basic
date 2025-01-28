#include "ram.h"
#include "vdu.h"
#include <stdbool.h>
#include <stdio.h>

static const uint8_t test_values[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

bool test_at(volatile uint8_t *ptr) {
  const uint8_t *values = test_values;
  for (int i = 0; i < 16; i++)
    *ptr++ = *values++;

  ptr--;
  values--;

  for (int i = 0; i < 16; i++)
    if (*ptr-- != *values--)
      return false;

  return true;
}

#define _512K  (uint8_t *)0x200000 + 512 * 1024
#define _1024K (uint8_t *)0x200000 + 1024 * 1024
#define _1536K (uint8_t *)0x200000 + 1536 * 1024
#define _2048K (uint8_t *)0x200000 + 2048 * 1024

void osinit() {
  init_font_patterns();

  LOMEM = (uint8_t *)(((uint24_t)_heap + 255) & ~255);
  printf("LOMEM: %p\r\n", LOMEM);

  HIMEM = _512K; // assume at least 512K

  if (!test_at(_512K))
    return;

  HIMEM = _1024K; // assume at least 1M

  if (!test_at(_1024K))
    return;

  HIMEM = _1536K; // assume at least 1.5M

  if (!test_at(_1536K))
    return;

  HIMEM = _2048K; // assume at 2MB

  printf("HIMEM: %p\r\n", HIMEM);
  printf("%d bytes available\r\n", HIMEM - LOMEM);
}
