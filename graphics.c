#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <v99x8.h>

extern uint8_t *IY;

extern uint24_t expr_int24();
extern void     comma();
extern void     vdu_not_implemented() __attribute__((noreturn));
extern void     error_arguments() __attribute__((noreturn));
extern void     error_syntax_error() __attribute__((noreturn));
extern uint8_t  nxt();

uint24_t origin_x = 0;
uint24_t origin_y = 0;

uint24_t current_x;
uint24_t current_y;

const uint24_t scale_width  = 1280;
const uint24_t scale_height = 1024;

RGB current_colour = {0};

#define MAX_VDP_BYTES 16
static uint8_t data[MAX_VDP_BYTES];

uint24_t convert_x(uint24_t logical_x) { return vdp_get_screen_width() * (logical_x + origin_x) / scale_width; }

uint24_t convert_y(uint24_t logical_y) { return vdp_get_screen_height() * (scale_height - (logical_y + origin_y)) / scale_height; }

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

uint8_t consume_numbers();
void    set_origin();

void vdu() {

  uint8_t count = consume_numbers();
  printf("Received %d bytes [0]=%d\r\n", count, data[0]);

  uint8_t x = data[0];
  printf("x = %d\r\n", x);
  switch (x) {

  // VDU 19,logical,-1,r,g,b
  case 19: // set palette
  {
    if (count != 6)
      vdu_not_implemented();

    uint8_t logical  = data[1];
    int8_t  physical = data[2];
    if (physical != -1)
      vdu_not_implemented();

    uint8_t red   = data[3];
    uint8_t green = data[4];
    uint8_t blue  = data[5];

    printf("TODO: VDU 19,%d,%d,%d,%d,%d\r\n", logical, physical, red, green, blue);

    break;
  }

  // VDU 29,640;512;
  // VDU 29,128,2,0,2
  case 29: // set origin
  {
    if (count != 5)
      error_arguments();

    uint8_t *const bptr_origin_x = (uint8_t *)&origin_x;
    bptr_origin_x[0]             = data[1];
    bptr_origin_x[1]             = data[2];
    bptr_origin_x[2]             = 0;

    uint8_t *const bptr_origin_y = (uint8_t *)&origin_y;
    bptr_origin_y[0]             = data[3];
    bptr_origin_y[1]             = data[4];
    bptr_origin_y[2]             = 0;

    break;
  }

  default:
    vdu_not_implemented();
  }
}

uint8_t consume_numbers() {
  // if number ends in a ';' then its a word
  // if comma or line end, its a byte

  int i = 0;

next:
  if (i >= MAX_VDP_BYTES)
    error_arguments();

  uint24_t value = expr_int24();

  uint8_t separator = nxt();

  if (separator == ',') {
    data[i] = value & 255;
    i++;
    IY++;
    goto next;
  }

  if (separator == ';') {
    data[i++] = value & 255;
    if (i >= MAX_VDP_BYTES)
      error_arguments();
    data[i++] = (value >> 8) & 255;
    IY++;
    // may be end of line???
    if (nxt() == '\r') {
      return i;
    }

    goto next;
  }

  if (separator == '\r') {
    data[i++] = value & 255;
    return i;
  }

  error_syntax_error();
}
