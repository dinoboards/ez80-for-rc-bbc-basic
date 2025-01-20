		.ASSUME	ADL=1

		INCLUDE	"equs.inc"
		INCLUDE	"macros.inc"

	SECTION	.text, "ax", @progbits

		GLOBAL	STAR_VDP_STATUS

		extern	EXPR_24BIT_INT

; READ THE STATUS REGISTER OF THE VDP INTO THE INTEGER VARIABLE

; *VDP_STATUS REG_NUM, VAR_NAME

STAR_VDP_STATUS:
		INC	IY 							; skip the VDP_REGRD command
		INC	IY
		INC	IY
		INC	IY
		INC	IY
		INC	IY
		INC	IY
		INC	IY
		INC	IY
		INC	IY

		CALL	NXT 							; SKIP SPACES

		CALL	EXPR_24BIT_INT 						; expect an integer expression

		PUSH	IY
		PUSH	HL
		CALL	_vdp_get_status
		LD	HL, 0
		LD	L, A
		POP	AF
		POP	IY

		PUSH	HL							; Save status value as a 24 bit number (only 8bit value)

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES

		CALL	GETVAR 							; Try to get the variable
		JR	C, NOT_VAR 						; Error if not a variable
		CALL	NZ, PUTVAR 						; If it does not exist, then create the variable
		OR	A
		JP	M, BAD_VAR_TYPE 					; Error is is not a number variable

		POP	HL							; retrieve the saved status value
		LD	A, 0
		LD	(IX+0), HL
		LD	(IX+3), A
		LD	(IX+4), A						; INT TYPE
		RET

NOT_VAR:
		XOR	A
		CALL	EXTERR
		DB	"Expected variable name", 0

BAD_VAR_TYPE:
		LD	A, 6
		CALL	EXTERR
		DB	"String typed variable not allowed", 0
