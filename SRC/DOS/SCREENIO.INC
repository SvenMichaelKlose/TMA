;TMA macro assembler
;Basic screen IO
; (c)1996 Sven Klose

	.code

; >>> Display current pass <<<
showpass:
	mov ax,w[lines]
	or ax,ax
	je showpass2
	cmp ax,1000
	jb >l1
showpass2:
	call showpass3
	mov al,13
	int 29h
l1:	ret

showpass3:
	mov b stdoutmode,1
	mov si,TXT_pass
	call asciiz
	mov al,b[pass]
	xor ah,ah
	call printword2
	mov b stdoutmode,0
	ret

PrintWord:
	push dx
	call printword2
	pop si
	call asciiz
	ret

printword_2d:
	cmp eax,9
	ja printword2
	push ax
	mov al,"0"
	call cout
	pop ax
printword2:
	mov bx,10
	xor cx,cx
p2:	xor dx,dx
	div bx
	push dx
	inc cx
	or ax,ax
	jne p2
p3:	pop dx
	mov al,dl
	add al,"0"
	call cout
	loop p3
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

printd: push dx
	mov cx,4
l1:	rol ax,4
	call prdigit
	loop l1
	pop si
	call asciiz
	ret

printhex:
	push dx
	call printhex2
	pop si
	call asciiz
	ret

printhex2:
	ror al,4
	call prdigit
	ror al,4
	call prdigit
	mov al," "
	call cout
	ret

prdigit:push ax,bx
	and al,0fh
	mov bl,"0"
	cmp al,10
	jb >n1
	mov bl,"A"-10
n1:	add al,bl
	call cout
	pop bx,ax
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

asciiz: lodsb
	or al,al
	je >l1
	call cout
	jmp asciiz
l1:	ret

cret:	mov al,13
	call cout
	mov al,10
	call cout
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

drline: mov cx,79
l1:	mov al,196
	call cout
	loop l1
	call cret
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Display current input line <<<
showline:
	pushf
	mov ds,fs
	mov si,line
	call asciiz
	call cret
	popf
	ret


show_msg:
	call get_list_entry
	mov si,di
	call asciiz
	ret

get_list_entry:
	mov dx,ax
	mov ax,fs
	mov es,ax
	dec dx
	jz >l1
l2:	mov cx,-1
	xor al,al
	repne scasb
	dec dx
	jnz l2
l1:	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

cout:  #if !_screen
	#if !_quiet_mode
	cmp b fs:stdoutmode,1
	je >l1
	push ax,dx
	mov dl,al
	mov ah,2
	int 21h
	pop dx,ax
        #endif
	#endif
	ret
l1:	int 29h
	ret

	.data
stdoutmode	db ?

