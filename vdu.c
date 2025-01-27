#include "vdu.h"
#include "bbcbasic.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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

static rectangle_t gviewport              = {0, 0, scale_width - 1, scale_height - 1}; // inclusive or exclusive????
static point_t     current_gpos           = {0, 0};
static uint8_t     current_gfg_colour     = 0;
static uint8_t     current_operation_mode = 0;
static uint8_t     current_display_mode   = 255;
extern uint8_t     sysfont[];

static tpoint_t     current_tpos             = {0, 0};
static trectangle_t tviewport                = {0, 26, 63, 0};
static uint8_t      last_text_column         = 63;
static uint8_t      last_text_row            = 26;
static uint8_t      current_tbg_colour       = 0;
static uint8_t      current_tfg_colour       = 1;
static uint8_t      current_mode_colour_mask = 1;

#define MAX_VDP_BYTES 16
static uint8_t data[MAX_VDP_BYTES];
static uint8_t vdu_index           = 0;
static uint8_t vdu_required_length = 0;

typedef void (*mos_vdu_handler)();

static void vdu_set_tviewport();
static void vdu_set_origin();
static void vdu_mode();
static void vdu_set_gviewport();
static void vdu_plot();
static void vdu_gcol();
static void vdu_cls();
static void vdu_clg();
static void vdu_colour_define();
static void vdu_colour();
static bool line_clip(line_t *l);
static void preload_fonts();
static void mode_4_preload_fonts();
static void mode_5_preload_fonts();

static int16_t convert_x(int16_t logical_x) { return vdp_get_screen_width() * (logical_x + origin.x) / scale_width; }

static int16_t convert_y(int16_t logical_y) {
  return (vdp_get_screen_height() * (scale_height - (logical_y + origin.y))) / scale_height;
}

static void graphic_print_char(uint8_t ch);

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

  if (ch == 12) { // cls
    if (current_display_mode == 255)
      return 0x0C; // send formfeed to serial

    vdu_cls();
    return -1;
  }

  if (ch == 16) { // clg
    vdu_clg();
    return -1;
  }

  if (ch == 17) { // vdu_colour
    current_fn          = vdu_colour;
    vdu_required_length = 1;
    return -1;
  }

  if (ch == 18) { // gcol mode, colour
    current_fn          = vdu_gcol;
    vdu_required_length = 2;
    return -1;
  }

  if (ch == 19) { // colour
    current_fn          = vdu_colour_define;
    vdu_required_length = 5;
    return -1;
  }

  if (ch == 22) { // MODE
    current_fn          = vdu_mode;
    vdu_required_length = 1;
    return -1;
  }

  if (ch == 24) { // set g viewport
    current_fn          = vdu_set_gviewport;
    vdu_required_length = 8;
    return -1;
  }

  if (ch == 25) { // plot
    current_fn          = vdu_plot;
    vdu_required_length = 5;
    return -1;
  }

  if (ch == 28) { // set text viewport
    current_fn          = vdu_set_tviewport;
    vdu_required_length = 4;
    return -1;
  }

  if (ch == 29) { // set origin
    current_fn          = vdu_set_origin;
    vdu_required_length = 4;
    return -1;
  }

  if (current_display_mode == 255)
    return ch;

  graphic_print_char(ch);
  // print to graphic screen at current text post

  // for the time, lets dual output to serial and graphic
  return ch;
  // return -1;
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

/*
// VDU 12
VDU 12 clears either the current text viewport (by default or after a VDU 4
command) or the current graphics viewport (after a VDU 5 command) to the current
text or graphics background colour respectively. In addition the text or graphics
cursor is moved to its home position (see VDU 3)
*/

static void vdu_cls() {
  // for moment lets just erase to black
  // TODO constrain to text view port
  // apply correct back colour
  vdp_cmd_wait_completion();

  const uint16_t left   = (uint16_t)tviewport.left * 8;
  const uint16_t bottom = (uint16_t)tviewport.bottom * 8;
  const uint16_t right  = (uint16_t)tviewport.right * 8;
  const uint16_t top    = (uint16_t)tviewport.top * 8;

  const uint16_t width  = right - left + 8;
  const uint16_t height = bottom - top + 8;

  vdp_cmd_logical_move_vdp_to_vram(left, top, width, height, current_tbg_colour, 0, 0);

  current_tpos.x = tviewport.left;
  current_tpos.y = tviewport.top;
}

// VDU: 16 (0 bytes)
static void vdu_clg() {
  // for moment lets just erase to black
  // TODO: apply correct back colour

  int16_t        left   = convert_x(gviewport.left);
  int16_t        right  = convert_x(gviewport.right);
  int16_t        top    = convert_y(gviewport.top);
  int16_t        bottom = convert_y(gviewport.bottom);
  const uint16_t width  = right - left + 1;
  const uint16_t height = bottom - top + 1;

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vdp_to_vram(left, top, width, height, 0, 0, 0);

  current_gpos.x = 0;
  current_gpos.y = 0;
}

/*
VDU 17,n
VDU 17 sets either the text foreground (n<128) or background (n>=128) colours to
the value n. It is equivalent to COLOUR n
*/
static void vdu_colour() {
  if (data[0] >= 128)
    current_tbg_colour = data[0] & current_mode_colour_mask;
  else
    current_tfg_colour = data[0] & current_mode_colour_mask;

  preload_fonts();
}

// VDU: 18 (2 bytes)
static void vdu_gcol() {
  current_operation_mode = data[0];
  current_gfg_colour     = data[1];
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

// TODO currently assumes in 16 colour mode -make it work for all modes
static void vdu_colour_define() {
  const uint8_t l = data[0];
  const uint8_t p = data[1];

  if (p <= 15 && p >= 0) {
    const RGB physical_colour = default_16_colour_palette[p];

    vdp_reg_write(16, l & 15);
    vdp_out_pal((physical_colour.red & 7) * 16 + (physical_colour.blue & 7));
    vdp_out_pal(physical_colour.green & 7);
  }
}
// VDU: 22 (1 byte)

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

//    xx      | xx - serial I/O  MODE 255 (-1)

extern void vdp_set_graphic_4();

// VDU 22 This VDU code is used to change MODE. It is followed by one number which
// is the new mode. Thus VDU 22,7 is exactly equivalent to MODE 7 (except that
// it does not change HIMEM).

static void vdu_mode() {
  vdp_set_lines(212);
  current_display_mode = data[0];
  last_text_row        = 26;
  tviewport.left       = 0;
  tviewport.bottom     = 26;
  tviewport.top        = 0;

  switch (data[0]) {
  case 0:
    vdp_set_palette(default_2_colour_palette);
    current_tfg_colour       = 1;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 1;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    mode_5_preload_fonts();
    break;

  case 1:
    vdp_set_palette(default_4_colour_palette);
    current_tfg_colour       = 3;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 3;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    mode_5_preload_fonts();
    break;

  case 4:
    vdp_set_palette(default_2_colour_palette);
    current_tfg_colour       = 1;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 1;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    mode_5_preload_fonts();
    break;

  case 2:
    vdp_set_palette(default_16_colour_palette);
    current_tfg_colour       = 7;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 15;
    last_text_column         = 31;
    tviewport.right          = 31;
    vdp_set_graphic_4();
    mode_4_preload_fonts();
    break;

  case 5:
    vdp_set_palette(default_4_colour_palette);
    current_tfg_colour       = 3;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 3;
    last_text_column         = 31;
    tviewport.right          = 31;
    vdp_set_graphic_4();
    mode_4_preload_fonts();
    break;

  case 255:
    break;

  default:
    vdu_not_implemented();
  }

  vdu_cls();
}

/*VDU 24,x1;y1;x2;y2
VDU 24 defines a graphics viewport. The four parameters define the left, bottom,
right and top boundaries respectively, relative to the current graphics origin.
*/
static void vdu_set_gviewport() {
  uint8_t *p = (uint8_t *)&gviewport;
  *p++       = data[0];
  *p++       = data[1];
  *p++       = data[2];
  *p++       = data[3];
  *p++       = data[4];
  *p++       = data[5];
  *p++       = data[6];
  *p++       = data[7];

  if (gviewport.left < 0)
    gviewport.left = 0;
  if (gviewport.left > scale_width - 1)
    gviewport.left = scale_width - 1;

  if (gviewport.right < 0)
    gviewport.right = 0;
  if (gviewport.right > scale_width - 1)
    gviewport.right = scale_width - 1;

  if (gviewport.top < 0)
    gviewport.top = 0;
  if (gviewport.top > scale_height - 1)
    gviewport.top = scale_height - 1;

  if (gviewport.bottom < 0)
    gviewport.bottom = 0;
  if (gviewport.bottom > scale_height - 1)
    gviewport.bottom = scale_height - 1;
}

// VDU: 25 (5 bytes)

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
    uint8_t *const bptr_x = (uint8_t *)&current_gpos.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current_gpos.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];
    return;
  }

  case 5: {
    point_t previous_current = current_gpos;

    uint8_t *const bptr_x = (uint8_t *)&current_gpos.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current_gpos.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];

    // assume mode 7 and convert 6 bit RGB to G(3)R(3)B(2)

    line_t  l          = {previous_current, current_gpos};
    uint8_t intersects = line_clip(&l);

    if (intersects) {
      // printf("clipped: (%d, %d)-(%d,%d)\r\n", l.a.x, l.a.y, l.b.x, l.b.y);
      // printf("convert: (%d, %d) to (%d, %d) in (%d, %d)", convert_x(l.a.x), convert_y(l.a.y), convert_x(l.b.x), convert_y(l.b.y),
      //        current_gfg_colour, current_operation_mode);

      vdp_draw_line(convert_x(l.a.x), convert_y(l.a.y), convert_x(l.b.x), convert_y(l.b.y), current_gfg_colour,
                    current_operation_mode);
    }
    //  else
    //   printf("line outside of viewport\r\n");

    return;
  }

  case 69: {
    uint8_t *const bptr_x = (uint8_t *)&current_gpos.x;
    bptr_x[0]             = data[1];
    bptr_x[1]             = data[2];

    uint8_t *const bptr_y = (uint8_t *)&current_gpos.y;
    bptr_y[0]             = data[3];
    bptr_y[1]             = data[4];

    vdp_cmd_wait_completion();
    vdp_cmd_pset(convert_x(current_gpos.x), convert_y(current_gpos.y), current_gfg_colour, current_operation_mode);

    return;
  }

  default:
    vdu_not_implemented();
  }
}

// VDU 28,lx,by,rx,ty
// VDU 28 defines a text viewport. The parameters specify the boundary of the
// viewport; the left-most column, the bottom row, the right-most column and the top
// row respectively.
static void vdu_set_tviewport() {
  uint8_t *p = (uint8_t *)&tviewport;
  *p++       = data[0];
  *p++       = data[1];
  *p++       = data[2];
  *p++       = data[3];

  if (tviewport.left < 0)
    tviewport.left = 0;
  if (tviewport.left > last_text_column)
    tviewport.left = last_text_column;

  if (tviewport.right < 0)
    tviewport.right = 0;
  if (tviewport.right > last_text_column)
    tviewport.right = last_text_column;

  if (tviewport.top < 0)
    tviewport.top = 0;
  if (tviewport.top > last_text_row)
    tviewport.top = last_text_row;

  if (tviewport.bottom < 0)
    tviewport.bottom = 0;
  if (tviewport.bottom > last_text_row)
    tviewport.bottom = last_text_row;

  current_tpos.x = tviewport.left;
  current_tpos.y = tviewport.top;
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

  uint8_t codeA = bit_code(l->a);
  uint8_t codeB = bit_code(l->b);

  while (true) {
    if (!(codeA | codeB)) // both points within viewport
      return true;

    if (codeA & codeB) // line does not intersect viewport
      return false;

    if (codeA) { // a outside, intersect with clip edge
      l->a  = intersect(*l, codeA);
      codeA = bit_code(l->a);
    }

    if (codeB) { // b outside, intersect with clip edge
      l->b  = intersect(*l, codeB);
      codeB = bit_code(l->b);
    }
  }

  return true;
}

static point_t intersect(line_t l, uint8_t edge) {
  return edge & 8   ? (point_t){l.a.x + (l.b.x - l.a.x) * (gviewport.top - l.a.y) / (l.b.y - l.a.y), gviewport.top}
         : edge & 4 ? (point_t){l.a.x + (l.b.x - l.a.x) * (gviewport.bottom - l.a.y) / (l.b.y - l.a.y), gviewport.bottom}
         : edge & 2 ? (point_t){gviewport.right, l.a.y + (l.b.y - l.a.y) * (gviewport.right - l.a.x) / (l.b.x - l.a.x)}
         : edge & 1 ? (point_t){gviewport.left, l.a.y + (l.b.y - l.a.y) * (gviewport.left - l.a.x) / (l.b.x - l.a.x)}
                    : (point_t){-1, -1}; // will this happen?
}

static uint8_t bit_code(point_t p) {
  uint8_t code = 0;

  if (p.x < gviewport.left)
    code |= 1; // left
  else if (p.x > gviewport.right)
    code |= 2; // right

  if (p.y < gviewport.bottom)
    code |= 4; // bottom
  else if (p.y > gviewport.top)
    code |= 8; // top

  return code;
}

static void vdu_cr() { current_tpos.x = tviewport.left; }

static void vdu_lf() {
  current_tpos.y++;

  if (current_tpos.y >= tviewport.bottom) {
    current_tpos.y--;

    uint16_t left   = (uint16_t)tviewport.left * 8;
    uint16_t top    = (uint16_t)tviewport.top * 8;
    uint16_t right  = (uint16_t)tviewport.right * 8;
    uint16_t bottom = (uint16_t)tviewport.bottom * 8;

    uint16_t width  = right - left + 8;
    uint16_t height = bottom - top;

    // TODO: ONLY SCROLL IF TEXT CURSOR IS ACTIVE

    vdp_cmd_wait_completion();
    vdp_cmd_move_vram_to_vram(left, top + 8, left, top, width, height, 0);
    vdp_cmd_wait_completion();
    vdp_cmd_vdp_to_vram(left, bottom - 8, width, 8, current_tbg_colour, 0);
  }
}

static void vdu_bs() {
  if (current_tpos.x > tviewport.left)
    current_tpos.x--;
}

static void mode_4_preload_fonts() {

  // erase all of the page, so we only need to write the on dots

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vdp_to_vram(0, 256, vdp_get_screen_width(), vdp_get_screen_height(), current_tbg_colour, 0, 0);

  uint8_t *p = sysfont;

  const uint8_t starting_ch = ' ';
  uint16_t      gpos_x      = (starting_ch % 32) * 8;
  uint16_t      gpos_y      = 256 + (starting_ch / 32) * 8;

  for (int ch_row = ' '; ch_row < 127; ch_row += 32) {
    for (int ch_col = 0; ch_col < 32; ch_col++) {
      for (int y = 0; y < 8; y++) {
        uint8_t r = *p++;
        for (int x = 0; x < 8; x++) {
          const bool pixel_on = (r & (1 << (7 - x)));
          if (pixel_on) {
            vdp_cmd_wait_completion();
            vdp_cmd_pset(gpos_x, gpos_y, current_tfg_colour, 0);
          }
          gpos_x++;
        }

        gpos_x -= 8;
        gpos_y++;
      }

      gpos_x += 8;
      gpos_y -= 8;
    }
    gpos_x = 0;
    gpos_y += 8;
  }
}

static void mode_5_preload_fonts() { mode_4_preload_fonts(); }

static void preload_fonts() {
  switch (current_display_mode) {
  case 0:
  case 1:
  case 4:
    mode_5_preload_fonts();
    return;

  case 2:
  case 5:
    mode_4_preload_fonts();
    return;

  default:
    printf("todo: preload fonts\r\n");
  }
}

static void graphic_print_char(uint8_t ch) {

  // calculate real physical location to begin printing;

  if (ch == '\r') {
    vdu_cr();
    return;
  }

  if (ch == 8) {
    vdu_bs();
    return;
  }

  if (ch == '\n') {
    vdu_lf();
    return;
  }

  if ((ch < ' ') || (ch >= 127)) {
    // printf("TODO: process char 0x'%X'\r\n", ch);
    return;
  }

  const point_t gpos = (point_t){(int16_t)current_tpos.x * 8, (int16_t)current_tpos.y * 8};

  // calculate x and y of 'ch'
  const uint16_t from_x = (ch % 32) * 8;
  const uint16_t from_y = 256 + (ch / 32) * 8;

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vram_to_vram(from_x, from_y, gpos.x, gpos.y, 8, 8, 0, 0);

  current_tpos.x++;
  if (current_tpos.x > tviewport.right) {
    vdu_cr();
    vdu_lf();
  }
}
