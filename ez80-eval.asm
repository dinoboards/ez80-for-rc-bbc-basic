	.ASSUME	ADL = 1

	INCLUDE	"equs.inc"
	INCLUDE "macros.inc"

	section	.text, "ax", @progbits

	global EXPR_24BIT_INT

	; evaluate the expression stored at IY
	; and return it as a 24 bit value in HL

EXPR_24BIT_INT:
	CALL	EXPRN		; RESULT IN HLH'L' C SHOULD BE ZERO
	; U(HL) < l
	; H < H'
	; L < L'

	EXX
	LD	(conversion_store), HL
	EXX
	LD	A, L
	LD	(conversion_store + 2), A
	LD	HL, (conversion_store)
	RET



; extern uint24_t expr_int24(uint8_t** iy);

	global	_expr_int24

_expr_int24:
	push	ix
	ld	iy, (_IY)
	call	EXPR_24BIT_INT
	ld	(_IY), iy
	pop	ix
	ret


; extern void comma();

	global	_comma

_comma:
	push	ix
	ld	iy, (_IY)
	call	COMMA
	ld	(_IY), iy
	pop	ix
	ret

; extern uint8_t nxt(uint8_t** iy);

	global	_nxt

_nxt:
	push	ix
	ld	iy, (_IY)
	call	NXT
	ld	(_IY), iy
	pop	ix
	ret

; void oswrite(uint8_t ch);

	global	_oswrite

_oswrite:
	push	ix
	ld	ix, 0
	add	ix, sp
	ld	a, (ix+6)
	ld	iy, (_IY)
	call	OSWRCH
	ld	(_IY), iy
	pop	ix
	ret

; void oswrite_int16(uint16_t d);

	global	_oswrite_int16

_oswrite_int16:
	push	ix
	ld	ix, 0
	add	ix, sp
	ld	a, (ix+6)
	ld	iy, (_IY)
	call	OSWRCH
	ld	a, (ix+7)
	call	OSWRCH
	ld	(_IY), iy
	pop	ix
	ret


; void oswrite_point(const point_t *p);


	global	_oswrite_point

_oswrite_point:
	push	ix
	ld	ix, 0
	add	ix, sp
	ld	hl, (ix+6)
	ld	iy, (_IY)
	ld	a, (hl)
	call	OSWRCH
	inc	hl
	ld	a, (hl)
	call	OSWRCH
	inc	hl
	ld	a, (hl)
	call	OSWRCH
	inc	hl
	ld	a, (hl)
	call	OSWRCH
	ld	(_IY), iy
	pop	ix
	ret


	global	_IY

_IY:	dw24 	0

