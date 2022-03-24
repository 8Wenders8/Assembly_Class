LOCALS @@
.MODEL SMALL
.STACK 100h
ORG 100H
.DATA
GLOBAL PRINT_STRING : PROC
GLOBAL COLOR_STRING : PROC
.CODE
	
PRINT_STRING	PROC
		MOV AH, 9H			; Prepare for displaying string
		INT 21H				; Interupt - DOS API 21H - DISPLAY STRING 9H
		RET
PRINT_STRING	ENDP

	
COLOR_STRING	PROC
		MOV AX, @DATA			; Move data segment address to AX register
		MOV ES, AX			; Move data segment address to ES segment
		MOV AH, 13H			; Prepare for writing string
		MOV AL, 1
		MOV BH, 0			; Prepare PAGE 
		MOV BL, 113			; Prepare color ( text-background * 16 + text-color )
		INT 10H				; Interupt - BIOS VIDEO ACCESS 10H - WRITE STRING 13H
		RET
COLOR_STRING	ENDP

START:	
		MOV AX, 4C00H
		INT 21H
		END START
