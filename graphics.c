#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <v99x8.h>

extern uint8_t *IY;

extern uint24_t expr_int24();
extern void     comma();
extern void     vdu_not_implemented() __attribute__((noreturn));

const uint24_t origin_x = 0;
const uint24_t origin_y = 0;

uint24_t current_x;
uint24_t current_y;

const uint24_t scale_width  = 1280;
const uint24_t scale_height = 1024;

RGB current_colour = {0};

uint24_t convert_x(uint24_t logical_x) { return vdp_get_screen_width() * logical_x / scale_width; }

uint24_t convert_y(uint24_t logical_y) { return vdp_get_screen_height() * (scale_height - logical_y) / scale_height; }

void move() {
  printf("iy is at %p.  iy[0]=%x, iy[1]=%x, iy[2]=%x\r\n", IY, IY[0], IY[1], IY[2]);

  uint24_t x1 = expr_int24();
  comma();
  uint24_t y1 = expr_int24();

  printf("(%d, %d)\r\n", convert_x(x1), convert_y(y1));

  current_x = x1;
  current_y = y1;
}

void draw() {
  printf("iy is at %p.  iy[0]=%x, iy[1]=%x, iy[2]=%x\r\n", IY, IY[0], IY[1], IY[2]);

  uint24_t x1 = expr_int24();
  comma();
  uint24_t y1 = expr_int24();

  printf("draw line from (%d, %d) to (%d, %d)", convert_x(current_x), convert_y(current_y), convert_x(x1), convert_y(y1));

  // assume mode 7 and convert 6 bit RGB to G(3)R(3)B(2)
  uint8_t colour = (current_colour.green >> 3) << 5 | (current_colour.red >> 3) << 2 | current_colour.blue >> 4;

  vdp_draw_line(convert_x(current_x), convert_y(current_y), convert_x(x1), convert_y(y1), colour, 0);

  current_x = x1;
  current_y = y1;
}

// GCOL mode,red,green,blue FOR MODE 7 ONLY - NO PALETTE

void gcol() {

  uint24_t mode = expr_int24();
  if (mode != 0) {
    printf("TODO to support modes other than 0\r\n");
  }
  comma();
  current_colour.red = expr_int24();
  comma();
  current_colour.green = expr_int24();
  comma();
  current_colour.blue = expr_int24();
}

void vdu() {

  uint24_t vdu_fn = expr_int24();

  switch (vdu_fn) {
  case 19: // palette
  {
    // VDU 19,logical,-1,r,g,b
    uint24_t logical = expr_int24();
    comma();
    int24_t physical = expr_int24();
    if (physical != -1)
      vdu_not_implemented();

    comma();
    uint24_t red = expr_int24();
    comma();
    uint24_t green = expr_int24();
    comma();
    uint24_t blue = expr_int24();

    printf("TODO: VDU 19,%d,%d,%d,%d,%d\r\n", logical, physical, red, green, blue);

    break;
  }

  default:
    vdu_not_implemented();
  }
}
