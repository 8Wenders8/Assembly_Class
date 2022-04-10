ORG 100
LOCALS @@
.MODEL SMALL
.STACK 200H
.DATA
GLOBAL STRING_TO_HEX : PROC
;GLOBAL ASCII_TO_HEX : PROC
EXTRN TABLE : byte
EXTRN CHARS_READ : word 
EXTRN OUTPUT_POINTER : byte
EXTRN TEXT_POINTER : byte

.CODE

EXTRN ADD_SPACE : PROC

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


STRING_TO_HEX	PROC	 			; DX - INPUT, BX - OUTPUT
		; PREPARE REGISTERS
		XOR BX, BX
		XOR CX, CX			; Reset CX register, counter of characters changed to hex
		XOR DX, DX

		MOV DL, TEXT_POINTER		; Move first address of text buffer array to DX
		MOV BL, OUTPUT_POINTER		; Move first address of output array to BX
		ADD BX, 7			; For numbers

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

START:
		MOV AX, 4C00H
		INT 21H
		END START
