;TMA macro assembler
;xrm byte generation
; (c)1996-1997 Sven Klose

; ======================================================
; >>> Generate ModRM opcode from operand info buffer <<<
; =====================================================
	.code
GetModRMbyte:
	mov dl,b[ealength]      ;Only constant?
	or dl,dl
	je >c1                  ;Yes...
	test b[eadesc],(_w+_d)*8
	je >e1                  ;Error: no BYTE register allowed...
	test b[eadesc],_w*8     ;WORD register?
	jne >g1                 ;Yes, generate 16bit EA...
	mov b[addrflag],1
	call geteaconstX
	cmp dh,2
	if e mov b[eaconstsize],_d ;32 bit doesn't WORD displacement; force DWORD
	dec dl                  ;Only 1 register in combination?
	jne >n1                 ;Nope...
	mov al,b[eadesc]        ;Get register code
	and al,7
	cmp al,4
	je >e2                  ;Error: Can't use SP register.
	;
	cmp b[ea_sirflag],1     ;Scaling wanted?
	jne >l2
	mov b[ea_r2],5          ;Lock base register.
	mov b[ea_m],4
	mov b[ea_i],al
	cmp b[eaconstflag],1    ;Constqant used?
	je >l1
	xor eax,eax
	mov [eaconst],eax
	mov b[eaconstflag],1
l1:     mov b[eaconstsize],_d
	mov b[ea_x],al
	ret
g1:     jmp get16bitmodrm
c1:     jmp >c1
	;
l2:     mov b[ea_m],al
	ret
	;
n1:     mov b[ea_m],4           ;2 registers used.
	mov b[ea_sirflag],1
	mov ax,w[eadesc]
	push ax
	and ax,707h
	cmp ah,5                ;Can't use BP as index.
	je >e6
	pop ax
	test ah,_d
	je >e5                  ;Error: Index register isn't of size DWORD
	and ax,707h
	mov b[ea_r2],al
	mov b[ea_i],ah
	ret

e1:     mov ax,em_bytesizedeareg
	jmp error
e4:     mov ax,em_eadblreg
	jmp errorhandler
e5:     mov ax,em_nodwscaling
	jmp errorhandler
e6:     mov ax,em_ebpbase
	jmp errorhandler
e8:     mov ax,em_invalid16single
	jmp errorhandler

c1:     mov b[ea_x],0
	mov b[ea_m],6
	mov al,b[eaconstsize]
	cmp al,_b
	jne >l2
	mov b[eaconstsize],_w
	jmp >l1
l2:     cmp al,_d
	jne >l1
	mov b[addrflag],1
l1:     ret

get16bitmodrm:
	cmp b[ea_sirflag],1     ;Scaling wanted?
	je e5                   ;Error: Not accessible in 16 bit EAs...
	mov bx,w[eadesc]        ;Load register codes
	cmp dl,1
	jne >n1
	;Single register
	cmp bl,_bp              ;Bp used?
	jne >n2
	cmp b[eaconstflag],1    ;+Constant?
	je >n2                  ;Yes, continue as usual....
	mov b[ea_x],1           ;Force [bp+0]
	mov b[ea_m],6
	mov b[eaconst],0
	mov b[eaconstsize],_b
	mov b[eaconstflag],1
	ret
	;
n2:     sub bl,8                ;Mask off size information.
	xor bh,bh
	mov al,b[ea161regs+bx]  ;Valid register used?
	cmp al,-1
	je e8                   ;No...
	jmp >t1

f1:     jmp e1

	;Base and index register
n1:     cmp bl,bh
	je >e2                  ;Error: Register accessed twice...
	jb >l3
	xchg bl,bh              ;Base register must be the first entry.
l3:     push bx
	sub bl,8                ;Mask off size information.
	xor bh,bh
	mov al,b[ea162bregs+bx]
	cmp al,-1               ;Right base register used?
	je >e3                  ;Error...
	pop bx
	mov bl,bh
	sub bl,8
	jc f1                   ;Error: Index register is of size BYTE.
	xor bh,bh
	or al,b[ea162iregs+bx]
	cmp al,-1               ;Right index register used?
	je >e7                  ;Error...
t1:     mov b[ea_m],al          ;Store M nibble
	call geteaconstx
	cmp b[eaconstsize],_d
	je >e8
	ret

geteaconstX:                    ;Create X nibble of displacement size.
	xor dh,dh
	cmp b[eaconstflag],1
	jne >l1                 ;No displacement used...
	mov al,b[eaconstsize]
	mov dh,2                ;(D)WORD displacement
	cmp al,_b
	jne >l1
	dec dh                  ;SBYTE displacement
l1:     mov b[ea_x],dh
	ret

e2:     mov ax,em_eaSPreg       ;Error: Can't access ESP register.
	jmp errorhandler
e3:     mov ax,em_invalid16base
	jmp errorhandler
e7:     mov ax,em_invalid16index
	jmp errorhandler
e8:     mov ax,em_dwea16disp
	jmp errorhandler

#if !_mdltiny
_text   segment para
#else
	.code
#endif
ea161regs:
	db -1,-1,-1,7,-1,6,4,5  ;Single 16 bit registers available.
ea162bregs:
	db -1,-1,-1,0,-1,2,-1,-1 ;Available base registers.
ea162iregs:
	db -1,-1,-1,-1,-1,-1,0,1 ;Available 16 bit index registers.

