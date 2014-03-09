	.code
; *************************
; *** IRQ Vektor setzen ***
; *************************
; AL=interrupt no.
Switchirq:
        pusha
	push ds,es
        mov ax,cs
        mov ds,ax
        mov al,b[intvec]        ;Alten IRQ lesen
	mov ah,35h
	push ax
	int 21h
	mov ds,es
	mov dx,bx
        push ds,w[cs:irqseg]
        pop ds,w[cs:irqseg]
        push dx,w[cs:irqadr]
        pop dx,w[cs:irqadr]
	pop ax
	mov ah,25h              ;Neuen IRQ schreiben
	int 21h
	pop es,ds
	popa
	ret

; **********************
; *** PIC einstellen ***
; **********************
; AL=interrupt no.
SetIRQ: push ax
	cmp al,7
	ja setirq2
	add al,8
	jmp setirq3
setirq2:add al,70h-8
setirq3:mov b[cs:intvec],al
	;
	mov ah,0feh
	pop cx
	cmp cl,8
	jb SetIRQ4
	sub cl,8
	in al,0a1h
	mov b[cs:oldirqmask],al
	shl ah,cl
	and al,ah
;	 out 0a1h,al
	ret

SetIRQ4:in al,21h
	mov b[cs:oldirqmask],al
	shl ah,cl
	and al,ah
;	 out 21h,al
	ret

; **********************
; *** IRQ abschalten ***
; **********************
LockIRQ:
	mov al,b[cs:oldirqmask]
	cmp b[cs:irqNr],7
	jbe LockIRQ2
;	 out 0a1h,al
	ret
LockIRQ2:
;	 out 21h,al
	ret

	.data
irqnr:		db ?
intvec: 	db ?
oldIrqMask:	db ?
oldirqtab:	dd ?,?,?
irqadr: 	dw ?
irqseg: 	dw ?

