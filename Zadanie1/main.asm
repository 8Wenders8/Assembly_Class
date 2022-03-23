LOCALS @@
.MODEL SMALL
.STACK	300h
;----------------------------------------------------------------
.DATA
TEXT_BUFFER	DB 25 dup(?) 			; Text buffer array for reading from the file
BUFFER_SIZE	DW 24				; Size of the buffer, how many bytes to read
BUFFER_OFFSET	DW 0				; Says how many times are we reading from file
OUTPUT		DB 80 dup(?)			; Output array 
CHARS_READ	DW ?				; Says how many bytes we actually read from the file
FILE_HANDLE	DW ?				; File handler for working with files
FILE_NAME	DB "in.txt", 0			; File name to read from
TABLE		DB "0123456789ABCDEF", 0	; Auxiliary table for turning ascii values to hex values
TOP		DB "INDEX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 "  
						; Top index row for readability
LF		EQU 0AH				; Constant Line Feed ( Moves cursor down a line )
CR		EQU 0DH				; Constant Carriage Return ( Sets cursor at the start of line )
SPC		EQU 20H				; Constant Space
;----------------------------------------------------------------

.CODE

NEW_LINE	MACRO
		MOV AH, 2H			; Prepare for character output
		MOV DL, CR			; Move Carriage Return to DL register
		INT 21H				; Interupt - DOS API 21H - CHARACTER STDOUT 2H
		MOV DL, LF			; Move Line Feed to DL register
		INT 21H				; Interupt - DOS API 21h - CHARACTER STDOUT 2H
		ENDM

F_OPEN		PROC
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
		MOV DX, OFFSET TEXT_BUFFER	; Prepare buffer for data - Store text buffer address to DX register
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


ASCII_TO_HEX	PROC
		; PREPARE REGISTERS
		PUSH BX				; Push BX register to stack
		PUSH CX				; Push CX register to stack	
		MOV BX, OFFSET TABLE		; Copy the address of TABLE array to BX

		; CALCULATE HEX VALUE
		MOV AH, AL			; Make AL and AH equal so we can isolate each half of the byte
		MOV CL, 4			; Move to CL the number of bits to shift
		SHR AH, CL			; Right shift 4 bits of the AH register, result is high part
		AND AL, 0FH			; Mask the upper 4 bits, result is low part
		XLAT				; Lookup AL's content in TABLE ( low part of byte )
		XCHG AH, AL			; Flip around the bytes
		XLAT				; Lookups AL's content in TABLE ( high part of byte )

		; SAVE HEX VALUE
		POP CX				; Restore CX register from stack
		POP BX				; Restore BX register from stack
		MOV [BX], AX			; Store calculated HEX value to the output array	
		RET
ASCII_TO_HEX	ENDP


ADD_SPACE	PROC
		MOV BYTE PTR [BX], SPC		; Store space (20H) at the current addres of BX
		INC BX				; Increment pointer to output array
		RET
ADD_SPACE	ENDP


STRING_TO_HEX	PROC
		; PREPARE REGISTERS
		MOV DX, OFFSET TEXT_BUFFER	; Move first address of text buffer array to DX
		MOV BX, OFFSET OUTPUT		; Move first address of output array to BX
		ADD BX, 7			; For numbers
		XOR CX, CX			; Reset CX register, counter of characters changed to hex

		@@LOOP_STRING:
		CMP CX, CHARS_READ		; Transformed all the characters from text buffer?
		JZ @@ADD_STRING_TERMINATOR	; If so, jump to add terminator	
		
		; STORE CURRENT CHAR TO AX 	( DX can't be used as a pointer ) 	
		PUSH BX				; Push BX register to stack
		MOV BX, DX			; Move current pointer to text buffer to BX
		MOV AX, [BX]			; Store value at that address to AX
		POP BX				; Restore BX from stack

		CALL ASCII_TO_HEX		; Call procedure ASCII_TO_HEX
		INC CX				; Increment the number of chars transformed
		INC DX				; Increment text buffer pointer
		ADD BX, 2			; Increment twice ( we added 2B ) output pointer
		CALL ADD_SPACE			; Add space after hex value
		JMP @@LOOP_STRING
		@@ADD_STRING_TERMINATOR:
		MOV BYTE PTR [BX], '$'		; Store string terminator to the current address of BX ( end of output ) 
		RET
STRING_TO_HEX	ENDP

PRINT_STRING	PROC
		MOV AH, 9H			; Prepare for displaying string
		INT 21H				; Interupt - DOS API 21H - DISPLAY STRING 9H
		RET
PRINT_STRING	ENDP

	
COLOR_STRING	PROC
		MOV AX, @DATA			; Move data segment address to AX register
		MOV ES, AX			; Move data segment address to ES segment
		MOV AH, 13H			; Prepare for writing string
		MOV BH, 0			; Prepare PAGE 
		MOV BL, 113			; Prepare color ( text-background * 16 + text-color )
		INT 10H				; Interupt - BIOS VIDEO ACCESS 10H - WRITE STRING 13H
		RET
COLOR_STRING	ENDP


NUM_TO_ASCII	PROC				; Move value to AX before call. Divides value of register by 10 and stores remainders to the stack
		; PREPARE REGISTERS		
		XOR CX, CX			; Reset CX register
		MOV SI, BX			; Copy BX to SI ( instead of stack )
		MOV BX, 10			; Move divisor (10) to BX register
		
		@@CALCULATE:
		XOR DX, DX			; Reset DX register, here is the value of remainder stored
		DIV BX				; Divide AX by BX
		PUSH DX				; Store remainder to the stack
		INC CX				; Increment counter of digits
		TEST AX, AX			; Test if quotient is zero ( result after division stored in AX )
		JNZ @@CALCULATE			; If not, continue loop

		; CALCULATE NUMBER OF SPACES
		PUSH CX				; Push CX register to stack
		MOV AX, 5			; Set AX to number 5 ( 5 digits max with 64KB file )
		SUB AX, CX			; Substract number of digits calculated from max digits (5)
		MOV CX, AX			; Move calculated number of spaces needed to CX register
		
		MOV BX, SI			; Restore BX register
		@@ADD_SPACES:
		CALL ADD_SPACE			; Call procedure ADD_SPACE, adds [CX] spaces to the output array 
		LOOP @@ADD_SPACES
		
		POP CX				; Restore CX register from stack

		@@TO_ASCII:
		POP DX				; Restore last remainder from the stack
		ADD DL, 30H			; Add 30H, which makes it a ASCII character of the number
		MOV [BX], DX			; Move last remainder to the output array
		INC BX				; Increment output array pointer
		LOOP @@TO_ASCII			; Loop until CX is zero
		
		MOV BYTE PTR [BX], ':'		; Add ':' char to the output array
		INC BX				; Increment output array pointer
		CALL ADD_SPACE			; Add space to the output array
		RET
NUM_TO_ASCII	ENDP


LINE_NUMBER	PROC
		; PREPARE REGISTERS
		MOV AX, [BUFFER_SIZE]		; Move buffer size to AX register
		MUL [BUFFER_OFFSET]		; Multiply by how many times file was read
		MOV BX, OFFSET OUTPUT		; Move address of output array to BX register
		CALL NUM_TO_ASCII		; Transform hex value to ascii and add it to output
		RET
LINE_NUMBER	ENDP	


CLEAR_SCREEN	PROC
		MOV AH, 2H			; Prepare for character output
		MOV DL, LF			; Move Line Feed to DL register
		MOV CX, 24			; How many lines to print

		@@CLS:
		INT 21H				; Interupt - DOS API 21H - CHARACTER STDOUT 2H
		LOOP @@CLS			; Repeat until CX=0
		RET	
CLEAR_SCREEN	ENDP


START:		MOV AX, @DATA			; Move data segment address to AX register
		MOV DS, AX			; Move data segment address to DS from AX register

		CALL F_OPEN			; Call procedure for opening file

		CALL CLEAR_SCREEN		; Call procedure for clearing the screen

		MOV BP, OFFSET TOP		; Prepare top row index string, required in BP register
		MOV DX, 0			; DL=0 ( x - screen position ) DH=0 ( y - screen position )
		MOV CX, 79			; Number of bytes to print
		CALL COLOR_STRING		; Call procedure for printing out colored strings
		NEW_LINE
MAIN_LOOP:
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
		CALL F_CLOSE			; Call procedure for closing file
		MOV AX, 4C00H			; Prepare for terminate with return code
		INT 21H				; Interup - DOS API 21H - TERMINATE WITH RETURN CODE 4CH
		END START	


		
