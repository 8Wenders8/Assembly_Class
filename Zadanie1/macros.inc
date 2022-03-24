NEW_LINE	MACRO
		MOV AH, 2H			; Prepare for character output
		MOV DL, CR			; Move Carriage Return to DL register
		INT 21H				; Interupt - DOS API 21H - CHARACTER STDOUT 2H
		MOV DL, LF			; Move Line Feed to DL register
		INT 21H				; Interupt - DOS API 21h - CHARACTER STDOUT 2H
		ENDM


