LOCALS @@
ORG 100
.MODEL SMALL
.STACK 100H
.DATA
GLOBAL LINE_NUMBER : PROC
GLOBAL NUM_TO_ASII : PROC
EXTRN BUFFER_SIZE : WORD
EXTRN BUFFER_OFFSET : WORD
EXTRN OUTPUT_POINTER : BYTE

.CODE
EXTRN ADD_SPACE : PROC

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
		XOR BX, BX
		MOV BL, OUTPUT_POINTER		; Move address of output array to BX register
		CALL NUM_TO_ASCII		; Transform hex value to ascii and add it to output
		RET
LINE_NUMBER	ENDP	


START:
		MOV AX, 4C00H
		INT 21H
		END START
