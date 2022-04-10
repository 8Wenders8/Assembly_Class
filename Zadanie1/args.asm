LOCALS @@ 
.MODEL SMALL
.STACK 100H

.DATA
GLOBAL ARG_READ 		: PROC
GLOBAL B_PG				: BYTE
GLOBAL B_REV			: BYTE
EXTRN FILE_POINTER 		: BYTE 
ARG_SIZE 	DB ?				; Length of arguments string
ARGS		DB 128 dup(?)				; Arguments array
READ		DB 0
HELP		DB " -h "
PG			DB " -p "
REVERSE		DB " -r "
COUNTER		DB 0	
ITERATIONS	DB 0
B_HELP		DB 0
B_PG		DB 0
B_REV		DB 0
ERR_NO_FILE	DB "Filename missing..$" 
ERR_LONG	DB "Filename is too long..$"
HELP_MSG	DB "File hexifier by Matej Volansky",13,10,"Format: hex [OPTIONS] [FILE_NAME]", 13, 10, "Program supports filenames up to 15 characters '.txt' included.", 13, 10, "Options:", 13, 10, "h - Help", 13, 10, "p - Output with pagging", 13, 10, "r - Reversed output", 13, 10, '$'

.CODE

NO_FILE		PROC
			MOV DX, OFFSET ERR_NO_FILE				
			MOV AX, 0900H
			INT 21H
			RET
NO_FILE		ENDP

ARG_LOAD	PROC
			XOR AX, AX
			XOR CX, CX
			MOV CL, BYTE PTR ES:[80H]
			MOV ARG_SIZE, CL
			MOV BX, 81H
			MOV AX, OFFSET ARGS

			TEST CX, CX
			JZ @@END

			@@LOOP: 
			MOV DL, BYTE PTR ES:[BX]
			PUSH BX
			MOV BX, AX
			MOV BYTE PTR [BX], DL 
			POP BX
			INC AX
			INC BX
			LOOP @@LOOP

			MOV BX, AX
			MOV BYTE PTR [BX], '$'
			@@END:
			INC READ
			RET
ARG_LOAD	ENDP


ARGIFY		PROC
			MOV AX, DS
			MOV ES, AX
			@@INIT:
			XOR DX, DX
			MOV SI, OFFSET ARGS
			MOV DL, ARG_SIZE
			
			SUB DX, 3
			CMP DX, 0
			JL @@EXIT				
			

			@@LOOP:
			MOV BX, OFFSET HELP
			ADD BL, COUNTER
			MOV DI, BX
			MOV CX, 4
			CLD
			MOV AX, SI
			REPE CMPSB
			JZ @@FOUND
			INC AX
			MOV SI, AX
			DEC DX
			AND DX, DX
			JNZ @@LOOP

			; NOT FOUND
			ADD COUNTER, 4
			CMP ITERATIONS, 2			
			JE @@EXIT
			@@CONTINUE:
			INC ITERATIONS
			JMP @@INIT
			
			MOV DX, 'N'
			MOV AH, 2H
			INT 21H
			JMP @@EXIT

			
			@@FOUND:
			MOV AL, ITERATIONS

			CMP AL, 0
			JE @@HLP
			CMP AL, 1
			JE @@PG
			CMP AL, 2
			JE @@REV
		
			@@HLP:
			INC B_HELP
			JMP @@CONTINUE	
			
			@@PG:
			INC B_PG
			JMP @@CONTINUE
	
			@@REV:
			INC B_REV
		
			@@EXIT:
			RET
ARGIFY		ENDP


GET_FNAME	PROC
			MOV CL, ARG_SIZE
			MOV BX, OFFSET ARGS
			TEST BX, BX
			JZ @@LOOP
			DEC BX

			@@LOOP:
			INC BX	
			DEC CL
			JZ @@ERR
			CMP BYTE PTR [BX], ' '
			JE @@LOOP
			CMP BYTE PTR [BX], '-'
			JNE @@FILENAME
			
			@@SKIP:
			INC BX
			CMP BYTE PTR [BX], ' '
			JNE @@SKIP
			
			@@SPACE:
			INC BX

			@@FILENAME:
			XOR AX, AX
			MOV AL, FILE_POINTER
			MOV SI, BX
			MOV DI, AX
			MOV CX, 15

			@@COPY:
			CLD
			MOVSB			
			CMP BYTE PTR [SI], ' '
			JE @@ZERO_TERMINATE
			DEC CX
			JNZ @@COPY
			
			@@ZERO_TERMINATE:
			MOV BYTE PTR [SI], 0
			JMP @@EXIT

			@@ERR:
			CALL NO_FILE
			XOR AX, AX
			RET

			@@EXIT:
			TEST SP, SP
			RET
GET_FNAME 	ENDP
			

ARG_READ	PROC
			MOV AL, READ
			TEST AL, AL
			JZ @@LOAD
			JMP @@CHECK

			@@LOAD:
			CALL ARG_LOAD

			@@CHECK:
			MOV AL, ARG_SIZE
			TEST AL, AL
			JE @@NO_FILE
			JMP  @@READ

			@@NO_FILE:
			CALL NO_FILE
			RET

			@@READ:
			CALL ARGIFY
			MOV AL, B_HELP
			TEST AL, AL
			JNZ @@HELP

			CALL GET_FNAME
			TEST SP, SP	
			RET

			@@HELP:
			MOV DX, OFFSET HELP_MSG
			MOV AH, 9H
			INT 21H
			XOR AX, AX
			TEST AX, AX	
			RET
ARG_READ	ENDP	




START: 
		MOV AX, 4C00H
		INT 21H
		END START

