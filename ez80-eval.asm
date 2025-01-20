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

