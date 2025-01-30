#include "vdu.h"
#include "bbcbasic.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <v99x8.h>

#include <stdio.h>

static void graphic_print_char(uint8_t ch);

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

  if (ch == 8) {
    vdu_bs();
    return ch;
  }

  if (ch == '\n') {
    vdu_lf();
    return ch;
  }

  if (ch == 12) { // cls
    if (current_display_mode != 255)
      vdu_cls();

    return 0x0C; // send formfeed to serial
  }

  if (ch == '\r') {
    vdu_cr();
    return ch;
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

  if (ch == 23) { // multi purpose
    current_fn          = vdu_multi_purpose;
    vdu_required_length = 9;
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

  if (ch == 31) {
    current_fn          = vdu_tab;
    vdu_required_length = 2;
    return -1;
  }

  if (current_display_mode == 255)
    return ch;

  graphic_print_char(ch);
  // print to graphic screen at current text post

  // for the time, lets dual output to serial and graphic
  if (ch <= 127)
    return ch;

  return -1;
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

static void graphic_print_char(uint8_t ch) {

  // calculate real physical location to begin printing;

  if ((ch < ' ')) { // TODO: WE NEED TO RENDER OUT BITMAPS FOR ALL CHARS AFTER ' '
    // printf("TODO: process char 0x'%X'\r\n", ch);
    return;
  }

  const point_t gpos = (point_t){(int16_t)current_tpos.x * 8, (int16_t)current_tpos.y * 8};

  // calculate x and y of 'ch'
  const uint16_t from_x = (ch % 32) * 8;
  const uint16_t from_y = 256 + (ch / 32) * 8;

  prepare_font_pattern(ch, from_x, from_y);

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vram_to_vram(from_x, from_y, gpos.x, gpos.y, 8, 8, 0, 0);

  current_tpos.x++;
  if (current_tpos.x > tviewport.right) {
    vdu_cr();
    vdu_lf();
  }
}
