;--------------------------------------------------------
LOCALS @@
.MODEL SMALL
.STACK	300h
.386
;--------------------------------------------------------
;ORG 100h
.DATA
; GLOBAL VARIABLES FOR EXTERN ASM FILES
GLOBAL FILE_POINTER		: BYTE
GLOBAL TEXT_POINTER 	: BYTE
GLOBAL TABLE 			: BYTE
GLOBAL OUTPUT_POINTER 	: BYTE
GLOBAL FILE_HANDLE 		: WORD			
GLOBAL BUFFER_SIZE 		: WORD
GLOBAL BUFFER_OFFSET 	: WORD
GLOBAL CHARS_READ 		: WORD
GLOBAL ADD_SPACE 		: PROC
EXTRN B_PG				: BYTE
EXTRN B_REV				: BYTE
; DECLARATION OF VARIABLES
TEXT_BUFFER		DB 25 dup(?) 					; Text buffer array for reading from the file
TEXT_POINTER	DB OFFSET TEXT_BUFFER			; Pointer to the text buffer array
BUFFER_SIZE		DW 24							; Size of the buffer, how many bytes to read
BUFFER_OFFSET	DW 0							; Says how many times are we reading from file
OUTPUT			DB 80 dup(?)					; Output array 
OUTPUT_POINTER  DB OFFSET OUTPUT				; Pointer to the output array
CHARS_READ		DW ?							; Says how many bytes we actually read from the file
FILE_HANDLE		DW ?							; File handler for working with files
FILE_NAME		DB 16 dup(?)
FILE_POINTER	DB OFFSET FILE_NAME
LINE_NUM		DW 0
;FILE_NAME		DB "in.txt", 0					; File name to read from
TABLE			DB "0123456789ABCDEF", 0		; Auxiliary table for turning ascii values to hex values
TOP				DB 'INDEX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ', 13, 10
												; Top index row for readability
;--------------------------------------------------------

.CODE
INCLUDE includes.inc


START:											
		MOV AX, @DATA							; Move data segment address to AX register
		MOV DS, AX								; Move data segment address to DS from AX register
	
		CALL ARG_READ
		JZ EXIT
		
		CALL F_OPEN								; Call procedure for opening file
		JZ EXIT

		NEW_PAGE

MAIN_LOOP:
		MOV AX, LINE_NUM
		CMP AX, 0
		JZ PROCESS


		MOV DL, B_PG
		TEST DL, DL
		JZ PAGE_OVERFLOW

		CMP AX, 23
		JNZ PROCESS 
		CALL PG_CONTINUE
		JMP NEXT_PAGE

PAGE_OVERFLOW:
		CMP AX, 24
		JNZ PROCESS

NEXT_PAGE:
		NEW_PAGE
		MOV LINE_NUM, 0

PROCESS: 
		
		MOV TEXT_POINTER, OFFSET TEXT_BUFFER	; Set a pointer to the text buffer address
		MOV OUTPUT_POINTER, OFFSET OUTPUT		; Set a pointer to the output address
		CALL F_READ_LINE						; Read line from file, move to TEXT_BUFFER
		JZ CLOSE									; If EOF jump to exit

		CALL LINE_NUMBER						; Add the line number to the output array
		INC BUFFER_OFFSET						; Increment counter of how many times file was read
		INC LINE_NUM
		CALL STRING_TO_HEX						; Transform TEXT_BUFFER to hex values, stored in OUTPUT
		
		MOV BP, OFFSET OUTPUT					; Prepare output array, required in BP register
		MOV DL, 0								; DL=0 ( x - screen position ), start of the line
		MOV DH, BYTE PTR [LINE_NUM]				; DH=LINE_INDEX ( y - screen position ) 
		MOV CX, 6								; Max 5 digits + ':' = 6 bytes to print
		CALL COLOR_STRING						; Call procedure for printing out colored strings

		MOV DX, OFFSET OUTPUT					; Move address of the array output to DX register
		ADD DX, 6								; Offset the pointer of output array because first 6 bytes were already printed out
		CALL PRINT_STRING						; Print OUTPUT
		JMP MAIN_LOOP							; Repeat MAIN_LOOP
CLOSE:
		CALL F_CLOSE
EXIT:	
		MOV AX, 4C00H							; Prepare for terminate with return code
		INT 21H									; Interupt - DOS API 21H - TERMINATE WITH RETURN CODE 4CH
		END START	

