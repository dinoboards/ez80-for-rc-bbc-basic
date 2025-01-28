#ifndef __VDU_H
#define __VDU_H

#include <stdint.h>
#include <v99x8.h>
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

typedef struct tpoint {
  uint8_t x;
  uint8_t y;
} tpoint_t;

typedef struct trectangle {
  uint8_t left;
  uint8_t bottom;
  uint8_t right;
  uint8_t top;
} trectangle_t;

typedef struct triangle {
  point_t vt1;
  point_t vt2;
  point_t vt3;
} triangle_t;

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

typedef void (*mos_vdu_handler)();

extern void vdu_bs();
extern void vdu_lf();
extern void vdu_cls();
extern void vdu_cr();
extern void vdu_clg();
extern void vdu_colour();
extern void vdu_gcol();
extern void vdu_colour_define();
extern void vdu_mode();
extern void vdu_multi_purpose();
extern void vdu_set_gviewport();
extern void vdu_plot();
extern void vdu_set_tviewport();
extern void vdu_set_origin();

extern void        mode_4_preload_fonts();
extern void        preload_fonts();
static inline void mode_5_preload_fonts() { mode_4_preload_fonts(); }

extern int16_t convert_x(int16_t logical_x);
extern int16_t convert_y(int16_t logical_y);
extern point_t convert_point(const point_t p);

// vdu variables

extern point_t origin;
extern RGB     default_2_colour_palette[16];
extern RGB     default_4_colour_palette[16];
extern RGB     default_16_colour_palette[16];

extern const int16_t scale_width;
extern const int16_t scale_height;

extern rectangle_t gviewport;
extern rectangle_t gsviewport;
extern point_t     current_gpos;
extern point_t     previous_gpos;
extern uint8_t     current_gfg_colour;
extern uint8_t     current_operation_mode;
extern uint8_t     current_display_mode;
extern uint8_t     sysfont[];

extern tpoint_t     current_tpos;
extern trectangle_t tviewport;
extern uint8_t      last_text_column;
extern uint8_t      last_text_row;
extern uint8_t      current_tbg_colour;
extern uint8_t      current_tfg_colour;
extern uint8_t      current_mode_colour_mask;

#define MAX_VDP_BYTES 16
extern uint8_t data[MAX_VDP_BYTES];
extern uint8_t vdu_index;
extern uint8_t vdu_required_length;

extern mos_vdu_handler current_fn;

#endif
