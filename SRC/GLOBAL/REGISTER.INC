	.data
tab_modules:	dd 64 dup ?
ptr_modules	dd ?
	
	.code

init_registering:
	mov eax,tab_modules
	mov d fs:ptr_modules,eax
	ret

; This will deinitialize started modules when terminating TMA.

; In:	EAX	Function addr, segment will be global FS
register_module:
	pushad
	mov es,fs
	mov edi,ptr_modules
	cld
	stosd
	mov ptr_modules,edi
	popad
	ret

; >>> Exit program
; In:	AL	error code
_exit:	pushad
	pushf
	mov ds,fs
	std
	mov esi,ptr_modules
l1:	mov bx,[esi-4]
	push esi,ds,ax
	call bx
	pop ax,ds,esi
	std
	sub esi,4
	cmp esi,tab_modules
	jne l1
	cld
	mov si,err_noexitmod
	call asciiz
	popf
	popad
l1:	jmp l1		;Hang if we get here...

err_noexitmod:
	db"Didn't call exit module - idle now.",0

