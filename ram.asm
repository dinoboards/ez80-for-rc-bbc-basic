;
; Title:	BBC Basic Interpreter - Z80 version
;		RAM Module for BBC Basic Interpreter
;		For use with Version 2.0 of BBC BASIC
;		Standard CP/M Distribution Version
; Author:	(C) Copyright  R.T.Russell 31-12-1983
; Modified By:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/06/2023
;
; Modinfo:
; 06/06/2023:	Modified to run in ADL mode
; 26/06/2023:	Added temporary stores R0 and R1

			.ASSUME	ADL = 1

			section	.bss_on_chip, "aw", @progbits

			XDEF	_ACCS
			XDEF	ACCS
			XDEF	_BUFFER
			XDEF	BUFFER
			XDEF	BUFFER2
			XDEF	_STAVAR
			XDEF	STAVAR
			XDEF	DYNVAR
			XDEF	FNPTR
			XDEF	PROPTR
			XDEF	PAGE_
			XDEF	TOP
			XDEF	_LOMEM
			XDEF	LOMEM
			XDEF 	_FREE
			XDEF 	FREE
			XDEF	_HIMEM
			XDEF	HIMEM
			XDEF	LINENO
			XDEF	TRACEN
			XDEF	AUTONO
			XDEF	ERRTRP
			XDEF	ERRTXT
			XDEF	DATPTR
			XDEF	ERL
			XDEF	ERRLIN
			XDEF	RANDOM
			XDEF	COUNT
			XDEF	WIDTH
			XDEF	ERR
			XDEF	LISTON
			XDEF	INCREM

			XDEF	FLAGS
			XDEF	OSWRCHPT
			XDEF	OSWRCHCH
			XDEF	OSWRCHFH
			XDEF	KEYDOWN
			XDEF	KEYASCII
			XDEF	KEYCOUNT

			XDEF	R0
			XDEF	R1

			XDEF	RAM_START
			XDEF	RAM_END
			XDEF	conversion_store

			BALIGN 		256		; ACCS, BUFFER & STAVAR must be on page boundaries
RAM_START:
;
_ACCS:
ACCS:			DS		256             ; String Accumulator 2 E100
_BUFFER:
BUFFER:			DS		256             ; String Input Buffer 2 E200
BUFFER2:		DS		256             ; String Input Buffer 2 E200
_STAVAR:
STAVAR:			DS	 	27*4            ; Static Variables
DYNVAR: 		DS 		54*3            ; Dynamic Variable Pointers
FNPTR:  		DS    		3               ; Dynamic Function Pointers
PROPTR: 		DS		3               ; Dynamic Procedure Pointers
;
PAGE_:   		DS		3               ; Start of User Program
TOP:    		DS		3               ; First Location after User Program
_LOMEM:
LOMEM:  		DS		3               ; Start of Dynamic Storage
_FREE:
FREE:   		DS		3               ; First Free Space Byte
_HIMEM:
HIMEM:  		DS		3               ; First Protected Byte
;
LINENO: 		DS		3               ; Line Number
TRACEN:			DS		3               ; Trace Flag
AUTONO:			DS		3               ; Auto Flag
ERRTRP:			DS		3               ; Error Trap
ERRTXT:			DS		3               ; Error Message Pointer
DATPTR:			DS		3               ; Data Pointer
ERL:			DS		3               ; Error Line
ERRLIN:			DS		3               ; The "ON ERROR" Line
RANDOM:			DS		5               ; Random Number
COUNT:			DS		1               ; Print Position
WIDTH:			DS		1               ; Print Width
ERR:			DS		1               ; Error Number
LISTON:			DS		1               ; LISTO (bottom nibble)
							; - BIT 0: If set, output a space after the line number
							; - BIT 1: If set, then indent FOR/NEXT loops
							; - BIT 2: If set, then indent REPEAT/UNTIL loops
							; - BIT 3: If set, then output to buffer for *EDIT
							; OPT FLAG (top nibble)
							; - BIT 4: If set, then list whilst assembling
							; - BIT 5: If set, then assembler errors are reported
							; - BIT 6: If set, then place the code starting at address pointed to by O%
							; - BIT 7: If set, then assemble in ADL mode, otherwise assemble in Z80 mode
INCREM:			DS		1               ; Auto-Increment Value
;
; Extra Agon-implementation specific system variables
;
FLAGS:			DS		1		; Miscellaneous flags
							; - BIT 7: Set if ESC pressed
							; - BIT 6: Set to disable ESC
OSWRCHPT:		DS		3		; Pointer for *EDIT
OSWRCHCH:		DS		1		; Channel of OSWRCH
							; - 0: Console
							; - 1: File
OSWRCHFH:		DS		1		; File handle for OSWRCHCHN
KEYDOWN:		DS		1		; Keydown flag
KEYASCII:		DS		1		; ASCII code of pressed key
KEYCOUNT:		DS		1		; Counts every time a key is pressed
R0:			DS		3		; General purpose storage for 8/16 to 24 bit operations
R1:			DS		3		;

conversion_store:	ds 		4		; Scratch pad to convert integer representations

;
; This must be at the end
;
	XDEF _RAM_END
_RAM_END:
RAM_END:

	XDEF	RAM_SIZE
	XDEF	RAM_START_P1

RAM_SIZE:	EQU	RAM_END - RAM_START - 1
RAM_START_P1:	EQU	RAM_START + 1
