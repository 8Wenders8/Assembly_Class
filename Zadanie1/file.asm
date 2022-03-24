LOCALS @@
.MODEL SMALL
.STACK 100h
ORG 100H
.DATA
GLOBAL	F_OPEN : PROC	
GLOBAL F_CLOSE : PROC
GLOBAL F_READ_LINE : PROC

EXTRN TEXT_POINTER : byte
EXTRN FILE_NAME : byte
EXTRN BUFFER_SIZE : word
EXTRN CHARS_READ : word
EXTRN FILE_HANDLE : word

.CODE

F_OPEN	PROC
		MOV AX, 3D00H			; Prepare for opening file for reading
		MOV DX, OFFSET FILE_NAME	; Prepare file name - Store required file name to DX register
		INT 21H				; Interupt - DOS API 21H - OPEN FILE 3DH - READ 00H
		MOV [FILE_HANDLE], AX		; Store obtained file handler to a variable
		RET
F_OPEN		ENDP

F_CLOSE		PROC
		MOV AX, 3E00H			; Prepare for closing the file
		MOV BX, [FILE_HANDLE]		; Prepare file handler - Store required file handler to BX register
		INT 21H				; Interupt - DOS API 21H - CLOSE FILE
		RET
F_CLOSE		ENDP

F_READ_LINE	PROC
		; READ FROM FILE
		MOV AX, 3F00H			; Prepare for reading from the file
		MOV BX, [FILE_HANDLE]		; Prepare file handler - Store required file handler to BX register
		;MOV DX, 0
		MOV DL, TEXT_POINTER		; Prepare buffer for data - Store text buffer address to DX register
		MOV CX, BUFFER_SIZE		; Prepare number of bytes to read - Store text buffer size to CX register
		INT 21H				; Interupt - DOS API 21H - READ FROM FILE 3FH

		; TEST IF EOF			
		MOV [CHARS_READ], AX		; Store number of bytes read to CHARS_READ
		TEST AX,AX			; Test if AX=0, means EOF ( End of file )
		JZ @@EXIT			; If AX=0 jump to exit
		
		; APPEND STRING TERMINATOR TO TEXT_BUFFER
		MOV BX, DX			; Copy text buffer address from DX to BX
		ADD BX, CHARS_READ		; Add number of bytes read, BX is now at the end of the array
		MOV BYTE PTR [BX], '$'		; At the last index store string terminator

		@@EXIT:				; TODO if read, close, seek error codes
		RET
F_READ_LINE	ENDP

START:
		MOV AX, 4C00H
		INT 21H
		END START
