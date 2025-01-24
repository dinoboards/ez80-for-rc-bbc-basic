;
; Title:	BBC Basic Interpreter - Z80 version
;		Useful macros
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	11/06/2023
;
; Modinfo:
; 11/06/2023:	Modified to run in ADL mode

.macro			EXREG			rp1 rp2
			PUSH 	rp1
			POP	rp2
.endm

.macro			ADD8U_DE			reg
			ADD	A, E
			LD	E, A
			ADC	A, D
			SUB	E
			LD	D, A
.endm

.macro			ADD8U_HL			reg
			ADD	A, L
			LD	L, A
			ADC	A, H
			SUB	L
			LD	H, A
.endm

.macro			VDU			val
			LD	A, \val
			CALL	OSWRCH
.endm

.macro			SET_GPIO		reg val
			IN0	A,(reg)
			OR	val
			OUT0	(reg),A
.endm

.macro			RES_GPIO		reg val
			PUSH	BC
			LD	A, val
			CPL
			LD	C, A
			IN0	A,(reg)
			AND	C
			OUT0	(reg),A
			POP	BC
.endm

.macro			DW24  addr
			d24	\addr
.endm

.macro			log name
	PUSH	IY
	PUSH	IX
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	EXX
	PUSH	HL
	PUSH	DE
	PUSH	BC
	EX	AF, AF'
	PUSH	AF
	LD	HL, .XX\@
	push	hl
	CALL	_log_info
	pop	hl
	POP	AF
	EX	AF, AF'
	POP	BC
	POP	DE
	POP	HL
	EXX
	POP	AF
	POP	BC
	POP	DE
	POP	HL
	POP	IX
	POP	IY

	section	.data, "aw", @progbits
.XX\@:
	DB	"\name", 0

	section	.text, "ax", @progbits

.endm
