; Segmentation test
; Compile with the -e switch.

        .model small
	.stack 100h
	.code
main:	mov ds,data
	mov trial,ax
	jmp bollocks:msg

	.data
trial	dq ?,?,?,?

bollocks segment para
	db"Little gap"

msg:	mov dx,message
	mov ah,9
	mov ds,bollocks
	int 21h
	mov ax,4c00h
	int 21h

message:db"This is a small model program!",10,13,"$"


