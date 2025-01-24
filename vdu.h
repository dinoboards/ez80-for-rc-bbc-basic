#ifndef __VDU_H
#define __VDU_H

#include <stdint.h>

typedef struct point {
  int16_t x;
  int16_t y;
} point_t;

typedef struct line {
  point_t a;
  point_t b;
} line_t;

#endif
