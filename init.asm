;
; Title:	BBC Basic ADL for AGON - Initialisation Code
;		Initialisation Code
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/11/2023
;
; Modinfo:
; 11/07/2023:	Fixed *BYE for ADL mode
; 26/11/2023:	Moved the ram clear routine into here

			section	.text, "ax", @progbits


			XDEF	_end

			XREF	_main				; In main.asm

			XREF	RAM_START			; In ram.asm
			XREF	RAM_END

			.ASSUME	ADL = 1

			INCLUDE	"equs.inc"

argv_ptrs_max:		EQU	16				; Maximum number of arguments allowed in argv

;
; Start in ADL mode
;
			JP	_start				; Jump to start
;
; The header stuff is from byte 64 onwards
;
_exec_name:		DB	"BBCBASIC.EXE", 0		; The executable name, only used in argv

			BALIGN	64
	; TODO!!!!
			DB	"MOS"				; Flag for MOS - to confirm this is a valid MOS command
			DB	00h				; MOS header version 0
			DB	01h				; Flag for run mode (0: Z80, 1: ADL)
;
; And the code follows on immediately after the header
;
_start:			PUSH		AF			; Preserve the rest of the registers
			PUSH		BC
			PUSH		DE
			PUSH		IX
			PUSH		IY

			LD		(_sps), SP 		; Preserve the 24-bit stack pointer (SPS)

			LD		IX, _argv_ptrs		; The argv array pointer address
			PUSH		IX
			CALL		_parse_params		; Parse the parameters
			POP		IX			; IX: argv
			LD		B, 0			;  C: argc
			CALL		_clear_ram
			JP		_main			; Start user code
;
; This bit of code is called from STAR_BYE and returns us safely to the OS????
;
_end:			LD		SP, (_sps)		; Restore the stack pointer

			POP		IY			; Restore the registers
			POP		IX
			POP		DE
			POP		BC
			POP		AF
			RET					; Return to MOS

;Clear the application memory
;
_clear_ram:		PUSH		BC
			LD		HL, RAM_START
			LD		DE, RAM_START_P1
			LD		BC, RAM_SIZE
			XOR		A
			LD		(HL), A
			LDIR
			POP		BC
			RET

; Parse the parameter string into a C array
; Parameters
; - HL: Address of parameter string
; - IX: Address for array pointer storage
; Returns:
; -  C: Number of parameters parsed
;
_parse_params:		LD	BC, _exec_name
			LD	(IX+0), BC		; ARGV[0] = the executable name
			INC	IX
			INC	IX
			INC	IX
			CALL	_skip_spaces		; Skip HL past any leading spaces
;
			LD	BC, 1			; C: ARGC = 1 - also clears out top 16 bits of BCU
			LD	B, argv_ptrs_max - 1	; B: Maximum number of argv_ptrs
;
_parse_params_1:
			PUSH	BC			; Stack ARGC
			PUSH	HL			; Stack start address of token
			CALL	_get_token		; Get the next token
			LD	A, C			; A: Length of the token in characters
			POP	DE			; Start address of token (was in HL)
			POP	BC			; ARGC
			OR	A			; Check for A=0 (no token found) OR at end of string
			RET	Z
;
			LD	(IX+0), DE		; Store the pointer to the token
			PUSH	HL			; DE=HL
			POP	DE
			CALL	_skip_spaces		; And skip HL past any spaces onto the next character
			XOR	A
			LD	(DE), A			; Zero-terminate the token
			INC	IX
			INC	IX
			INC	IX			; Advance to next pointer position
			INC	C			; Increment ARGC
			LD	A, C			; Check for C >= A
			CP	B
			JR	C, _parse_params_1	; And loop
			RET

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; -  C: Length of token (in characters)
;
_get_token:		LD	C, 0			; Initialise length
_get_token_loop:	LD	A, (HL)			; Get the character from the parameter string
			OR	A			; Exit if 0 (end of parameter string in MOS)
			RET 	Z
			CP	13			; Exit if CR (end of parameter string in BBC BASIC)
			RET	Z
			CP	' '			; Exit if space (end of token)
			RET	Z
			INC	HL			; Advance to next character
			INC 	C			; Increment length
			JR	_get_token_loop

; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
;    F: Z if at end of string, otherwise NZ if there are more tokens to be parsed
;
_skip_spaces:		LD	A, (HL)			; Get the character from the parameter string
			CP	' '			; Exit if not space
			RET	NZ
			INC	HL			; Advance to next character
			JR	_skip_spaces		; Increment length

; Storage
;
_sps:			DS	3			; Storage for the stack pointer
_argv_ptrs:		DS	argv_ptrs_max*3		; Storage for the argv array pointers
