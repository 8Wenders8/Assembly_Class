LOCALS @@
.MODEL SMALL
.STACK 300H

.DATA
GLOBAL PG_CONTINUE : PROC
MSG		DB  "      Press any key to continue..                                                "
.CODE
EXTRN COLOR_STRING : PROC


PG_CONTINUE	PROC
			PUSH BP
			MOV BP, OFFSET MSG
			MOV CX, 79
			MOV DL, 0
			MOV DH, 24
			CALL COLOR_STRING
			POP BP
			
			MOV AH, 8H
			INT 21H
			RET
PG_CONTINUE	ENDP
			
START:
		MOV AX, 4C00H
		INT 21H
		END START