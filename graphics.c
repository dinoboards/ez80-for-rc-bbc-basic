#include "bbcbasic.h"
#include "vdu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void move() {
  const uint24_t x1 = expr_int24();
  comma();
  const uint24_t y1 = expr_int24();

  const point_t p = {x1, y1};

  oswrite_int16(25 + (4 << 8));
  oswrite_point(&p);
}

void draw() {
  const uint24_t x1 = expr_int24();
  comma();
  const uint24_t y1 = expr_int24();

  const point_t p = {x1, y1};

  oswrite_int16(25 + (5 << 8));
  oswrite_point(&p);
}

// GCOL mode,red,green,blue FOR GR-MODE 7 ONLY - NO PALETTE
// GCOL mode,index - FOR ALL OTHER
void gcol() {
  uint24_t mode = expr_int24();
  if (mode != 0) {
    printf("TODO to support modes other than 0\r\n");
  }

  comma();

  uint8_t index = expr_int24();
  oswrite_int16(18 + (mode << 8));
  oswrite(index);

  // code for mode 7 scenario
  //  comma();
  //  uint8_t red = expr_int24();
  //  comma();
  //  uint8_t green = expr_int24();
  //  comma();
  //  uint8_t blue = expr_int24();

  // oswrite_int16(18 + (mode << 8));
  // uint8_t colour = (green >> 3) << 5 | (red >> 3) << 2 | blue >> 4;
  // oswrite(colour);
}
