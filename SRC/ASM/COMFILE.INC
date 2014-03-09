;TMA macro assembler
;COM file generation
; (c)1996 Sven Klose

	.code
; Create COM file of generated code.
;
; No arguments.
; Returns nothing.
create_comfile:
	xor ecx,ecx
	mov di,100h
	mov cx,w[segflags+seg_max]
	sub cx,di
	mov [sizeof_executable],ecx
	mov es,w[seg_code]
	call savefile
	ret


