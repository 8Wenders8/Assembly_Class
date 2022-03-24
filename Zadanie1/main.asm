LOCALS @@
.MODEL SMALL
.STACK	300h
ORG 100h
;----------------------------------------------------------------
.DATA
GLOBAL FILE_HANDLE : word
GLOBAL FILE_NAME : byte
GLOBAL BUFFER_SIZE : word
GLOBAL BUFFER_OFFSET : WORD
GLOBAL CHARS_READ : word
GLOBAL TEXT_POINTER : byte

GLOBAL TABLE : byte
GLOBAL OUTPUT_POINTER : byte
GLOBAL ADD_SPACE : PROC

TEXT_BUFFER	DB 25 dup(?) 			; Text buffer array for reading from the file
TEXT_POINTER	DB OFFSET TEXT_BUFFER
BUFFER_SIZE	DW 24				; Size of the buffer, how many bytes to read
BUFFER_OFFSET	DW 0				; Says how many times are we reading from file
OUTPUT		DB 80 dup(?)			; Output array 
OUTPUT_POINTER  DB OFFSET OUTPUT
CHARS_READ	DW ?				; Says how many bytes we actually read from the file
FILE_HANDLE	DW ?				; File handler for working with files
FILE_NAME	DB "in.txt", 0			; File name to read from
TABLE		DB "0123456789ABCDEF", 0	; Auxiliary table for turning ascii values to hex values
TOP		DB 'INDEX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ', 13, 10
						; Top index row for readability
LF		EQU 0AH				; Constant Line Feed ( Moves cursor down a line )
CR		EQU 0DH				; Constant Carriage Return ( Sets cursor at the start of line )
SPC		EQU 20H				; Constant Space
;----------------------------------------------------------------

.CODE
INCLUDE includes.inc


START:		MOV AX, @DATA			; Move data segment address to AX register
		MOV DS, AX			; Move data segment address to DS from AX register

		CALL F_OPEN			; Call procedure for opening file
		CALL CLEAR_SCREEN		; Call procedure for clearing the screen
		MOV BP, OFFSET TOP		; Prepare top row index string, required in BP register
		MOV DX, 0			; DL=0 ( x - screen position ) DH=0 ( y - screen position )
		MOV CX, 79			; Number of bytes to print
		CALL COLOR_STRING		; Call procedure for printing out colored strings
MAIN_LOOP:
		MOV TEXT_POINTER, OFFSET TEXT_BUFFER
		MOV OUTPUT_POINTER, OFFSET OUTPUT
		CALL F_READ_LINE		; Read line from file, move to TEXT_BUFFER
		JZ EXIT				; If EOF jump to exit

		CALL LINE_NUMBER		; Add the line number to the output array
		INC BUFFER_OFFSET		; Increment counter of how many times file was read
		CALL STRING_TO_HEX		; Transform TEXT_BUFFER to hex values, stored in OUTPUT
		
		MOV BP, OFFSET OUTPUT		; Prepare output array, required in BP register
		MOV DL, 0			; DL=0 ( x - screen position ), start of the line
		MOV DH, BYTE PTR [BUFFER_OFFSET]; DH=LINE_INDEX ( y - screen position ) 
		MOV CX, 6			; Max 5 digits + ':' = 6 bytes to print
		CALL COLOR_STRING		; Call procedure for printing out colored strings

		MOV DX, OFFSET OUTPUT		; Move address of the array output to DX register
		ADD DX, 6			; Offset the pointer of output array because first 6 bytes were already printed out
		CALL PRINT_STRING		; Print OUTPUT
		JMP MAIN_LOOP			; Repeat MAIN_LOOP
EXIT:	
		MOV AX, 4C00H			; Prepare for terminate with return code
		INT 21H				; Interup - DOS API 21H - TERMINATE WITH RETURN CODE 4CH
		END START	
