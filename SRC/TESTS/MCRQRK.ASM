blah=1

prints	macro str
	mov dx,str
	mov ah,9
	int 21h
	endm

start:	mov ah,2
	mov dl,'!'
	int 21h
	prints msg
Adios:
	mov ax,4c00h
	int 21h

msg	db"Hello!",10,13,"$"
