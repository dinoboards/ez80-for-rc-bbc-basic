;
; Title:	BBC Basic for AGON - Graphics stuff
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	07/06/2023
;
; Modinfo:
; 07/06/2023:	Modified to run in ADL mode

			.ASSUME	ADL = 1

			INCLUDE	"equs.inc"
			INCLUDE "macros.inc"

			section	.text, "ax", @progbits

			XDEF	CLG
			XDEF	CLRSCN
			XDEF	MODE
			XDEF	COLOUR
			XDEF	GCOL
			XDEF	MOVE
			XDEF	PLOT
			XDEF	DRAW
			XDEF	POINT
			XDEF	GETSCHR

			XREF	OSWRCH
			XREF	ASC_TO_NUMBER
			XREF	EXTERR
			XREF	EXPRI
			XREF	COMMA
			XREF	XEQ
			XREF	NXT
			XREF	BRAKET
			XREF	COUNT0
			XREF	CRTONULL
			XREF	NULLTOCR
			XREF	CRLF
			XREF	EXPR_W2
			XREF	INKEY1

; CLG: clears the graphics area
;
CLG:			VDU	10h
			JP	XEQ

; CLS: clears the text area
; ;
; CLRSCN:			LD	A, 0Ch
; 			JP	OSWRCH

; MODE n: Set video mode
;
MODE:			CALL    EXPRI
			EXX
			VDU	23			; Mode change
			VDU	L
			JP	XEQ

; GET(x,y): Get the ASCII code of a character on screen
;
GETSCHR:		INC	IY
			CALL    EXPRI      		; Get X coordinate
			EXX
			LD	(VDU_BUFFER+0), HL
			CALL	COMMA
			CALL	EXPRI			; Get Y coordinate
			EXX
			LD	(VDU_BUFFER+2), HL
			CALL	BRAKET			; Closing bracket
;
			PUSH	IX			; Get the system vars in IX
			; MOSCALL	mos_sysvars		; Reset the semaphore
			; RES	1, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			; VDU	vdp_scrchar
			VDU	(VDU_BUFFER+0)
			VDU	(VDU_BUFFER+1)
			VDU	(VDU_BUFFER+2)
			VDU	(VDU_BUFFER+3)
; $$:			BIT	1, (IX+sysvar_vpd_pflags)
; 			JR	Z, $B			; Wait for the result
			LD	A, (IX+sysvar_scrchar)	; Fetch the result in A
			OR	A			; Check for 00h
			SCF				; C = character map
			; JR	NZ, $F			; We have a character, so skip next bit
			XOR	A			; Clear carry
			DEC	A			; Set A to FFh
			POP	IX
			JP	INKEY1			; Jump back to the GET command

; POINT(x,y): Get the pixel colour of a point on screen
;
POINT:			CALL    EXPRI      		; Get X coordinate
			EXX
			LD	(VDU_BUFFER+0), HL
			CALL	COMMA
			CALL	EXPRI			; Get Y coordinate
			EXX
			LD	(VDU_BUFFER+2), HL
			CALL	BRAKET			; Closing bracket
;
			PUSH	IX			; Get the system vars in IX
			; MOSCALL	mos_sysvars		; Reset the semaphore
			RES	2, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			; VDU	vdp_scrpixel
			VDU	(VDU_BUFFER+0)
			VDU	(VDU_BUFFER+1)
			VDU	(VDU_BUFFER+2)
			VDU	(VDU_BUFFER+3)
; $$:			BIT	2, (IX+sysvar_vpd_pflags)
			; JR	Z, $B			; Wait for the result
;
; Return the data as a 1 byte index
;
			LD	L, (IX+(sysvar_scrpixelIndex))
			POP	IX
			JP	COUNT0


; COLOUR colour
; COLOUR L,P
; COLOUR L,R,G,B
;
COLOUR:			CALL	EXPRI			; The colour / mode
			EXX
			LD	A, L
			LD	(VDU_BUFFER+0), A	; Store first parameter
			CALL	NXT			; Are there any more parameters?
			CP	','
			JR	Z, COLOUR_1		; Yes, so we're doing a palette change next
;
			VDU	11h			; Just set the colour
			VDU	(VDU_BUFFER+0)
			JP	XEQ
;
COLOUR_1:		CALL	COMMA
			CALL	EXPRI			; Parse R (OR P)
			EXX
			LD	A, L
			LD	(VDU_BUFFER+1), A
			CALL	NXT			; Are there any more parameters?
			CP	','
			JR	Z, COLOUR_2		; Yes, so we're doing COLOUR L,R,G,B
;
			VDU	13h			; VDU:COLOUR
			VDU	(VDU_BUFFER+0)		; Logical Colour
			VDU	(VDU_BUFFER+1)		; Palette Colour
			VDU	0			; RGB set to 0
			VDU	0
			VDU	0
			JP	XEQ
;
COLOUR_2:		CALL	COMMA
			CALL	EXPRI			; Parse G
			EXX
			LD	A, L
			LD	(VDU_BUFFER+2), A
			CALL	COMMA
			CALL	EXPRI			; Parse B
			EXX
			LD	A, L
			LD	(VDU_BUFFER+3), A
			VDU	13h			; VDU:COLOUR
			VDU	(VDU_BUFFER+0)		; Logical Colour
			VDU	0FFh			; Physical Colour (-1 for RGB mode)
			VDU	(VDU_BUFFER+1)		; R
			VDU	(VDU_BUFFER+2)		; G
			VDU	(VDU_BUFFER+3)		; B
			JP	XEQ

; PLOT mode,x,y
;
PLOT:			CALL	EXPRI		; Parse mode
			EXX
			PUSH	HL		; Push mode (L) onto stack
			CALL	COMMA
			CALL	EXPR_W2		; Parse X and Y
			POP	BC		; Pop mode (C) off stack
PLOT_1:			VDU	24		; VDU code for PLOT
			VDU	C		;  C: Mode
			VDU	E		; DE: X
			VDU	D
			VDU	L		; HL: Y
			VDU	H
			JP	XEQ

; GCOL mode,colour
; GCOL mode,red,green,blue
;
GCOL:			LD	(_IY), IY
			CALL	_gcol
			LD	IY, (_IY)
			JP	XEQ


; MOVE x,y
;
MOVE:			LD	(_IY), IY
			CALL	_move
			LD	IY, (_IY)
			JP	XEQ

; DRAW x1,y1
; DRAW x1,y1,x2,y2
;
DRAW:			LD	(_IY), IY
			CALL	_draw
			LD	IY, (_IY)
			JP	XEQ


	global VDU
VDU:			LD	(_IY), IY
			CALL	_vdu
			LD	IY, (_IY)
			JP	XEQ

	global _vdu_not_implemented

_vdu_not_implemented:
		LD	A, 0
		CALL	EXTERR
		DB	"VDU Function not implemented", 0
