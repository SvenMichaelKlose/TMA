;TMA macro assembler
;Memory handling
; (c)1996 Sven Klose

	code segment
AllocMemErr:
	mov ax,3
	int 10h
	mov ah,9
	int 21h
	mov ax,4cffh
	int 21h

AllocMem:
	push ds,es
#if !_mdltiny
	mov ds,data
#else
	mov ds,fs
#endif
	call defragm	;defrag DOS memory
	mov cl,33
	mov al,128	;Test on 386 CPU
	shr al,cl
	jnz AllocMem2
	mov dx,ERR_no386
	jmp AllocMemErr
AllocMem4b:
	jmp AllocMem4

AllocMem2:
#if _mdltiny
	mov bx,3000h
	mov es,cs
	mov ah,4ah
	cmp b[flag_ed],2
	jmp >a1 ;jne >a1
	mov bx,3000h
a1:	int 21h
	jc AllocMem4b	;Error...
#endif
	db 66h
	xor bx,bx
	mov ah,48h	;Size of available memory
	mov bx,-1
	int 21h
	cmp al,8
	jne AllocMem4b
	mov w[DOSlen],bx
	mov ah,48h
	int 21h
	jc AllocMem4b
	mov w[dossegstart],ax
	mov w[dosstart],ax
	mov w[DOSptr],ax
	add ax,w[doslen]
	mov w[dosend],ax
	pop es,ds
	call rmb_APIinit
	ret

mmreset:int 19h

AllocMem4:
	push ax
	mov ax,3
	int 10h
	pop ax
	jmp DOSerrorRoutine

ReallocMem:
#if !_mdltiny
	mov ds,data
#else
	mov ds,cs
#endif
	mov ah,49h	;free DOS memory
	mov es,w[DOSstart]
	int 21h
	jc AllocMem4
	call rmb_APIdeinit
	ret

;Defragment memory
defragm:push ds
	mov ah,52h
	int 21h
	mov bx,w[es:bx-2]
	xor si,si
dfm2:	mov ds,bx
	call dfmcb
	jne dfm7
	mov dh,al
	cmp w[1],0
	jne dfm6
dfm3:	mov ax,ds
	inc ax
	add ax,cx
	mov ds,ax
	call dfmcb
	jne dfm4
	mov dh,al
	cmp w[1],0
	je dfm3
dfm4:	mov cx,ds
	sub cx,bx
	dec cx
	push ds
	mov ds,bx
	cmp cx,w[3]
	je dfm5
	inc si
	mov w[3],cx
	mov b[0],dh
dfm5:	pop bx
	jmp dfm2
dfm6:	inc cx
	add bx,cx
	jmp dfm2
dfm7:	pop ds
	ret

dfmcb:	mov al,b[0]
	cmp al,"M"
	je dfmcb2
	cmp al,"Z"
	jne dfmcb3
dfmcb2: mov cx,w[3]
dfmcb3: ret

#if !_mdltiny
_text	segment para
#endif

#if lang_english
ERR_no386:
	db"No 386-CPU detected. Time to upgrade !",10,13,"$"
ERR_nomem:
	db"Not enough memory. See documentation for details.",10,13,"$"
#endif
#if lang_german
ERR_no386:
	db"Keine 80386-CPU gefunden. Zeit zum aufrÅsten !",10,13,"$"
ERR_nomem:
	db"Zu wenig Speicher. Bitte lesen Sie im Handbuch nach.",10,13,"$"
#endif

einfotab:
	db"Register information",0
	db"EAX:",0," EBX:",0," ECX:",0," EDX:",0
	db 10,13," ESI:",0," EDI:",0
	db 10,13,"CS:",0," DS:",0," ES:",0," FS:",0," GS:",0
	db 10,13,"SS:",0," SP:",0," IP:",0," Flags:",0
	db 10,13,"Stack:",10,13,0

	.data
dossegstart:	dw ?
dosptr: 	dw ?
dosend: 	dw ?
doslen: 	dw ?
dosstart:	dw ?
;DOSsize:	dd ?
XMSdriver	dd ?

