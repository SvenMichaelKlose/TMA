;TMA macro assembler
;(c)1996-1998 Sven Klose
; Generates opcodes from generation pattern and pregenerated information

	.data
flg_set_reloc_addr:
		db ?	;true: store relocation info
codeptr:	dd ?	;Generator's code position and segment.
codeseg:	dw ?

	.code
createcode:
	call writeoverride
	cmp b[addrflag],1       ;ADDR: prefix ?
	jne >n1
	mov al,67h
	call writecode
n1:     cmp b[randflag],1       ;RAND: prefix ?
	jne >l1
	mov al,66h
	call writecode
l1:     mov di,si
	cmp w[currmnemdesc],t_test
	if e mov b[dbit],0      ;Don't add anything if TESTing
	shl b[dbit],1          ;Shift d-bit to the right position.
l10:    mov bl,[di]
	inc di
	xor bh,bh
n3:     cmp bl,x7m
	ja >n3
	cmp bl,x0m
	jb >n4
	mov ah,bl
	sub ah,x0m
	mov al,b[ea_x]
	shl al,3
	or al,ah
	shl al,3
	or al,b[ea_m]
	call writecode
	jmp l10
	
n4:     add bx,bx
	jmp w[dsc_jump+bx]
	
n3:     cmp bl,_end
	je createconstants      ;End of pattern.
	ja >e2                  ;Internal error: Damaged desciptor.
	cmp bl,processorflags
	jb l10
	cmp bl,customcpu
	jb >n2
	cmp b[processormode],bl
	jne >e1                 ;Error: Instruction not supported for the
				;       target CPU.
	jmp l10
n2:     cmp b[processormode],bl
	jae l10                 ;Supported instruction, continue...
e1:     mov si,em_processorver  ;First part of the error message.
	call asciiz
	mov di,processornames   ;Display current target CPU.
	mov al,b[processormode]
	sub al,processorflags-2
	xor ah,ah
	call show_msg
	mov si,cpu              ;Second part of the error message.
	call asciiz
	mov ax,em_cpu
	jmp error

e2:     mov ax,em_descdamage
	jmp error

dsc_o:  mov al,[di]
	add al,b[wbit]
	add al,b[dbit]
	add al,b[condition]
	inc di
	call writecode
	jmp l10

dsc_ow: mov al,[di]
	inc di
	call writecode
	mov al,[di]
	add al,b[wbit]
	add al,b[dbit]
	add al,b[condition]
	inc di
	call writecode
	jmp l10

dsc_os: mov al,[di]
	inc di
	call writecode
	;
dsc_s:
s1:     mov cl,3
	xor ah,ah
s2:     mov al,[di]
	inc di
	cmp al,"x"	  ;x char
	jne >n3
	mov al,b[ea_x]
	jmp >n4
n3:     cmp al,"r"
	jne >n3
	mov al,b[ea_r]
	jmp >n4
n3:     cmp al,"m"
	jne >n3
	mov al,b[ea_m]
	jmp >n4
n3:     cmp al,"s"
	jne >n3
	mov al,b[segregister]
	jmp >n4
n3:     cmp al,"n"
	jne >n3
	mov al,b[sysrnum]
	jmp >n4
n3:     cmp al,"p"
	jne >n3
	mov al,b[pflag]
	jmp >n4
l100:   jmp l10
n3:     sub al,"0"
n4:     shl ah,3
	or ah,al
	dec cl          ;get three digits
	jnz s2
	mov al,ah
	add al,b[dbit]
	add al,b[wbit]
n5:     call Writecode
	jmp l10

dsc_xrm:mov al,b[ea_x]
	shl al,3
	or al,b[ea_r]           ;<- get register from OPDESC !!!
	shl al,3
	or al,b[ea_m]
	jmp n5

dsc_xpm:mov al,b[ea_x]
	shl al,3
	or al,b[pflag]
	shl al,3
	or al,b[ea_m]
	jmp n5

dsc_xsm:mov al,b[ea_x]
	shl al,3
	or al,b[segregister]
	shl al,3
	or al,b[ea_m]
	jmp n5

l101:   jmp l100

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
	; Ende des Opcodes
createconstants:
	cmp b[ea_sirflag],1     ;Write SIR?
	jne >l3
	cmp b[addrflag],1
	jne >l3                 ;No, normal XRM
	mov al,b[ea_i]
	shl al,3
	or al,b[ea_r2]
	or al,b[ea_scaling]
	call writecode

l3:     cmp b[eaconstflag],1
	jne >n1
	mov cl,b[eaconstsize]
	mov di,offset eaconst
	call writeconst

n1:     cmp b[constflag],1
	jne >n1
	cmp b[flg_set_reloc_addr],1
	jne >v1
	mov es,w[seg_reloc]     ;Save relocation position.
	mov bp,[reloc_items]
	shl bp,2
	mov bx,[lastsegflag]    ;(current position from filestart)
	mov edx,seg_pos[bx]
	inc w[reloc_items]
	;
v1:     mov cl,b[constsize]
	cmp cl,af               ;Absolute Far ?
	jne >d1
	push bp,es,edx
	mov di,offset constant  ;Yes, create address.
	mov cl,_w               ;(32 bit)
	cmp b[addrflag],1
	jne >d2
	mov cl,_d               ;(48 bit)
d2:     call writeconst
	pop edx,es,bp
	add edx,[codeptr]
	mov ebx,edx
	shr ebx,4
	and dx,15
	push bx,dx
	pop edx
	mov [es:bp],edx
	mov b[flg_set_reloc_addr],0
	mov cl,_w
	mov di,offset afseg     ;(segment/selector)
	call writeconst
	stc
	ret

d1:     cmp b[flg_set_reloc_addr],1
	jne >v1
	add edx,[codeptr]
	mov ebx,edx
	shr ebx,4
	and dx,15
	push bx,dx
	pop edx
	mov [es:bp],edx
	mov b[flg_set_reloc_addr],0

v1:	mov di,offset constant
	call writeconst
n1:	mov ax,w[currmnemdesc]	;Instruction is a prefix,
	cmp ax,t_prefixstart	; continue with rest of line.
	jb >n3
	cmp ax,t_prefixend
	jb >n2
n3:	cmp b[flag_contline],1	;Continue with rest of line,
	je >n3			; continue with same mnemonic.
	stc
	ret
	;
n3:	call init_continueline	;Init code generation.
	xor bp,bp
	mov si,w[lineptr]
	jmp contline
n2:	mov si,w[lineptr]
	jmp parse1

writeoverride:
	mov bl,b[override]
	or bl,bl
	js >n1
	xor bh,bh
	mov al,b[overridetab+bx]
	call writecode
n1:	ret

; Write constant to file
; CL=Type
; DI=Value
Writeconst:
	cmp cl,_b
	jne >l1
	mov cl,1
	jmp >l2
l1:	cmp cl,_w
	jne >l1
	mov cl,2
	jmp >l2
l1:	cmp cl,_d
	jne >l4
l3:	mov cl,4
	jmp >l2
l4:	inc cl
l2:	xor ch,ch
l1:	mov al,[di]
	inc di
	call writecode
	loop l1
	ret

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

writecode:
	cmp b[fs:generation_mode],_data
	je >l1
	push es,di
	mov es,w[fs:seg_code]
	mov di,w[fs:codeptr]
	stosb
	mov w[fs:codeptr],di
	pop di,es
	ret
l1:	inc w[fs:codeptr]
	ret

writerand:
	push ax
	cmp b[randflag],1
	jne >l1
	mov al,66h
	call writecode
l1:	pop ax
	ret

writeaddr:
	push ax
	cmp b[addrflag],1
	jne >l1
	mov al,67h
	call writecode
l1:	pop ax
	ret

