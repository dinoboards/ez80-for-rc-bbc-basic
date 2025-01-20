		.ASSUME	ADL=1

		INCLUDE	"equs.inc"
		INCLUDE	"macros.inc"

	SECTION	.text, "ax", @progbits

		GLOBAL	STAR_VDP_STATUS
		GLOBAL	STAR_VDP_REGWR
		GLOBAL	STAR_VDP_CLEAR_MEM
		GLOBAL	STAR_VDP_CMD_VDP_TO_VRAM
		GLOBAL	STAR_VDP_GRAPHIC_MODE

		EXTERN	EXPR_24BIT_INT

; READ THE STATUS REGISTER OF THE VDP INTO THE INTEGER VARIABLE

; *VDP_STATUS REG_NUM, VAR_NAME

STAR_VDP_STATUS:
		LEA	IY, IY+10 							; skip the VDP_STATUS command

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


; WRITE THE VAR_VAL BYTE TO THE VDP'S CONTROL REGISTER

; *VDP_REGWR REG_NUM, VAR_VAL

STAR_VDP_REGWR:
		LEA	IY, IY+9						; skip the VDP_STATUS command

		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE REG NUMBER

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_VAL value

		POP	DE							; reg_num
		LD	H, E							; reg_num to high byte

		PUSH	IY							; protect iy
		PUSH	HL							; low byte is the val
		LOG	vdp_reg_write
		CALL	__vdp_reg_write
		POP	HL
		POP	IY

		RET

; *VDP_CLEAR_MEM

STAR_VDP_CLEAR_MEM:
		PUSH	IY
		CALL	_vdp_clear_all_memory
		POP	IY
		RET

; *VDP_CMD_VDP_TO_VRAM x, y, width, height, colour, direction

STAR_VDP_CMD_VDP_TO_VRAM:
		LEA	IY, IY+19						; skip the VDP_STATUS command

		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE X

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE Y

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE width

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE height

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
		PUSH	HL							; SAVE color

		CALL	COMMA
		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value
										; direction

		POP	DE	; COLOUR					; Create correct argument
		POP	BC	; HEIGHT					; order for clang call
		EXX
		POP	HL	; WIDTH
		POP	DE	; Y
		POP	BC	; X

		PUSH	IY							; Protect IY
		EXX
		PUSH	HL							; DIRECTION
		PUSH	DE							; COLOUR
		PUSH	BC							; HEIGHT
		EXX
		PUSH	HL							; WIDTH
		PUSH	DE							; Y
		PUSH	BC							; X
		CALL	_vdp_cmd_vdp_to_vram
		LD	HL, 18							; restore call stack
		ADD	HL, SP
		LD	SP, HL

		POP	IY							; restore iy
		RET


; *VDP_GRAPHIC_MODE num
STAR_VDP_GRAPHIC_MODE:
		LEA	IY, IY+16						; skip the VDP_STATUS command

		CALL	NXT 							; SKIP SPACES
		CALL	EXPR_24BIT_INT 						; Evaluate REG_NUM value

		DEC	L
		LD	IX, .not_supported_yet
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, .not_supported_yet
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, .not_supported_yet
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, .not_supported_yet
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, .not_supported_yet
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, _vdp_set_graphic_6
		JR	Z, .apply_gr_mode
		DEC	L
		LD	IX, _vdp_set_graphic_7
		JR	Z, .apply_gr_mode

		XOR	A
		CALL	EXTERR
		DB	"Graphic mode must be a between 1 and 7 (inclusive)", 0

.apply_gr_mode:
		PUSH	IY
		LD	HL, .restore_iy
		PUSH	HL
		JP	(ix)
.restore_iy:
		POP	IY
		RET

.not_supported_yet:
		XOR	A
		CALL	EXTERR
		DB	"Not implemented yet", 0
