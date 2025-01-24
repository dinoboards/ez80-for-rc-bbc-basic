#include "vdu.h"
#include "bbcbasic.h"
#include <stdint.h>
#include <stdlib.h>
#include <v99x8.h>

#include <stdio.h>

// Any pixel between 0-1280 and 0-1024 will be mapped to actual physical pixels.
// logical scale is -32767 to 32767
// viewport is relative to current origin
//

point_t origin = {0, 0};

// static int16_t       viewport_left          = 0;
// static int16_t       viewport_bottom        = 0;
// static int16_t       viewport_right         = 1279; // inclusive or exclusive????
// static int16_t       viewport_top           = 1023;
static point_t current                = {0, 0};
static uint8_t current_fg_colour      = 0;
static uint8_t current_operation_mode = 0;

static const int16_t scale_width  = 1280;
static const int16_t scale_height = 1024;

#define MAX_VDP_BYTES 16
static uint8_t data[MAX_VDP_BYTES];
static uint8_t vdu_index           = 0;
static uint8_t vdu_required_length = 0;

typedef void (*mos_vdu_handler)();

static void vdu_set_origin();
static void vdu_plot();
static void vdu_gcol();
static void vdu_clg();

static int16_t convert_x(int16_t logical_x) { return vdp_get_screen_width() * (logical_x + origin.x) / scale_width; }

static int16_t convert_y(int16_t logical_y) {
  return vdp_get_screen_height() * (scale_height - (logical_y + origin.y)) / scale_height;
}

mos_vdu_handler current_fn = NULL;

uint24_t mos_oswrite(uint8_t ch) {
  if (vdu_required_length) {
    data[vdu_index++] = ch;
    if (vdu_index == vdu_required_length) {
      current_fn();

      current_fn          = NULL;
      vdu_index           = 0;
      vdu_required_length = 0;
    }
    return -1;
  }

  if (ch == 16) { // clg
    vdu_clg();
    return -1;
  }

  if (ch == 18) { // set mode, colour
    current_fn          = vdu_gcol;
    vdu_required_length = 2;
    return -1;
  }

  if (ch == 24) { // plot
    current_fn          = vdu_plot;
    vdu_required_length = 5;
    return -1;
  }

  if (ch == 29) { // set origin
    current_fn          = vdu_set_origin;
    vdu_required_length = 4;
    return -1;
  }

  return ch;
}

void vdu() {
  // if number ends in a ';' then its a word
  // if comma or line end, its a byte

  while (true) {
    uint24_t value = expr_int24();

    uint8_t separator = nxt();

    if (separator == ',') {
      oswrite(value & 255);
      IY++;
      continue;
    }

    if (separator == ';') {
      oswrite(value & 255);
      oswrite((value >> 8) & 255);
      IY++;
      // may be end of line???
      if (nxt() == '\r') {
        return;
      }

      continue;
    }

    if (separator == '\r') {
      oswrite(value & 255);
      return;
    }

    error_syntax_error();
  }
}

// VDU: 16 (0 bytes)
static void vdu_clg() {
  // for moment lets just erase to black
  vdp_cmd_logical_move_vdp_to_vram(0, 0, vdp_get_screen_width(), vdp_get_screen_height(), 0, 0, 0);
}

// VDU: 18 (2 bytes)
static void vdu_gcol() {
  current_operation_mode = data[0];
  current_fg_colour      = data[1];
}

// VDU: 24 (5 bytes)

/*
modes:
0	Move relative to the last point.
1	Draw a line, in the current graphics foreground colour, relative to the last point.
2	Draw a line, in the logical inverse colour, relative to the last point.
3	Draw a line, in the current graphics background colour, relative to the last point.
4	Move to the absolute position.
5	Draw a line, in the current graphics foreground colour, to the absolute coordinates specified by X and Y.
6	Draw a line, in the logical inverse colour, to the absolute coordinates specified by X and Y.
7	Draw a line, in the current graphics background colour, to the absolute coordinates specified by X and Y.
*/
static void vdu_plot() {

  switch (data[0]) { // mode
  case 4: {
    uint8_t *const bptr_x = (uint8_t *)&current.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];
    return;
  }

  case 5: {
    point_t previous_current = current;

    uint8_t *const bptr_x = (uint8_t *)&current.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];

    printf("draw line from (%d, %d) to (%d, %d)", convert_x(previous_current.x), convert_y(previous_current.y),
           convert_x(current.x), convert_y(current.y));

    // assume mode 7 and convert 6 bit RGB to G(3)R(3)B(2)

    vdp_draw_line(convert_x(previous_current.x), convert_y(previous_current.y), convert_x(current.x), convert_y(current.y),
                  current_fg_colour, current_operation_mode);

    return;
  }

  default:
    vdu_not_implemented();
  }
}

// VDU: 29 (4bytes)
static void vdu_set_origin() {
  uint8_t *const bptr_x = (uint8_t *)&origin.x;
  bptr_x[0]             = data[0];
  bptr_x[1]             = data[1];

  uint8_t *const bptr_y = (uint8_t *)&origin.y;
  bptr_y[0]             = data[2];
  bptr_y[1]             = data[3];
  current_fn            = NULL;
}
