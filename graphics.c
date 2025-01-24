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

  oswrite_int16(24 + (4 << 8));
  oswrite_point(&p);
}

void draw() {
  const uint24_t x1 = expr_int24();
  comma();
  const uint24_t y1 = expr_int24();

  const point_t p = {x1, y1};

  oswrite_int16(24 + (5 << 8));
  oswrite_point(&p);
}

// GCOL mode,red,green,blue FOR MODE 7 ONLY - NO PALETTE

void gcol() {
  uint24_t mode = expr_int24();
  if (mode != 0) {
    printf("TODO to support modes other than 0\r\n");
  }
  comma();
  uint8_t red = expr_int24();
  comma();
  uint8_t green = expr_int24();
  comma();
  uint8_t blue = expr_int24();

  oswrite_int16(18 + (mode << 8));
  uint8_t colour = (green >> 3) << 5 | (red >> 3) << 2 | blue >> 4;
  oswrite(colour);
}

// void vdp_draw_line_clipped_by_viewport(int24_t from_x, int24_t from_y, int24_t to_x, int24_t to_y, uint8_t colour, uint8_t
// operation) {
//   if (from_x < viewport_left)
//     goto clip_it;

//   if (from_x > viewport_right)
//     goto clip_it;

//   if (to_x < viewport_left)
//     goto clip_it;

//   if (to_x > viewport_right)
//     goto clip_it

//   if (from_y < viewport_left) {

//   }
// }
// uint8_t consume_numbers();
// void    set_origin();

// void vdu() {
//   consume_numbers();
//   printf("Received %d bytes [0]=%d\r\n", count, data[0]);

//   uint8_t x = data[0];
//   printf("x = %d\r\n", x);
//   switch (x) {

//   // VDU 19,logical,-1,r,g,b
//   case 19: // set palette
//   {
//     if (count != 6)
//       vdu_not_implemented();

//     uint8_t logical  = data[1];
//     int8_t  physical = data[2];
//     if (physical != -1)
//       vdu_not_implemented();

//     uint8_t red   = data[3];
//     uint8_t green = data[4];
//     uint8_t blue  = data[5];

//     printf("TODO: VDU 19,%d,%d,%d,%d,%d\r\n", logical, physical, red, green, blue);

//     break;
//   }

//   // VDU 24,left;bottom;right;top;
//   case 24: // set graphics view port
//   {
//     if (count != 9)
//       error_arguments();

//     const uint16_t *ptr = (uint16_t *)&data[1];
//     viewport_left       = ptr[0];
//     viewport_bottom     = ptr[1];
//     viewport_right      = ptr[2];
//     viewport_top        = ptr[3];

//     break;
//   }

//   // VDU 29,640;512;
//   // VDU 29,128,2,0,2
//   case 29: // set origin
//   {
//     if (count != 5)
//       error_arguments();

//     break;
//   }

//   default:
//     vdu_not_implemented();
//   }
// }
