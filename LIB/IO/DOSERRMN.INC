	.code
; **********************************************
; *** Gibt DOS-Fehler auf dem Bildschirm aus ***
; **********************************************
; AL=DOS-Fehlernummer

DOSerrorRoutine:
#if !_mdltiny
	mov ds,_text
#else
	mov ds,cs
#endif
	push ax
	mov dx,TXT_DOSerr
	mov ah,9
	int 21h
	pop ax
	push ax
	cmp ax,27h
	jb DOSerrrt1
	mov ax,27h
DOSerrRt1:
	mov cx,ax
	mov si,TXT_doserrors
DOSErrRt2:
	lodsb
	cmp al,"$"
	jne Doserrrt2
	dec cx
	jnz Doserrrt2
	mov dx,si
	mov ah,9
	int 21h
	call cret
	pop ax
	jmp _exit

#if !_mdltiny
_text	segment para
#endif
TXT_DOSerr:db"DOS: $"


