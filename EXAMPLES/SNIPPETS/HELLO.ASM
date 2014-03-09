; Hello world!

		MOV DX,msg
		MOV AH,9
		INT 21H
		MOV AX,4C00h
		INT 21H

	msg:	DB"Hello world!",10,13,"$"

