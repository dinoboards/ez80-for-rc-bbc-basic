#ifndef __BBCBASIC_H
#define __BBCBASIC_H

#include "vdu.h"
#include <stdint.h>

extern uint8_t *IY;

extern uint24_t expr_int24();
extern void     comma();
extern void     vdu_not_implemented() __attribute__((noreturn));
extern void     error_arguments() __attribute__((noreturn));
extern void     error_syntax_error() __attribute__((noreturn));
extern uint8_t  nxt();
extern void     oswrite(uint8_t ch);
extern void     oswrite_int16(uint16_t d);
extern void     oswrite_point(const point_t *p);

#endif
