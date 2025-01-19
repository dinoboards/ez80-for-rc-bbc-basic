;
; Title:	BBC Basic Interpreter - Z80 version
;		Catch-all for unimplemented functionality
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	12/05/2023
;
; Modinfo:

			.ASSUME	ADL = 1

			section	.text, "ax", @progbits

			XDEF	ENVEL
			XDEF	ADVAL
			XDEF	PUTIMS
			XDEF	GETIMS

			XREF	EXTERR

ENVEL:
ADVAL:
PUTIMS: ; set time string format
GETIMS: ; Get time string
			XOR     A
			CALL    EXTERR
			DEFM	"Not implemented"
			DEFB    0
