	.ASSUME	ADL = 1

	INCLUDE	"equs.inc"
	INCLUDE "macros.inc"

	section	.text, "ax", @progbits

	XDEF	OSWRCH


;	TITLE	BBC BASIC (C) R.T.RUSSELL 1984
;
;PATCH FOR BBC BASIC TO CP/M 2.2 & 3.0
;*    PLAIN VANILLA CP/M VERSION     *
;(C) COPYRIGHT R.T.RUSSELL, 25-12-1986

; MODIFIED BY DEAN NETHERTON
; CONVERTED TO WORK WITH MARSHALLED CPM FROM ADL MODE
; (C) COPYRIGHT 2025
;
; THE FCB NEED AND ALL I/O BUFFERS NEED TO BE ON THE CP/M 64K PAGE (SECTION .bss64k)
; THE FCB WERE PREVIOUSLY ALLOCATED FROM HIMEM FOR THE SPOOL FCB AND LOMEM FOR THE OTHER
; UPTO 7 FILE HANDLES
; TODO IS TO UPDATE TO AVOID NEED TO 'ALLOCATE' AND JUST PRE-ALLOCATE THE FCB_BLOCKS
;
;
	XDEF	OSINIT
	XDEF	OSRDCH
	XDEF	OSWRCH
	XDEF	OSLINE
	XDEF	OSSAVE
	XDEF	OSLOAD
	XDEF	OSOPEN
	XDEF	OSSHUT
	XDEF	OSBGET
	XDEF	OSBPUT
	XDEF	OSSTAT
	XDEF	GETEXT
	XDEF	GETPTR
	XDEF	PUTPTR
	XDEF	PROMPT
	XDEF	RESET
	XDEF	LTRAP
	XDEF	OSCLI
	XDEF	TRAP
	XDEF	OSKEY
	XDEF	OSCALL
	XDEF	_TABLE
	XDEF	_KEYS
;
	EXTERN	ESCAPE
	EXTERN	EXTERR
	EXTERN	CHECK
	EXTERN	CRLF
	EXTERN	TELL
;
	EXTERN	ACCS
	EXTERN	FREE
	EXTERN	HIMEM
	EXTERN	ERRLIN
	EXTERN	USER

	xdef		_OPTVAL
;
;OSSAVE - Save an area of memory to a file.
;   Inputs: HL addresses filename (term CR)
;           DE = start address of data to save
;           BC = length of data to save (bytes)
; Destroys: A,B,C,D,E,H,L,F
;
STSAVE:	CALL	SAVLOD		;*SAVE
	JP	C,HUH		;"Bad command"
	PUSH	HL
	JR	OSS1
;
OSSAVE:	PUSH	BC		;SAVE
	CALL	SETUP0
OSS1:	EX	DE,HL
	CALL	CREATE
	JR	NZ,SAVE
DIRFUL:	LD	A,190
	CALL	EXTERR
	DEFM	"Directory full"
	DEFB	0
SAVE:	CALL	WRITE
	ADD	HL,BC
	EX	(SP),HL
	SBC	HL,BC
	EX	(SP),HL
	JR	Z,SAVE1
	JR	NC,SAVE
SAVE1:	POP	BC
CLOSE:	LD	A,16
	CALL	BDOS1
	INC	A
	RET	NZ
	LD	A,200
	CALL	EXTERR
	DEFM	"Close error"
	DEFB	0
;
;OSSHUT - Close disk file(s).
;   Inputs: E = file channel
;           If E=0 all files are closed (except SPOOL)
; Destroys: A,B,C,D,E,H,L,F
;
OSSHUT: LD	A,E
	OR	A
	JR	NZ,SHUT1
SHUT0:	INC	E
	BIT	3,E
	RET	NZ
	PUSH	DE
	CALL	SHUT1
	POP	DE
	JR	SHUT0
;
SESHUT:	LD	HL,FLAGS
	RES	0,(HL)		;STOP EXEC
	RES	1,(HL)		;STOP SPOOL
	LD	E,8		;SPOOL/EXEC CHANNEL
SHUT1:	CALL	FIND1
	RET	Z
	XOR	A
	LD	(HL),A		;CLEAR CHANNEL PTR
	DEC	HL
	LD	(HL),A
	DEC	HL
	LD	(HL),A
	LD	HL,37
	ADD	HL,DE
	BIT	7,(HL)
	INC	HL
	CALL	NZ,WRITE
	LD	HL,FCBSIZ
	ADD	HL,DE
	LD	BC,(FREE_FCB_TABLE)
	SBC	HL,BC
	JP	NZ,CLOSE
	LD	(FREE_FCB_TABLE),DE	;RELEASE SPACE
	JP	CLOSE
;
;TYPE - *TYPE command.
;Types file to console output.
;
TYPE:	SCF			;*TYPE
	CALL	OSOPEN
	OR	A
	JR	Z,NOTFND
	LD	E,A
TYPE1:	LD	A,(FLAGS)	;TEST
	BIT	7,A		;FOR
	JR	NZ,TYPESC	;ESCape
	CALL	OSBGET
	CALL	OSWRCH		;N.B. CALLS "TEST"
	JR	NC,TYPE1
	JP	OSSHUT
;
TYPESC:	CALL	OSSHUT		;CLOSE!
	JP	ABORT
;
;OSLOAD - Load an area of memory from a file.
;   Inputs: HL addresses filename (term CR)
;           DE = address at which to load
;           BC = maximum allowed size (bytes)
;  Outputs: Carry reset indicates no room for file.
; Destroys: A,B,C,D,E,H,L,F
;
STLOAD:	CALL	SAVLOD		;*LOAD
	PUSH	HL
	JR	OSL1
;
OSLOAD:	PUSH	BC		;LOAD
	CALL	SETUP0
OSL1:	EX	DE,HL
	CALL	OPEN
	JR	NZ,LOAD0
NOTFND:	LD	A,214
	CALL	EXTERR
	DEFM	"File not found"
	DEFB	0
LOAD:	CALL	READ
	JR	NZ,LOAD1
	CALL	INCSEC
	ADD	HL,BC
LOAD0:	EX	(SP),HL
	SBC	HL,BC
	EX	(SP),HL
	JR	NC,LOAD
LOAD1:	POP	BC
	PUSH	AF
	CALL	CLOSE
	POP	AF
	CCF
OSCALL:	RET
;
;OSOPEN - Open a file for reading or writing.
;   Inputs: HL addresses filename (term CR)
;           Carry set for OPENIN, cleared for OPENOUT.
;  Outputs: A = file channel (=0 if cannot open)
;           DE = file FCB
; Destroys: A,B,C,D,E,H,L,F
;
OPENIT:	PUSH	AF		;SAVE CARRY
	CALL	SETUP0
	POP	AF
	CALL	NC,CREATE
	CALL	C,OPEN
	RET
;
OSOPEN:	CALL	OPENIT
	RET	Z		;ERROR
	LD	B,7		;MAX. NUMBER OF FILES
	LD	IX,TABLE+7*3
OPEN1:	XOR	A

	OR	(IX+0)				; lower byte
	JR	NZ, OPEN_TRYNEXT

	OR	(IX+1)				; high byte
	JR	NZ, OPEN_TRYNEXT

	OR	(IX+2)				; upper byte
	JR	NZ, OPEN_TRYNEXT

	JR	OPEN2

OPEN_TRYNEXT:
	DEC	IX
	DEC	IX
	DEC	IX
	DJNZ	OPEN1

	LD	A,192
	CALL	EXTERR
	DEFM	"Too many open files"
	DEFB	0
;
OPEN2:	LD	DE,(FREE_FCB_TABLE)	;FREE SPACE POINTER
	LD	(IX+0), DE
	LD	A,B		;CHANNEL (1-7)
	LD	HL,FCBSIZ
	ADD	HL,DE		;RESERVE SPACE
	LD	(FREE_FCB_TABLE),HL
OPEN3:
	LD	HL,FCB		;ENTRY FROM SPOOL/EXEC
	PUSH	DE
	LD	BC,36
	LDIR			;COPY FCB
	EX	DE,HL
	INC	HL
	LD	(HL),C		;CLEAR PTR
	INC	HL
	POP	DE
	LD	B,A
	CALL	RDF		;READ OR FILL
	LD	A,B
	CALL	CHECK
	RET

;
;OSBPUT - Write a byte to a random disk file.
;   Inputs: E = file channel
;           A = byte to write
; Destroys: A,B,C,F
;
OSBPUT:	PUSH	DE
	PUSH	HL
	LD	B,A
	CALL	FIND
	LD	A,B
	LD	B,0
	DEC	HL
	LD	(HL),B		;CLEAR EOF
	INC	HL
	LD	C,(HL)
	RES	7,C
	SET	7,(HL)
	INC	(HL)
	INC	HL
	PUSH	HL
	; maybe wrong - as its now a 24 bit add
	ADD	HL,BC
	LD	(HL),A
	POP	HL
	CALL	Z,WRRDF		;WRITE THEN READ/FILL
	POP	HL
	POP	DE
	RET
;
;OSBGET - Read a byte from a random disk file.
;   Inputs: E = file channel
;  Outputs: A = byte read
;           Carry set if LAST BYTE of file
; Destroys: A,B,C,F
;
OSBGET:	PUSH	DE
	PUSH	HL
	CALL	FIND
	LD	C,(HL)
	RES	7,C
	INC	(HL)
	INC	HL
	PUSH	HL
	LD	B,0
	ADD	HL,BC
	LD	B,(HL)
	POP	HL
	CALL	PE,INCRDF	;INC SECTOR THEN READ
	CALL	Z,WRRDF		;WRITE THEN READ/FILL
	LD	A,B
	POP	HL
	POP	DE
	RET
;
;OSSTAT - Read file status.
;   Inputs: E = file channel
;  Outputs: Z flag set - EOF
;           (If Z then A=0)
;           DE = address of file block.
; Destroys: A,D,E,H,L,F
;
OSSTAT:	CALL	FIND
	DEC	HL
	LD	A,(HL)
	INC	A
	RET
;
;GETEXT - Find file size.
;   Inputs: E = file channel
;  Outputs: DEHL = file size (0-&800000)
; Destroys: A,B,C,D,E,H,L,F
;
GETEXT:	CALL	FIND
	EX	DE,HL
	LD	DE,FCB
	LD	BC,36
	PUSH	DE
	LDIR			;COPY FCB
	EX	DE,HL
	EX	(SP),HL
	EX	DE,HL
	LD	A,35
	CALL	BDOS1		;COMPUTE SIZE
	POP	HL
	XOR	A
	JR	GETPT1
;
;GETPTR - Return file pointer.
;   Inputs: E = file channel
;  Outputs: DEHL = pointer (0-&7FFFFF) ?????
; Destroys: A,B,C,D,E,H,L,F
;
GETPTR:	CALL	FIND
	LD	A,(HL)
	ADD	A,A
	DEC	HL
GETPT1:	DEC	HL
	LD	D,(HL)
	DEC	HL
	LD	E,(HL)
	DEC	HL
	LD	H,(HL)
	LD	L,A
	SRL	D
	RR	E
	RR	H
	RR	L
	RET
;
;PUTPTR - Update file pointer.
;   Inputs: A = file channel
;           DEHL = new pointer (0-&7FFFFF)
; Destroys: A,B,C,D,E,H,L,F
;
PUTPTR: LD	D,L
	ADD	HL,HL
	RL	E
	LD	B,E
	LD	C,H
	LD	E,A		;CHANNEL
	PUSH	DE
	CALL	FIND
	POP	AF
	AND	7FH
	BIT	7,(HL)		;PENDING WRITE?
	JR	Z,PUTPT1
	OR	80H
PUTPT1:	LD	(HL),A
	PUSH	DE
	PUSH	HL
	DEC	HL
	DEC	HL
	DEC	HL
	LD	D,(HL)
	DEC	HL
	LD	E,(HL)
	EX	DE,HL
	OR	A
	SBC	HL,BC
	POP	HL
	POP	DE
	RET	Z
	INC	HL
	OR	A
	CALL	M,WRITE
	PUSH	HL
	DEC	HL
	DEC	HL
	DEC	HL
	LD	(HL),0
	DEC	HL
	LD	(HL),B
	DEC	HL
	LD	(HL),C		;NEW RECORD NO.
	POP	HL
	JR	RDF
;
;WRRDF - Write, read; if EOF fill with zeroes.
;RDF - Read; if EOF fill with zeroes.
;   Inputs: DE address FCB.
;           HL addresses data buffer.
;  Outputs: A=0, Z-flag set.
;           Carry set if fill done (EOF)
; Destroys: A,H,L,F
;
WRRDF:	CALL	WRITE
RDF:	CALL	READ
	DEC	HL
	RES	7,(HL)
	DEC	HL
	LD	(HL),A		;CLEAR EOF FLAG
	RET	Z
	LD	(HL),-1		;SET EOF FLAG
	INC	HL
	INC	HL
	PUSH	BC
	XOR	A
	LD	B,128
FILL:	LD	(HL),A
	INC	HL
	DJNZ	FILL
	POP	BC
	SCF
	RET
;
;INCRDF - Increment record, read; if EOF fill.
;   Inputs: DE addresses FCB.
;           HL addresses data buffer.
;  Outputs: A=1, Z-flag reset.
;           Carry set if fill done (EOF)
; Destroys: A,H,L,F
;
INCRDF:	CALL	INCSEC
	CALL	RDF
	INC	A
	RET
;
;READ - Read a record from a disk file.
;   Inputs: DE addresses FCB.
;           HL = address to store data.
;  Outputs: A<>0 & Z-flag reset indicates EOF.
;           Carry = 0
; Destroys: A,F
;
;BDOS1 - CP/M BDOS call.
;   Inputs: A = function number
;          DE = parameter
;  Outputs: AF = result (carry=0)
; Destroys: A,F
;
READ:	PUSH	HL
	LD	HL, DISK_BUFFER				; RETRIEVE BLOCK TO DISK_BUFFER (WITHIN 64K PAGE)
	CALL	SETDMA
	POP	HL

	LD	A,33
	CALL	BDOS0					; INITIATE THE TRANSFER
	JR	NZ,CPMERR

	PUSH	HL					; COPY FROM DISK_BUFFER TO DESTINATION ADDR
	PUSH	DE
	PUSH	BC
	EX	DE, HL
	LD	HL, DISK_BUFFER
	LD	BC, 128
	LDIR
	POP	BC
	POP	DE
	POP	HL

	OR	A		;*
	RET			;*

BDOS1:	CALL	BDOS0		;*
	JR	NZ,CPMERR	;*
	OR	A		;*
	RET			;*

CPMERR:	LD	A,255		;* CP/M 3
	CALL	EXTERR		;* BDOS ERROR
	DEFM	"CP/M Error"	;*
	DEFB	0		;*
;
BDOS0:	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	LD	C,A
	CALL	BDOS
	INC	H		;* TEST H
	DEC	H		;* CP/M 3 ONLY
	POP	IY
	POP	IX
	POP	HL
	POP	DE
	POP	BC
	RET
;
;WRITE - Write a record to a disk file.
;   Inputs: DE addresses FCB.
;           HL = address to get data.
; Destroys: A,F
;
WRITE:	PUSH	DE				; TRANSFER DATA TO THE 64K PAGE
	PUSH	BC
	LD	DE, DISK_BUFFER
	LD	BC, 128
	LDIR
	POP	BC
	POP	DE
	LD	HL, DISK_BUFFER			; DISK_BUFFER CAN NOW BE WRITTEN TO DISK
	CALL	SETDMA
	LD	A,40				; INITATE THE DATA TRANSFER
	CALL	BDOS1
	JR	Z,INCSEC
	LD	A,198
	CALL	EXTERR
	DEFM	"Disk full"
	DEFB	0
;
;INCSEC - Increment random record number.
;   Inputs: DE addresses FCB.
; Destroys: F
;
INCSEC:	PUSH	HL
	LD	HL,33
	ADD	HL,DE
INCS1:	INC	(HL)
	INC	HL
	JR	Z,INCS1
	POP	HL
	RET
;
;OPEN - Open a file for access.
;   Inputs: FCB set up.
;  Outputs: DE = FCB
;           A=0 & Z-flag set indicates Not Found.
;           Carry = 0
; Destroys: A,D,E,F
;
OPEN:	LD	DE,FCB
	LD	A,15
	CALL	BDOS1
	INC	A
	RET
;
;CREATE - Create a disk file for writing.
;   Inputs: FCB set up.
;  Outputs: DE = FCB
;           A=0 & Z-flag set indicates directory full.
;           Carry = 0
; Destroys: A,D,E,F
;
CREATE:	CALL	CHKAMB
	LD	DE,FCB
	LD	A,19
	CALL	BDOS1		;DELETE
	LD	A,22
	CALL	BDOS1		;MAKE
	INC	A
	RET
;
;CHKAMB - Check for ambiguous filename.
; Destroys: A,D,E,F
;
CHKAMB:	PUSH	BC
	LD	DE,FCB
	LD	B,12
CHKAM1:	LD	A,(DE)
	CP	'?'
	JR	Z,AMBIG		;AMBIGUOUS
	INC	DE
	DJNZ	CHKAM1
	POP	BC
	RET
AMBIG:	LD	A,204
	CALL	EXTERR
	DEFM	"Bad name"
	DEFB	0
;
;SETDMA - Set "DMA" address.
;   Inputs: HL = address
; Destroys: A,F
;
SETDMA:	LD	A,26
	EX	DE,HL
	CALL	BDOS0
	EX	DE,HL
	RET
;
;FIND - Find file parameters from channel.
;   Inputs: E = channel
;  Outputs: DE addresses FCB
;           HL addresses pointer byte (FCB+37)
; Destroys: A,D,E,H,L,F
;
FIND:	CALL	FIND1
	LD	HL,37
	ADD	HL,DE
	RET	NZ
	LD	A,222
	CALL	EXTERR
	DEFM	"Channel"
	DEFB	0
;
;FIND1 - Look up file table.
;   Inputs: E = channel
;  Outputs: Z-flag set = file not opened
;           If NZ, DE addresses FCB
;                  HL points into table
; Destroys: A,D,E,H,L,F
;
FIND1:	LD	A, E
	AND	7
	LD	HL, 0			; HL = A
	LD	DE, 0
	LD	E, A			; DE = A
	LD	L, A
	ADD	HL, HL			; HL *= 3
	ADD	HL, DE
	LD	DE, TABLE		; DE = TABLE
	ADD	HL, DE			; HL = &TABLE[A]
	XOR	A
	LD	DE, (HL)		; DE = *HL
	INC	HL
	INC	HL
	CP	E
	RET	NZ
	CP	D
	RET	NZ
	CP	(HL)
	RET
;
;SETUP - Set up File Control Block.
;   Inputs: HL addresses filename
;           Format  [A:]FILENAME[.EXT]
;           Device defaults to current drive
;           Extension defaults to .BBC
;           A = fill character
;  Outputs: HL updated
;           A = terminator
;           BC = 128
; Destroys: A,B,C,H,L,F
;
;FCB FORMAT (36 BYTES TOTAL):
; 0      0=SAME DISK, 1=DISK A, 2=DISK B (ETC.)
; 1-8    FILENAME, PADDED WITH SPACES
; 9-11   EXTENSION, PADDED WITH SPACES
; 12     CURRENT EXTENT, SET TO ZERO
; 32-35  CLEARED TO ZERO
;
SETUP0:	LD	A,' '
SETUP:	PUSH	DE
	PUSH	HL
	LD	DE,FCB+9
	LD	HL,BBC
	LD	BC,3
	LDIR
	LD	HL,FCB+32
	LD	B,4
SETUP1:	LD	(HL),C
	INC	HL
	DJNZ	SETUP1
	POP	HL
	LD	C,A
	XOR	A
	LD	(DE),A
	POP	DE
	CALL	SKIPSP
	CP	'"'
	JR	NZ,SETUP2
	INC	HL
	CALL	SKIPSP
	CALL	SETUP2
	CP	'"'
	INC	HL
	JR	Z,SKIPSP
BADSTR:	LD	A,253
	CALL	EXTERR
	DEFM	"Bad string"
	DEFB	0
;
PARSE:	LD	A,(HL)
	INC	HL
	CP	'`'
	RET	NC
	CP	'?'
	RET	C
	XOR	40H
	RET
;
SETUP2:	PUSH	DE
	INC	HL
	LD	A,(HL)
	CP	':'
	DEC	HL
	LD	A,B
	JR	NZ,DEVICE
	LD	A,(HL)		;DRIVE
	AND	31
	INC	HL
	INC	HL
DEVICE:	LD	DE,FCB
	LD	(DE),A
	INC	DE
	LD	B,8
COPYF:	LD	A,(HL)
	CP	'.'
	JR	Z,COPYF1
	CP	' '
	JR	Z,COPYF1
	CP	CR
	JR	Z,COPYF1
	CP	'='
	JR	Z,COPYF1
	CP	'"'
	JR	Z,COPYF1
	LD	C,'?'
	CP	'*'
	JR	Z,COPYF1
	LD	C,' '
	INC	HL
	CP	'|'
	JR	NZ,COPYF2
	CALL	PARSE
	JR	COPYF0
COPYF1:	LD	A,C
COPYF2:	CALL	UPPRC
COPYF0:	LD	(DE),A
	INC	DE
	DJNZ	COPYF
COPYF3:	LD	A,(HL)
	INC	HL
	CP	'*'
	JR	Z,COPYF3
	CP	'.'
	LD	BC,3*256+' '
	LD	DE,FCB+9
	JR	Z,COPYF
	DEC	HL
	POP	DE
	LD	BC,128
SKIPSP:	LD	A,(HL)
	CP	' '
	RET	NZ
	INC	HL
	JR	SKIPSP
;
BBC:	DEFM	"BBC"
;
;HEX - Read a hex string and convert to binary.
;   Inputs: HL = text pointer
;  Outputs: HL = updated text pointer
;           DE = value
;            A = terminator (spaces skipped)
; Destroys: A,D,E,H,L,F
;
HEX:	LD	DE,0		;INITIALISE
	CALL	SKIPSP
HEX1:	LD	A,(HL)
	CALL	UPPRC
	CP	'0'
	JR	C,SKIPSP
	CP	'9'+1
	JR	C,HEX2
	CP	'A'
	JR	C,SKIPSP
	CP	'F'+1
	JR	NC,SKIPSP
	SUB	7
HEX2:	AND	0FH
	EX	DE,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	EX	DE,HL
	OR	E
	LD	E,A
	INC	HL
	JR	HEX1
;
;OSCLI - Process an "operating system" command
;
OSCLI:	CALL	SKIPSP
	CP	CR
	RET	Z
	CP	'|'
	RET	Z
	CP	'.'
	JP	Z,DOT		;*.
	EX	DE,HL
	LD	HL,COMDS
OSCLI0:	LD	A,(DE)
	CALL	UPPRC
	CP	(HL)
	JR	Z,OSCLI2
	JP	C,HUH
OSCLI1:	BIT	7,(HL)
	INC	HL
	JR	Z,OSCLI1
	INC	HL
	INC	HL
	INC	HL
	JR	OSCLI0
;
OSCLI2:	PUSH	DE
OSCLI3:	INC	DE
	INC	HL
	LD	A,(DE)
	CALL	UPPRC
	CP	'.'		;ABBREVIATED?
	JR	Z,OSCLI4
	XOR	(HL)
	JR	Z,OSCLI3
	CP	80H
	JR	Z,OSCLI4
	POP	DE
	JR	OSCLI1
;
OSCLI4:	POP	AF
	INC	DE
OSCLI5:	BIT	7,(HL)
	INC	HL
	JR	Z,OSCLI5
	ld	hl, (hl)
	PUSH	HL
	EX	DE,HL
	JP	SKIPSP
;
;
ERA:	CALL	SETUP0		;*ERA, *ERASE
	LD	C,19
	JR	XEQ		;"DELETE"
;
RES:	LD	C,13		;*RESET
	JR	XEQ		;"RESET"
;
DRV:	CALL	SETUP0		;*DRIVE
	LD	A,(FCB)
	DEC	A
	JP	M,HUH
	LD	E,A
	LD	C,14
	JR	XEQ0
;
REN:	CALL	SETUP0		;*REN, *RENAME
	CP	'='
	JR	NZ,HUH
	INC	HL		;SKIP "="
	PUSH	HL
	CALL	EXISTS
	LD	HL,FCB
	LD	DE,FCB+16
	LD	BC,12
	LDIR
	POP	HL
	CALL	SETUP0
	CALL	CHKAMB
	LD	C,23
XEQ:	LD	DE,FCB
XEQ0:	LD	A,(HL)
	CP	CR
	JR	NZ,HUH
BDC:	LD	A,C
	CALL	BDOS1
	RET	P
HUH:	LD	A,254
	CALL	EXTERR
	DEFM	"Bad command"
	DEFB	0
;
EXISTS:	LD	HL,DSKBUF

	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	_abort_exists

	CALL	SETDMA
	LD	DE,FCB
	LD	A,17
	CALL	BDOS1		;SEARCH
	INC	A
	RET	Z
	LD	A,196
	CALL	EXTERR
	DEFM	"File exists"
	DEFB	0
;
SAVLOD:	CALL	SETUP0		;PART OF *SAVE, *LOAD
	CALL	HEX
	CP	'+'
	PUSH	AF
	PUSH	DE
	JR	NZ,SAVLO1
	INC	HL
SAVLO1:	CALL	HEX
	CP	CR
	JP	NZ,HUH
	EX	DE,HL
	POP	DE
	POP	AF
	RET	Z
	OR	A
	SBC	HL,DE
	RET	NZ
	JR	HUH
;
DOT:	INC	HL
DIR:	LD	A,'?'		;*DIR
	CALL	SETUP
	CP	CR
	JR	NZ,HUH
	LD	C,17
DIR0:	LD	B,4
DIR1:	CALL	LTRAP
	LD	DE,FCB
	LD	HL,DSKBUF
	CALL	SETDMA
	LD	A,C
	CALL	BDOS1		;SEARCH DIRECTORY
	JP	M,CRLF
	RRCA
	RRCA
	RRCA
	AND	60H
	ld	de, 0
	LD	E,A
	LD	HL,DSKBUF+1
	ADD	HL,DE
	PUSH	HL
	LD	DE,8		;**
	ADD	HL,DE
	LD	E,(HL)		;**
	INC	HL		;**
	BIT	7,(HL)		;SYSTEM FILE?
	POP	HL
	LD	C,18
	Jr	NZ,DIR1
	PUSH	BC
	LD	A,(FCB)
	DEC	A
	LD	C,25
	CALL	M,BDC
	ADD	A,'A'
	CALL	OSWRCH
	LD	B,8
	LD	A,' '		;**
	BIT	7,E		;** READ ONLY?
	JR	Z,DIR3		;**
	LD	A,'*'		;**
DIR3:	CALL	CPTEXT
	LD	B,3
	LD	A,' '		;**
	CALL	SPTEXT
	POP	BC
	DJNZ	DIR2
	CALL	CRLF
	JR	DIR0
;
DIR2:	PUSH	BC
	LD	B,5
PAD:	LD	A,' '
	CALL	OSWRCH
	DJNZ	PAD
	POP	BC
	JR	DIR1
;
OPT:	CALL	HEX		;*OPT
	LD	A,E
	AND	3
SETOPT:	LD	(OPTVAL),A
	RET
;
RESET:	XOR	A
	JR	SETOPT
;
EXEC:	LD	A,00000001B	;*EXEC
	DEFB	1		;SKIP 2 BYTES (LD BC)
SPOOL:	LD	A,00000010B	;*SPOOL
	PUSH	AF
	PUSH	HL
	CALL	SESHUT		;STOP SPOOL/EXEC
	POP	HL
	POP	BC
	LD	A,(HL)
	CP	CR		;JUST SHUT?
	RET	Z
	LD	A,(FLAGS)
	OR	B
	LD	(FLAGS),A	;SPOOL/EXEC FLAG
	RRA			;CARRY=1 FOR EXEC
	CALL	OPENIT		;OPEN SPOOL/EXEC FILE
	RET	Z		;DIR FULL / NOT FOUND
	POP	IX		;RETURN ADDRESS
	LD	DE, FCB_SPOOL
	LD	(TABLE), DE
	CALL	OPEN3		;FINISH OPEN OPERATION
JPIX:	JP	(IX)		;"RETURN"
;
UPPRC:	AND	7FH
	CP	'`'
	RET	C
	AND	5FH		;CONVERT TO UPPER CASE
	RET
;
;*ESC COMMAND
;
ESCCTL:	LD	A,(HL)
	CALL	UPPRC		;**
	CP	'O'
	JR	NZ,ESCC1
	INC	HL
ESCC1:	CALL	HEX
	LD	A,E
	OR	A
	LD	HL,FLAGS
	RES	6,(HL)		;ENABLE ESCAPE
	RET	Z
	SET	6,(HL)		;DISABLE ESCAPE
	RET
;
;
COMDS:	DB	'AS','M'+80h		; ASM
	d24	STAR_ASM
	DEFM	"BY"
	DEFB	'E'+80H
	d24	BYE
	DEFM	"CP"
	DEFB	'M'+80H
	d24	BYE
	db	"DEBU", 'G' + 80H
	D24	debug
	DEFM	"DI"
	DEFB	'R'+80H
	d24	DIR
	DEFM	"DRIV"
	DEFB	'E'+80H
	d24	DRV
	DEFM	"ERAS"
	DEFB	'E'+80H
	d24	ERA
	DEFM	"ER"
	DEFB	'A'+80H
	d24	ERA
	DEFM	"ES"
	DEFB	'C'+80H
	d24	ESCCTL
	DEFM	"EXE"
	DEFB	'C'+80H
	d24	EXEC
	DEFM	"LOA"
	DEFB	'D'+80H
	d24	STLOAD
	DEFM	"OP"
	DEFB	'T'+80H
	d24	OPT
	DEFM	"RENAM"
	DEFB	'E'+80H
	d24	REN
	DEFM	"RE"
	DEFB	'N'+80H
	d24	REN
	DEFM	"RESE"
	DEFB	'T'+80H
	d24	RES
	DEFM	"SAV"
	DEFB	'E'+80H
	d24	STSAVE
	DEFM	"SPOO"
	DEFB	'L'+80H
	d24	SPOOL
	DEFM	"TYP"
	DEFB	'E'+80H
	d24	TYPE
	DB	'VERSIO','N'+80h	; VERSION
	d24	STAR_VERSION

	DEFB	0FFH

debug:
	push	iy
	CALL	_debug
	pop	iy
	ret
;
;Print text
;   Inputs: HL = address of text
;            B = number of characters to print
; Destroys: A,B,H,L,F
;
CPTEXT:	PUSH	AF		;**
	LD	A,':'
	CALL	OSWRCH
	POP	AF		;**
SPTEXT:	CALL	OSWRCH		;**
CPTXTL:	LD	A,(HL)
	AND	7FH
	INC	HL
	CALL	OSWRCH
	DJNZ	CPTXTL
	RET
;
;OSINIT - Initialise RAM mapping etc.
;If BASIC is entered by BBCBASIC FILENAME then file
;FILENAME.BBC is automatically CHAINed. TODO - THIS HAS BEEN DISABLED HERE
;   Outputs: DE = initial value of HIMEM (top of RAM)
;            HL = initial value of PAGE (user program)
;            Z-flag reset indicates AUTO-RUN.
;  Destroys: A,B,C,D,E,H,L,F
;
OSINIT:	LD	C,45		;*
	LD	E,254		;*
	CALL	BDOS		;*
	call	_debug		; retrieve LOMEM aligned to 256 byte boundary
NOBOOT:	LD	DE,$3FFFFF	;DE = HIMEM!!!!
	LD	E,A		;PAGE BOUNDARY
	RET
;
;BYE - Stop interrupts and return to CP/M.
;
BYE:	JP	_abort
;
;
;TRAP - Test ESCAPE flag and abort if set;
;       every 20th call, test for keypress.
; Destroys: A,H,L,F
;
;LTRAP - Test ESCAPE flag and abort if set.
; Destroys: A,F
;
TRAP:	LD	HL,TRPCNT
	DEC	(HL)
	CALL	Z,TEST20	;TEST KEYBOARD
LTRAP:	LD	A,(FLAGS)	;ESCAPE FLAG
	OR	A		;TEST
	RET	P
ABORT:	LD	HL,FLAGS	;ACKNOWLEDGE
	RES	7,(HL)		;ESCAPE
	JP	ESCAPE		;AND ABORT
;
;TEST - Sample for ESCape and CTRL/S. If ESCape
;       pressed set ESCAPE flag and return.
; Destroys: A,F
;
TEST20:	LD	(HL),20
TEST:	PUSH	DE
	LD	A,6
	LD	E,0FFH
	CALL	BDOS0
	POP	DE
	OR	A
	RET	Z
	CP	'S' & 1FH	;PAUSE DISPLAY?
	JR	Z,OSRDCH
	CP	ESC
	JR	Z,ESCSET
	LD	(INKEY),A
	RET
;
;OSRDCH - Read from the current input stream (keyboard).
;  Outputs: A = character
; Destroys: A,F
;
KEYGET:	LD	A,(SCREEN_TEXT_WIDTH)	;SCREEN WIDTH
	LD	B, A
OSRDCH:	LD	A,(FLAGS)
	RRA			;*EXEC ACTIVE?
	JR	C,EXECIN
	PUSH	HL
	SBC	HL,HL		;HL=0
	CALL	OSKEY
	POP	HL
	RET	C
	JR	OSRDCH
;
;EXECIN - Read byte from EXEC file
;  Outputs: A = byte read
; Destroys: A,F
;
EXECIN:	PUSH	BC		;SAVE REGISTERS
	PUSH	DE
	PUSH	HL
	LD	E,8		;SPOOL/EXEC CHANNEL
	LD	HL,FLAGS
	RES	0,(HL)
	CALL	OSBGET
	SET	0,(HL)
	PUSH	AF
	CALL	C,SESHUT	;END EXEC IF EOF
	POP	AF
	POP	HL		;RESTORE REGISTERS
	POP	DE
	POP	BC
	RET
;
;
;OSKEY - Read key with time-limit, test for ESCape.
;Main function is carried out in user patch.
;   Inputs: HL = time limit (centiseconds)
;  Outputs: Carry reset if time-out
;           If carry set A = character
; Destroys: A,H,L,F
;
OSKEY:	PUSH	HL
	LD	HL,INKEY
	LD	A,(HL)
	LD	(HL),0
	POP	HL
	OR	A
	SCF
	RET	NZ
	PUSH	DE
	CALL	GETKEY
	POP	DE
	RET	NC
	CP	ESC
	SCF
	RET	NZ
ESCSET:	PUSH	HL
	LD	HL,FLAGS
	BIT	6,(HL)		;ESC DISABLED?
	JR	NZ,ESCDIS
	SET	7,(HL)		;SET ESCAPE FLAG
ESCDIS:	POP	HL
	RET
;
;GETKEY - Sample keyboard with specified wait.
;   Inputs: HL = Time to wait (centiseconds)
;  Outputs: Carry reset indicates time-out.
;           If carry set, A = character typed.
; Destroys: A,D,E,H,L,F
;
GETKEY:	PUSH	BC
	PUSH	HL
	LD	C,6
	LD	E,0FFH
	CALL	BDOS		;CONSOLE INPUT
	POP	HL
	POP	BC
	OR	A
	SCF
	RET	NZ		;KEY PRESSED
	OR	H
	OR	L
	RET	Z		;TIME-OUT
	PUSH	HL
	LD	HL,TIME
	LD	A,(HL)
WAIT1:	CP	(HL)
	JR	Z,WAIT1		;WAIT FOR 10 ms.
	POP	HL
	DEC	HL
	JR	GETKEY
;
;OSWRCH - Write a character to console output.
;   Inputs: A = character.
; Destroys: Nothing
;
OSWRCH:	PUSH	AF
	PUSH	DE
	PUSH	HL
	LD	E,A
	CALL	TEST
	CALL	EDPUT
	POP	HL
	POP	DE
	POP	AF
	RET
;
EDPUT:	LD	A,(FLAGS)
	BIT	3,A
	JR	Z,WRCH
	LD	A,E
	CP	' '
	RET	C
	LD	HL,(EDPTR)
	LD	(HL),E
	INC	L
	RET	Z
	LD	(EDPTR),HL
	RET
;
PROMPT:	LD	E,'>'
WRCH:	LD	A,(OPTVAL)	;FAST ENTRY
	ADD	A,3
	CP	3
	JR	NZ,WRCH1
	ADD	A,E
	LD	A,2
	 JR	C,WRCH1
	LD	A,6
WRCH1:	CALL	BDOS0
	LD	HL,FLAGS
	BIT	2,(HL)
	LD	A,5		;PRINTER O/P
	CALL	NZ,BDOS0
	BIT	1,(HL)		;SPOOLING?
	RET	Z
	RES	1,(HL)
	LD	A,E		;BYTE TO WRITE
	LD	E,8		;SPOOL/EXEC CHANNEL
	PUSH	BC
	CALL	OSBPUT
	POP	BC
	SET	1,(HL)
	RET
;
TOGGLE:	LD	A,(FLAGS)
	XOR	00000100B
	LD	(FLAGS),A
	RET
;
;OSLINE - Read/edit a complete line, terminated by CR.
;   Inputs: HL addresses destination buffer.
;           (L=0)
;  Outputs: Buffer filled, terminated by CR.
;           A=0.
; Destroys: A,B,C,D,E,H,L,F
;
OSLINE:	LD	A,(FLAGS)
	BIT	3,A		;EDIT MODE?
	JR	Z,OSLIN1
	RES	3,A
	LD	(FLAGS),A
	LD	HL,(EDPTR)
	CP	L
OSLIN1:	LD	A,CR
	LD	(HL),A
	CALL	NZ,OSWRCH
	LD	L,0
	LD	C,L		;REPEAT FLAG
	JR	Z,OSWAIT	;SUPPRESS UNWANTED SPACE
UPDATE:	LD	B,0
UPD1:	LD	A,(HL)
	INC	B
	INC	HL
	CP	CR
	PUSH	AF
	PUSH	HL
	LD	E,A
	CALL	NZ,WRCH		;FAST WRCH
	POP	HL
	POP	AF
	JR	NZ,UPD1
	LD	A,' '
	CALL	OSWRCH
	LD	E,BS
UPD2:	PUSH	HL
	CALL	WRCH		;FAST WRCH
	POP	HL
	DEC	HL
	DJNZ	UPD2
OSWAIT:	LD	A,C
	DEC	B
	JR	Z,LIMIT
	OR	A		;REPEAT COMMAND?
LIMIT:	CALL	Z,KEYGET	;READ KEYBOARD
	LD	C,A		;SAVE FOR REPEAT
	LD	DE,OSWAIT	;RETURN ADDRESS
	PUSH	DE
	LD	A,(FLAGS)
	OR	A		;TEST FOR ESCAPE
	LD	A,C
	JP	M,OSEXIT
	CP	KY_CURSOR_UP
	JP	Z,LEFT
	CP	KY_CURSOR_DOWN
	JP	Z,RIGHT
	LD	B,0
	CP	KY_CLEAR_LEFT
	JP	Z,BACK
	CP	KY_START_OF_LINE
	JP	Z,LEFT
	CP	KY_CLEAR_RIGHT
	JP	Z,DELETE
	CP	KY_END_OF_LINE
	JP	Z,RIGHT
	LD	C,0		;INHIBIT REPEAT
	CP	'P' & 1FH
	JP	Z,TOGGLE
	CP	KY_BACKSPACE
	JR	Z,BACK
	CP	KY_CURSOR_LEFT
	JR	Z,LEFT
	CP	KY_DELETE
	JR	Z,DELETE
	CP	KY_INSERT
	JR	Z,INSERT
	CP	KY_CURSOR_RIGHT
	JP	Z,RIGHT
	CP	' '		;PRINTING CHARACTER
	JP	NC,SAVECH
	CP	CR		;ENTER LINE
	RET	NZ
OSEXIT:	LD	A,(HL)
	CALL	OSWRCH		;WRITE REST OF LINE
	INC	HL
	SUB	CR
	JR	NZ,OSEXIT
	POP	DE		;DITCH RETURN ADDRESS
	CP	C
	JP	NZ,ABORT	;ESCAPE
	LD	A,LF
	CALL	OSWRCH
	LD	DE,(ERRLIN)
	XOR	A
	LD	L,A
	LD	(EDPTR),HL
	CP	D
	RET	NZ
	CP	E
	RET	NZ
	LD	DE,EDITST
	LD	B,4
CMPARE:	LD	A,(DE)
	CP	(HL)
	LD	A,0
	RET	NZ
	INC	HL
	INC	DE
	LD	A,(HL)
	CP	'.'
	JR	Z,ABBR
	DJNZ	CMPARE
ABBR:	XOR	A
	LD	B,A
	LD	C,L
	LD	L,A
	LD	DE,LISTST
	EX	DE,HL
	LDIR
	LD	HL,FLAGS
	SET	3,(HL)
	RET
;
BACK:	SCF			;DELETE LEFT
LEFT:	INC	L		;CURSOR LEFT
	DEC	L
	JR	Z,STOP
	LD	A,BS
	CALL	OSWRCH
	DEC	L
	RET	NC
DELETE:	LD	A,(HL)		;DELETE RIGHT
	CP	CR
	JR	Z,STOP
	LD	D,H
	LD	E,L
DEL1:	INC	DE
	LD	A,(DE)
	DEC	DE
	LD	(DE),A
	INC	DE
	CP	CR
	JR	NZ,DEL1
DEL2:	POP	DE		;DITCH
	JP	UPDATE
;
INSERT:	LD	A,CR		;INSERT SPACE
	CP	(HL)
	RET	Z
	LD	D,H
	LD	E,254
INS1:	INC	DE
	LD	(DE),A
	DEC	DE
	LD	A,E
	CP	L
	DEC	DE
	LD	A,(DE)
	JR	NZ,INS1
	LD	(HL),' '
	JR	DEL2
;
RIGHT:	LD	A,(HL)		;CURSOR RIGHT
	CP	CR
	JR	Z,STOP
SAVECH:	LD	D,(HL)		;PRINTING CHARACTER
	LD	(HL),A
	INC	L
	JR	Z,WONTGO	;LINE TOO LONG
	CALL	OSWRCH
	LD	A,CR
	CP	D
	RET	NZ
	LD	(HL),A
	RET
;
WONTGO:	DEC	L
	LD	(HL),CR
	LD	A,BEL
	CALL	OSWRCH		;BEEP!
STOP:	LD	C,0		;STOP REPEAT
	RET
;
;
EDITST:	DEFM	"EDIT"
LISTST:	DEFM	"LIST"


;
; line editor cursor control (^) characters
KY_CURSOR_UP		EQU	'E' & 1FH	;CURSOR UP 11
KY_CURSOR_DOWN		EQU	'X' & 1FH	;CURSOR DOWN 10
KY_START_OF_LINE	EQU	'A' & 1FH	;START OF LINE  9
KY_END_OF_LINE		EQU	'F' & 1FH	;END OF LINE 8
KY_CLEAR_RIGHT		EQU	'T' & 1FH	;DELETE TO END OF LINE 7
KY_BACKSPACE		EQU	8H		;BACKSPACE & DELETE 6
KY_CLEAR_LEFT		EQU	'U' & 1FH	;CANCEL LINE 5
KY_CURSOR_LEFT		EQU	'S' & 1FH	;CURSOR LEFT 4
KY_CURSOR_RIGHT		EQU	'D' & 1FH	;CURSOR RIGHT 3
KY_DELETE		EQU	'G' & 1FH	;DELETE CHARACTER 2
KY_INSERT		EQU	'V' & 1FH	;INSERT CHARACTER 1

;
BEL	EQU	7
BS	EQU	8
HT	EQU	9
VT	EQU	0BH
DEL	EQU	7FH
;
BDOS	EQU	0x200005	; CP/M extended memory marshaller
;
FCBSIZ	EQU	128+36+2
;
	section	.data, "aw", @progbits

TIME:	DEFS	4
	XDEF	_TRPCNT
_TRPCNT:
TRPCNT:	DEFB	10
_TABLE:
TABLE:	DEFS	9*3		;FILE BLOCK POINTERS

	XDEF	_FREE_FCB_TABLE
_FREE_FCB_TABLE:
FREE_FCB_TABLE:	d24	FCB_BLOCKS
FLAGS:	DEFB	0
INKEY:	DEFB	0
EDPTR:	d24	0
_OPTVAL:
OPTVAL:	DEFB	0

SCREEN_TEXT_WIDTH:
	DB	80

	section	.bss_z80,"aw",@nobits

	XDEF	_FCB_BLOCKS
MAX_OPEN_FILES	EQU	7
_FCB_BLOCKS:
FCB_SPOOL:	DS	FCBSIZ
FCB_BLOCKS:	DS	FCBSIZ*MAX_OPEN_FILES

	section	.text, "ax", @progbits

; -----------------------------------------

	XDEF	PUTIME
	XDEF	GETIME
	XDEF	GETIMS
	XDEF	STAR_VERSION
	XDEF	PUTCSR
	XDEF	GETCSR

	XDEF	EXPR_W2

	; section	.text, "ax", @progbits

STAR_VERSION:
	CALL    TELL			; Output the welcome message
	DB    	"BBC BASIC eZ80 for RC (ADL) Version 1.03", 13, 10, 0
	RET

; *ASM string
;
STAR_ASM:		PUSH	IY			; Stack the BASIC pointer
			PUSH	HL			; HL = IY
			POP	IY
			CALL	ASSEM			; Invoke the assembler
			POP	IY
			RET

; PUTIME: set current time to DE:HL, in centiseconds.
;
PUTIME:
	RET

; GETIME: return current time in DE:HL, in centiseconds
;
GETIME:
	RET

; GETIMS - Get time from RTC
;
GETIMS:
	RET
; PUTCSR: move to cursor to x=DE, y=HL
;
PUTCSR:
	RET

; GETCSR: return cursor position in x=DE, y=HL
;
GETCSR:
	RET

; Get two word values from EXPR in DE, HL
; IY: Pointer to expression string
; Returns:
; DE: P1
; HL: P2
;
EXPR_W2:		CALL	EXPRI			; Get first parameter
			EXX
			PUSH	HL
			CALL	COMMA
			CALL	EXPRI			; Get second parameter
			EXX
			POP	DE
			RET


	.global	_FCB

	XREF	_CPM_SYS_FCB

FCB	=	_CPM_SYS_FCB
_FCB	=	_CPM_SYS_FCB

	XREF	_CPM_DMABUF
DSKBUF	=	_CPM_DMABUF

	section	.bss_z80,"aw",@nobits

	XDEF	_DISK_BUFFER
_DISK_BUFFER:
DISK_BUFFER:	DS	128		; RESERVE SOME SPACE IN THE CP/M 64K PAGE FOR READING/WRITING DATA TO DISK

