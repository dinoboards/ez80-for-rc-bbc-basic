;
; Title:	BBC Basic for AGON - Equs
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	08/06/2023
;
; Modinfo:
; 08/06/2023:	Added SIZEW

			XREF		STAVAR
			XREF		ACCS

; for a 2MB configuration, RAM is from 0x200000 to 0x21FFFF

RAM_Top:		EQU		$21FFFF	; Initial value of HIMEM
;Stack_Top:		EQU		0000h	; Stack at top
SIZEW:			EQU		3	; Size of a word (3 for ADL mode)

; For GPIO
; PA not available on eZ80L92
;
PA_DR:			EQU		096h
PA_DDR:			EQU		097h
PA_ALT1:		EQU		098h
PA_ALT2:		EQU		099h
PB_DR:          	EQU		09Ah
PB_DDR:        	 	EQU		09Bh
PB_ALT1:        	EQU		09Ch
PB_ALT2:        	EQU		09Dh
PC_DR:          	EQU		09Eh
PC_DDR:         	EQU		09Fh
PC_ALT1:        	EQU		0A0h
PC_ALT2:        	EQU		0A1h
PD_DR:          	EQU		0A2h
PD_DDR:			EQU		0A3h
PD_ALT1:		EQU		0A4h
PD_ALT2:		EQU		0A5h

GPIOMODE_OUT:		EQU		0	; Output
GPIOMODE_IN:		EQU		1	; Input
GPIOMODE_DIO:		EQU		2	; Open Drain IO
GPIOMODE_SIO:		EQU		3	; Open Source IO
GPIOMODE_INTD:		EQU		4	; Interrupt, Dual Edge
GPIOMODE_ALTF:		EQU		5;	; Alt Function
GPIOMODE_INTAL:		EQU		6	; Interrupt, Active Low
GPIOMODE_INTAH:		EQU		7	; Interrupt, Active High
GPIOMODE_INTFE:		EQU		8	; Interrupt, Falling Edge
GPIOMODE_INTRE:		EQU		9	; Interrupt, Rising Edge

; Originally in ram.asm
;
OC:			EQU     STAVAR+15*4     ; CODE ORIGIN (O%)
PC:			EQU     STAVAR+16*4     ; PROGRAM COUNTER (P%)
VDU_BUFFER:		EQU	ACCS		; Storage for VDU commands

; Originally in main.asm
;
CR:			EQU     0DH
LF:			EQU     0AH
ESC:			EQU     1BH
SINGLE_QUOTE		EQU 	27H


sysvar_vpd_pflags:	EQU	0 ; MOS params that will be removed when converted back to cp/m
sysvar_audioSuccess:	EQU	0 ; MOS params that will be removed when converted back to cp/m
sysvar_scrpixelIndex:	EQU	0 ; MOS params that will be removed when converted back to cp/m
sysvar_scrchar:		EQU	0 ; MOS params that will be removed when converted back to cp/m
