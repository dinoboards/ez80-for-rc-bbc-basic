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

typedef struct rectangle {
  int16_t left;
  int16_t bottom;
  int16_t right;
  int16_t top;
} rectangle_t;

#endif
