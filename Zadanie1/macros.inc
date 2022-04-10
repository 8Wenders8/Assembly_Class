NEW_LINE	MACRO
		MOV AH, 2H			; Prepare for character output
		MOV DL, CR			; Move Carriage Return to DL register
		INT 21H				; Interupt - DOS API 21H - CHARACTER STDOUT 2H
		MOV DL, LF			; Move Line Feed to DL register
		INT 21H				; Interupt - DOS API 21h - CHARACTER STDOUT 2H
		ENDM

NEW_PAGE 	MACRO
		CALL CLEAR_SCREEN	; Call procedure for clearing the screen
		PUSH BP				; Save BP to stack
		MOV BP, OFFSET TOP	; Prepare top row index string, required in BP register
		MOV DX, 0			; DL=0 ( x - screen position ) DH=0 ( y - screen position )
		MOV CX, 79			; Number of bytes to print
		CALL COLOR_STRING	; Call procedure for printing out colored strings 
		POP BP				; Restore BP
		ENDM

