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

#define RGB_BLACK                                                                                                                  \
  (RGB) { 0, 0, 0 }
#define RGB_WHITE                                                                                                                  \
  (RGB) { 7, 7, 7 }
#define RGB_RED                                                                                                                    \
  (RGB) { 7, 0, 0 }
#define RGB_GREEN                                                                                                                  \
  (RGB) { 0, 7, 0 }
#define RGB_BLUE                                                                                                                   \
  (RGB) { 0, 0, 7 }
#define RGB_YELLOW                                                                                                                 \
  (RGB) { 7, 7, 0 }
#define RGB_MAGENTA                                                                                                                \
  (RGB) { 7, 0, 7 }
#define RGB_CYAN                                                                                                                   \
  (RGB) { 0, 7, 7 }
#define RGB_FLASHING_BLACK_WHITE                                                                                                   \
  (RGB) { 3, 3, 3 }
#define RGB_FLASHING_RED_CYAN                                                                                                      \
  (RGB) { 7, 3, 3 }
#define RGB_FLASHING_GREEN_MAGENTA                                                                                                 \
  (RGB) { 3, 7, 3 }
#define RGB_FLASHING_YELLOW_BLUE                                                                                                   \
  (RGB) { 3, 3, 7 }
#define RGB_FLASHING_BLUE_YELLOW                                                                                                   \
  (RGB) { 7, 7, 3 }
#define RGB_FLASHING_MAGENTA_GREEN                                                                                                 \
  (RGB) { 7, 3, 7 }
#define RGB_FLASHING_CYAN_RED                                                                                                      \
  (RGB) { 3, 7, 7 }
#define RGB_FLASHING_WHITE_BLACK                                                                                                   \
  (RGB) { 3, 3, 3 }

static RGB default_2_colour_palette[16]  = {RGB_BLACK, RGB_WHITE};
static RGB default_4_colour_palette[16]  = {RGB_BLACK, RGB_RED, RGB_YELLOW, RGB_WHITE};
static RGB default_16_colour_palette[16] = {RGB_BLACK,
                                            RGB_RED,
                                            RGB_GREEN,
                                            RGB_YELLOW,
                                            RGB_BLUE,
                                            RGB_MAGENTA,
                                            RGB_CYAN,
                                            RGB_WHITE,
                                            RGB_FLASHING_BLACK_WHITE,
                                            RGB_FLASHING_RED_CYAN,
                                            RGB_FLASHING_GREEN_MAGENTA,
                                            RGB_FLASHING_YELLOW_BLUE,
                                            RGB_FLASHING_BLUE_YELLOW,
                                            RGB_FLASHING_MAGENTA_GREEN,
                                            RGB_FLASHING_CYAN_RED,
                                            RGB_FLASHING_WHITE_BLACK};

static const int16_t scale_width  = 1280;
static const int16_t scale_height = 1024;

static int16_t viewport_left          = 0;
static int16_t viewport_bottom        = 0;
static int16_t viewport_right         = scale_width - 1; // inclusive or exclusive????
static int16_t viewport_top           = scale_height - 1;
static point_t current                = {0, 0};
static uint8_t current_fg_colour      = 0;
static uint8_t current_operation_mode = 0;

#define MAX_VDP_BYTES 16
static uint8_t data[MAX_VDP_BYTES];
static uint8_t vdu_index           = 0;
static uint8_t vdu_required_length = 0;

typedef void (*mos_vdu_handler)();

static void vdu_set_origin();
static void vdu_mode();
static void vdu_plot();
static void vdu_gcol();
static void vdu_clg();
static void vdu_colour();
static bool line_clip(line_t *l);

static int16_t convert_x(int16_t logical_x) { return vdp_get_screen_width() * (logical_x + origin.x) / scale_width; }

static int16_t convert_y(int16_t logical_y) {
  return (vdp_get_screen_height() * ((scale_height - 1) - (logical_y + origin.y))) / scale_height;
}

mos_vdu_handler current_fn = NULL;

// VDU Code	Ctrl plus	Extra bytes	Meaning
// 0	2 or @	0	Do nothing
// 1	A	1	Send next character to printer only
// 2	B	0	Enable printer
// 3	C	0	Disable printer
// 4	D	0	Write text at text cursor
// 5	E	0	Write text at graphics cursor
// 6	F	0	Enable VDU driver
// 7	G	0	Generate bell sound
// 8	H	0	Move cursor back one character
// 9	I	0	Move cursor on one space
// 10	J	0	Move cursor down one line
// 11	K	0	Move cursor up one line
// 12	L	0	Clear text viewport
// 13	M	0	Move cursor to start of current line
// 14	N	0	Turn on page mode
// 15	O	0	Turn off page mode
// 16	P	0	Clear graphics viewport
// 17	Q	1	Define text colour
// 18	R	2	Define graphics colour
// 19	S	5	Define logical colour
// 20	T	0	Restore default logical colours
// 21	U	0	Disable VDU drivers
// 22	V	1	Select screen mode
// 23	W	9	Multi-purpose command
// 24	X	8	Define graphics viewport
// 25	Y	5	PLOT
// 26	Z	0	Restore default viewports
// 27	[	0	Does nothing
// 28	\	4	Define text viewport
// 29	]	4	Define graphics origin
// 30	6 or ^	0	Home text cursor
// 31	- or _	2	Move text cursor

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

  if (ch == 18) { // gcol mode, colour
    current_fn          = vdu_gcol;
    vdu_required_length = 2;
    return -1;
  }

  if (ch == 19) { // colour
    current_fn          = vdu_colour;
    vdu_required_length = 5;
    return -1;
  }

  if (ch == 23) { // MODE
    current_fn          = vdu_mode;
    vdu_required_length = 1;
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

    if (separator == ':') {
      oswrite(value & 255);
      return;
    }

    error_syntax_error();
  }
}

// VDU: 16 (0 bytes)
static void vdu_clg() {
  // for moment lets just erase to black
  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vdp_to_vram(0, 0, vdp_get_screen_width(), vdp_get_screen_height(), 0, 0, 0);
}

// VDU: 18 (2 bytes)
static void vdu_gcol() {
  current_operation_mode = data[0];
  current_fg_colour      = data[1];
}

// VDU 19,l,p,r,g,b
// VDU 19 is used to define the physical colours associated with the logical colour l.
// If p <= 15 & p >= 0, r, g and b are ignored, and one of the standard colour settings is
// used. This is equivalent to COLOUR l,p.
// If p = 16, the palette is set up to contain the levels of red, green and blue dictated
// by r, g and b. This is equivalent to COLOUR l,r,g,b.
// If p = 24, the border is given colour components according to r, g and b.
// If p = 25, the mouse logical colour l is given colour components according to r, g
// and b. This is equivalent to MOUSE COLOUR l,r,g,b.

static void vdu_colour() {
  const uint8_t l = data[0];
  const uint8_t p = data[1];

  if (p <= 15 && p >= 0) {
    const RGB physical_colour = default_16_colour_palette[p];

    vdp_reg_write(16, l & 15);
    vdp_out_pal((physical_colour.red & 7) * 16 + (physical_colour.blue & 7));
    vdp_out_pal(physical_colour.green & 7);
  }
}
// VDU: 23 (1 byte)

// MODE 0: 640x256 graphics, 80x32 characters, 2 colours, 20kB RAM
// MODE 1: 320x256 graphics, 40x32 characters, 4 colours, 20kB RAM
// MODE 2: 160x256 graphics, 20x32 characters, 16 colours, 20kB RAM
// MODE 3: no graphics, 80x25 characters, 2 colours, 16kB RAM
// MODE 4: 320x256 graphics, 40x32 characters, 2 colours, 10kB RAM
// MODE 5: 160x256 graphics, 20x32 characters, 4 colours, 10kB RAM
// MODE 6: no graphics, 40x25 characters, 2 colours, 8kB RAM
// MODE 7: teletext, 40x25 characters, 8 colours, 1kB RAM

// BBC MODE   | V9958 MODE
//    xx      | MULTI-COLOR   64x48 16 colours                            8,9
//    xx      | G1            32*8x24*8 16 colours only 256 patterns      10,11
//    xx      | G3            32*8x24*8 16 colours 768 patterns           12
//    xx      | G4 (212)      256x212 16 colours    => 2,5                13
//    xx      | G4 (192)      256x192 16 colours                          15
//    xx      | G5 (212)      512x212 4 colours     => 0,1,4              16
//    xx      | G5 (192)      512x192 4 colours                           17
//    xx      | G6 (212)      512x212 16 colours                          18
//    xx      | G6 (192)      512x192 16 colours                          19
//    xx      | G7 (212)      256x212 256 colours                         20
//    xx      | G7 (192)      256x192 256 colours                         21

extern void vdp_set_graphic_4();

static void vdu_mode() {
  vdp_set_lines(212);

  switch (data[0]) {
  case 0:
    vdp_set_palette(default_2_colour_palette);
    vdp_set_graphic_5();
    break;

  case 1:
    vdp_set_palette(default_4_colour_palette);
    vdp_set_graphic_5();
    break;

  case 4:
    vdp_set_palette(default_2_colour_palette);
    vdp_set_graphic_5();
    break;

  case 2:
    vdp_set_palette(default_16_colour_palette);
    vdp_set_graphic_4();
    break;

  case 5:
    vdp_set_palette(default_4_colour_palette);
    vdp_set_graphic_4();
    break;

  default:
    vdu_not_implemented();
  }
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
8-63	Enhanced line drawing modes.
64-71	Plot a single point.
72-79	Horizontal line fill to non-background.
80-87	Plot and fill a triangle.
88-95	Horizontal line fill to background right.
96-103	Plot and fill an axis-aligned rectangle.
104-111	Horizontal line fill to foreground.
112-119	Plot and fill a parallelogram.
120-127	Horizontal line fill to non-foreground right.
128-135	Flood-fill to non-background.
136-143	Flood-fill to foreground.
144-151	Draw a circle.
152-159	Plot and fill a disc.
160-167	Draw a circular arc.
168-175	Plot and fill a segment.
176-183	Plot and fill a sector.
185/189	Move a rectangular block.
187/191	Copy a rectangular block.
192-199	Draw an outline axis-aligned ellipse.
200-207  	Plot and fill a solid axis-aligned ellipse.
249/253	Swap a rectangular block.

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

    // assume mode 7 and convert 6 bit RGB to G(3)R(3)B(2)

    line_t  l          = {previous_current, current};
    uint8_t intersects = line_clip(&l);

    if (intersects) {
      printf("clipped: (%d, %d)-(%d,%d)\r\n", l.a.x, l.a.y, l.b.x, l.b.y);
      printf("draw line from (%d, %d) to (%d, %d) in (%d, %d)", convert_x(l.a.x), convert_y(l.a.y), convert_x(l.b.x),
             convert_y(l.b.y), current_fg_colour, current_operation_mode);

      vdp_draw_line(convert_x(l.a.x), convert_y(l.a.y), convert_x(l.b.x), convert_y(l.b.y), current_fg_colour,
                    current_operation_mode);
    } else
      printf("line outside of viewport\r\n");

    return;
  }

  case 69: {
    uint8_t *const bptr_x = (uint8_t *)&current.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];

    vdp_cmd_wait_completion();
    vdp_cmd_pset(convert_x(current.x), convert_y(current.y), current_fg_colour, current_operation_mode);

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

/* line clipping to view port */

static uint8_t bit_code(point_t p);
static point_t intersect(line_t l, uint8_t edge);

static bool line_clip(line_t *l) {

  const uint8_t codeA = bit_code(l->a);
  const uint8_t codeB = bit_code(l->b);

  if (!(codeA | codeB)) // both points within viewport
    return true;

  if (codeA & codeB) // line does not intersect viewport
    return false;

  if (codeA) // a outside, intersect with clip edge
    l->a = intersect(*l, codeA);

  if (codeB) // b outside, intersect with clip edge
    l->b = intersect(*l, codeB);

  return true;
}

static point_t intersect(line_t l, uint8_t edge) {
  return edge & 8   ? (point_t){l.a.x + (l.b.x - l.a.x) * (viewport_top - l.a.y) / (l.b.y - l.a.y), viewport_top}
         : edge & 4 ? (point_t){l.a.x + (l.b.x - l.a.x) * (viewport_bottom - l.a.y) / (l.b.y - l.a.y), viewport_bottom}
         : edge & 2 ? (point_t){viewport_right, l.a.y + (l.b.y - l.a.y) * (viewport_right - l.a.x) / (l.b.x - l.a.x)}
         : edge & 1 ? (point_t){viewport_left, l.a.y + (l.b.y - l.a.y) * (viewport_left - l.a.x) / (l.b.x - l.a.x)}
                    : (point_t){-1, -1}; // will this happen?
}

static uint8_t bit_code(point_t p) {
  uint8_t code = 0;

  if (p.x < viewport_left)
    code |= 1; // left
  else if (p.x > viewport_right)
    code |= 2; // right

  if (p.y < viewport_bottom)
    code |= 4; // bottom
  else if (p.y > viewport_top)
    code |= 8; // top

  return code;
}
