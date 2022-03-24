LOCALS @@
ORG 100H
.MODEL SMALL
.STACK 100H
.DATA
GLOBAL ADD_SPACE : PROC
GLOBAL CLEAR_SCREEN : PROC
LF		EQU 0AH				; Constant Line Feed ( Moves cursor down a line )
CR		EQU 0DH				; Constant Carriage Return ( Sets cursor at the start of line )
SPC		EQU 20H				; Constant Space
.CODE

ADD_SPACE	PROC
		MOV BYTE PTR [BX], SPC		; Store space (20H) at the current addres of BX
		INC BX				; Increment pointer to output array
		RET
ADD_SPACE	ENDP


CLEAR_SCREEN	PROC
		MOV AH, 2H			; Prepare for character output
		MOV DL, LF			; Move Line Feed to DL register
		MOV CX, 24			; How many lines to print

		@@CLS:
		INT 21H				; Interupt - DOS API 21H - CHARACTER STDOUT 2H
		LOOP @@CLS
		RET	
CLEAR_SCREEN	ENDP


START:
		MOV AX, 4C00H
		INT 21H
		END START
