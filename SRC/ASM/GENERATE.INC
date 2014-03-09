;TMA macro assembler
;Code generation
; (c)1996-1998 Sven Klose

	.data
generation_mode:dw ?	;true: Generate AND write code to the output buffer.
processormode:	db ?	;Destination CPU (limits instruction set)

; >>> Generate opcode from generation info and operand info buffer
	.code
	align 4
getol1  macro                   ;Get matching generation pattern.
	call _getol1
	#em
_getol1:
	cmp b[si],0
	jz >g41                 ;Jump instruction, don't check if sizes match...
g4:     lodsb
	mov ah,al
	and ah,nolbits          ;Mask off # of operands
	jz >g41                 ;Jump...
	mov dl,al
	and dl,opbits
	cmp dl,soo
	je >g41                 ;Exception 'soo'
	and al,olbits
	cmp al,ol1
	je >g2
g3:     lodsb                   ;Get next entry.
	or al,al
	je g4
	cmp al,_end
	jne g3
	mov ax,em_illegaloperand ;Generation pattern not found.
	jmp error
g2:     mov al,ah
	and al,transbits
	cmp al,_d               ;opcode  with w/d-bit
	jbe >g5
	cmp al,nb               ;Fixed size? (b/w/d)
	jb >g90                 ;No, it's wb,dw or bw.
	sub al,nb
g90:    cmp al,b[operandsize]   ;Sizes must match.
	jne g3
	jmp >g40

g5:     cmp al,b[operandsize]
	ja g3                   ;Wrong size, try next entry.
	xor bx,bx               ;wbit/randflag=0
	mov dl,al
	mov al,b[operandsize]
	cmp dl,_b
	jne >g20
	cmp al,_b               ;BYTE size?
	je >g20
	inc bl                  ;w bit=true
g20:    cmp al,_d
	jne >g30
	ja >g30
	inc bh                  ;Dword, RAND prefix needed
g30:    mov b[wbit],bl
	mov b[randflag],bh
g40:    and ah,ntransbits       ;right size, mask off size information
	mov al,ah               ;(Keep size info on 'soo')
	jmp >g42
g41:    mov al,ah
g42:    ret ;#em

	align 4
getol2  macro
	call _getol2
	#em
_getol2:
g7:     lodsb                   ;Get instruction info.
	mov ah,al
	and al,olbits
	cmp al,ol2              ;Right # of operands?
	je >g8
g9:     lodsb
	or al,al
	jz g7
	cmp al,_end
	jne g9
	mov ax,em_illegaloperand
	jmp error
g8:     and ah,nolbits
	mov al,ah
	and al,transbits
	cmp al,_d
	jbe >g100               ;Fixed size...
g94:    cmp al,nb
	jb >g91
	sub al,nb
	jmp >g95
g91:    cmp b[eaflag],-2        ;No size info because of empty brackets [] ?
	jz >g11
g95:    cmp al,b[operandsize]   ;Sizes must match
	jne g9
	jmp >g11
g100:   cmp b[operandsize],_d
	ja g9                   ;Try next...
g10:    cmp b[operandsize],al
	jb g9                   ;"
	xor bx,bx
	mov dl,al
	mov al,b[operandsize]
	cmp dl,_b
	jne >g50
	cmp al,_b
	je >g50
	inc bl
g50:    cmp al,_d
	jne >g60
	ja >g60
	inc bh
g60:    mov b[wbit],bl
	mov b[randflag],bh
g11:    and ah,ntransbits       ;Right size, mask off size info
	mov al,ah
	ret;#em

	align 4
getol3  macro
	call _getol3
	#em
_getol3:
g12:    lodsb
	mov ah,al
	and al,olbits
	cmp al,ol3
	je g13
g14:    lodsb                   ;Try next entry.
	or al,al
	jz g12
	cmp al,_end
	jne g14
	mov ax,em_toomuchops
	jmp error
g13:    and ah,nolbits
	cmp b[operandsize],_w   ;word ?
	je g15
	cmp b[operandsize],_d   ;dword ?
	je g16
	mov ax,em_wordordwordrequired
	jmp error
g16:    mov b[randflag],1
g15:    mov al,ah
	and al,0e0h
	ret;#em

create_ea macro
	and #1,7
	mov b[ea_m],#1
	mov b[ea_x],3
	#em
create_r macro
	and #1,7
	mov b[ea_r],#1
	#em

; Search end of generation info.
	align 4
scdend  macro
	call _scdend
	#em
_scdend:
g90:    lodsb
	cmp al,0
	je >g80
	cmp al,_end
	jne g90
	mov ax,em_illegaloperand ;Generation info not found.
	jmp error
g80:    ret;#em


mapoperands:
	xor al,al
	mov b[wbit],al
	mov b[dbit],al
	mov si,w[currmnemdesc] ;Pointer to current generation info.
	mov al,b[oplength]      ;# of operands.
	mov cx,w[opdesc]        ;Operand type info
	cmp si,t_mov            ;MOV instruction?
	je moveinstruction
	or al,al
	je noop
	cmp al,1
	je oneop
	cmp al,2
	je twoops
	cmp al,3
	je threeops
	mov ax,em_toomuchops
	jmp error

; ---------------------------------------------------------------------------
	; Generate MOV instruction
	; There're lots of MOVe instructions whose opcodes differ from the
	; usual way they're generated. (e.g. access to system registers)
	; This is why the generation of them is hardcoded.
moveinstruction:
	push ax
	call writeoverride
	pop ax
	cmp al,2                ;2 operands in instruction?
	je >l1
	mov ax,em_twoopsrequired
	jmp error
l2:     mov ax,em_notsamesize
	jmp error

l1:     xor bl,bl
	mov al,b[operandsize]
	cmp al,_w
	je >n5
	cmp al,_d
	ja l2
	jne >n4
	mov b[randflag],1
n5:     mov bl,1
n4:     mov b[wbit],bl          ;store w-bit
	xor dl,dl               ;clear d-bit
	call writeaddr
	mov bl,cl
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_mov]

mov_err:mov ax,em_codest
	jmp error

mov_r:  mov bl,ch
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_mov_r]

mov_e:  mov bl,ch
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_mov_e]

mov_r2r:call writerand
	mov al,b[opdata+1]
	mov ah,b[opdata]
	mov b[dbit],0
	keymac
	push ax
	mov al,210o
	add al,b[wbit]
	shl dl,1
	add al,dl               ;d-bit
	call writecode
	pop ax
	and al,7
	shl al,3
	and ah,7
	or al,ah
	or al,300o
	call writecode
	jmp createconstants
	
	;MOV Rx,Ea
mov_r2e:mov al,b[opdata]        ;AL/AX/EAX ?
	and al,7
	or al,al
	jne >n1
	cmp w[ea_xm],6          ;Ar,[Dw] possible?
	jne >n1                 ;No, only Ar,[Ea]
	cmp b[addrflag],0       ;16-Bit-EA ?
	jne >n1                 ;No, try another...
	call writerand
	mov al,240o             ;Instruction found.
	add al,b[wbit]
	call writecode
	jmp createconstants

	;MOV Ea,Rx
mov_e2r:
d1:     mov al,b[opdata+1]      ;[Dw],Ar possible?
	and al,7
	or al,al
	jne >y1
	cmp b[addrflag],0       ;32bit ea ?
	jne >y1                 ;yes, rave on...
	cmp w[ea_xm],6
	jne >y1
	call writerand
	mov al,242o             ;Instruction found.
	add al,b[wbit]
	call writecode
	jmp createconstants

n1:     mov dl,2                ;set d-bit
y1:     add dl,210o
	call writerand
	mov al,dl
	xor dl,dl
	add al,b[wbit]
	call writecode
	mov al,b[ea_x]
	shl al,3
	or al,b[ea_r]
	shl al,3
	or al,b[ea_m]
	call writecode
	jmp createconstants
e1:     mov ax,em_notsamesize
	jmp error
	
	;Ea,constant ?
mov_e2c:mov al,b[operandsize]
	mov b[constsize],al
	cmp al,_d
	ja e1
	jne >x1
	mov al,66h
	call writecode
x1:     mov al,306o
	add al,b[wbit]
	call writecode
	mov al,b[ea_x]
	shl al,6
	or al,b[ea_m]
	call writecode
	jmp createconstants

	;MOV Rx,constant
mov_r2c:mov al,b[operandsize]
	mov b[constsize],al
#if opt_movr0
	mov bl,al
	mov eax,[constant]
	or eax,eax
	jne >j1
	cmp bl,_d
	if e call writerand
	mov al,60o
	cmp bl,_b
	if a inc al
	call writecode
	mov bl,b[opdata]
	and bl,7
	mov al,bl
	shl al,3
	or al,bl
	or al,300o
	call writecode
	stc
	ret
j1:     
#endif
	cmp b[operandsize],_b
	jne >n2
	mov al,b[opdata]
	and al,7
	or al,260o
	call writecode
	jmp createconstants

e1:     mov ax,em_notsamesize
	jmp error

n2:     cmp b[operandsize],_d        ;Byte, word or dword?
	ja e1
	jne >n3
	mov al,66h
	call writecode
n3:     mov al,b[opdata]
	and al,7
	or al,270o
	call writecode
	jmp createconstants

s2:     mov b[opdesc],_sr
	mov b[opdata],cl
s4:     mov w[currmnemdesc],ax
	mov b[segregister],cl
	shl cl,3                ;Segment code of XSM byte
	mov b[ea_s],cl
	mov b[oplength],1
	mov al,_w
	mov [operandsize],al
	mov [constsize],al
	call mapoperands
	stc
	ret

s1:     mov cx,w[opdata]
	xchg cl,ch
	push cx
	mov ax,t_pushs
	call s2
	pop cx
	mov ax,t_pops
	mov cl,ch
	cmp cl,1        ;CS register is destination?
	jz >e2          ;Yes, Error...
	jmp s2

s3:     mov ax,t_pushc   ;PUSH <constant>
	mov b[opdesc],co
	mov cl,b[segregister]
	push cx
	call s4
	pop cx
	mov ax,t_pops    ;POP <segreg>
	mov b[constflag],0
	jmp s2

e1:     mov ax,em_wordrequired  ;Segment register is always WORD.
	jmp error
e2:     mov ax,em_nocsload
	jmp error

mov_s:  cmp cx,_sr by _sr
	je s1
	cmp cx,co by _sr
	je s3
	xor al,al
	cmp cx,_sr by e  ;ea,sr
	je >o1
	cmp cx,_sr by r  ;r,sr
	je >o2
	cmp cx,r by _sr  ;sr,r
	je >o3
	cmp cx,e by _sr  ;sr,e
	jne >n2
	mov al,2                ;set d-bit
o1:     cmp b[operandsize],_w
	jne e1
	add al,214o
	call writecode
	mov al,b[ea_x]
	shl al,6
	or al,b[ea_s]
	or al,b[ea_m]
	call writecode
	jmp createconstants

e2:     jmp near e1

	;MOV Rx,SR      Rx->Ea
o2:     cmp b[operandsize],_w
	jne e1
	mov al,214o
	call writecode
	mov al,b[ea_s]           ;create xsm byte
	or al,300o
	or al,b[ea_r]
	call writecode
	stc
	ret

e2:     jmp near e1

	;MOV SR,Rx      Rx->Ea
o3:     cmp b[operandsize],_w
	jne e1
	mov al,216o
	call writecode
	mov al,b[ea_s]
	or al,300o
	or al,b[ea_r]
	call writecode
	stc
	ret

e1:     mov ax,em_cr1onlyreadable
	jmp error
e2:     mov ax,em_sysrisdword
	jmp error
	
	;MOV Rd,sysr/MOV sysr,Rd
mov_sysr:
n2:     mov ah,040o
	cmp cx,sysr by r
	je >o1
	cmp cx,r by sysr
	jne >n2                 ;Error: Illegal operands
	mov ah,042o
	cmp b[sysrtype],_cr     ;CR1 register used?
	jne >o1
	cmp b[sysrnum],1
	je e1                   ;Yes, only read from it.
o1:     add ah,b[sysrcode]
	cmp b[operandsize],_d   ;System registers are always DWORD.
	jne e2
	mov al,017o
	call writecode  
	cmp b[sysrtype],_cr4    ;Exception: CR4
	je >n2
	mov al,ah
	call writecode
	mov al,b[sysrnum]       ;create 3NR opcode
	shl al,3
	or al,300o
	or al,b[register]
	call writecode
	stc
	ret

n2:     cmp cx,sysr by r
	jne >n2
	mov al,22h
	call writecode
	jmp >l1
n2:     mov al,20h
	call writecode
l1:     mov al,340o
	or al,b[register]
	call writecode
	stc
	ret

; ---------------------------------------------------------------------------
	; Instruction without an operand
noop:
l1:     lodsb
	or al,al
	jz createcode          ;Generation info found, generate instruction.
	cmp al,_end
	jnz l1
	mov ax,em_operandrequired
	jmp error
n10:    jmp twoops
c1:     jmp createcode
; ---------------------------------------------------------------------------
	;Instructions with 1 operand.
oneop:  mov bl,cl
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_op1]

op1err: mov ax,em_sysr          ;Error: Only use system registers within a
	jmp error               ;MOV instruction.

	; Rx
l1:     scdend
op1r:   getol1                  ;Get first generation info.
	cmp al,sor              ;single operand/register ?
	je createcode
	cmp al,soe              ;Convert register into effective address?
	jne l1
c2:     mov b[ea_x],3
	mov al,b[ea_r]
	mov b[ea_m],al
c1:     jmp createcode

	; EA
l1:     scdend
op1e:   getol1
	cmp al,soe              ;single operand/effective address ?
	je createcode
	cmp al,som              ;single operand/memory pointer ?
	jne l1
	cmp b[ea_x],0
	jne l1
	cmp b[ea_m],6           ;[word constant]?
	jne l1
c1:     jmp createcode

	; CO
l1:     scdend
op1c:   getol1
	or al,al                ;Jump instruction?
	jne >n3
	mov ebx,[constant]
	sub ebx,[codeptr]       ;Get relative address to destination.
	mov ecx,ebx
	lodsb                   ;Get possible range.
	cmp al,_jb              ;+-128?
	jne >n4
	sub ecx,2
	cmp ecx,-80h
	jl >l10
	cmp ecx,7fh
	jg >l10
	mov b[constsize],_b     ;signed byte operand
h1:     mov [constant],ecx
	jmp createcode
n4:     xor eax,eax
	lodsb                   ;Correct adress.
	sub ecx,eax
	;cmp ecx,7fffh
	;jg >l10
	;cmp ecx,-8000h
	;jl >l10
	mov b[constsize],_w
	jmp h1

l10:    cmp w[currmnemdesc],t_jcxz
	jne l1
	cmp b[pass],4
	je h1
	xor ecx,ecx
	mov b[newpass],1
	jmp h1

n3:     cmp al,so3              ;constant value 3 ?
	jne >n3
	cmp b[constant],3
	jne l1
	mov b[constflag],0      ;Yes, avoid constant generation.
	jmp createcode
n3:     cmp al,soc              ;single operand/constant?
	je createcode
n3:     cmp al,soo_cc           ;single operand/signed byte ?
	jne l1
	cmp d[constant],7fh
	jg l1
	cmp d[constant],-80h
	jl l1
	mov b[constsize],_b
	jmp createcode

	; sr
l1:     scdend
op1s:   getol1
	cmp al,sos              ;single operand/segment register?
	jne >l2
	cmp b[segregister],segr_fs  ;FS & GS not permitted.
	jae l1
	jmp createcode
l2:     cmp al,soo_fs           ;FS register?
	jne >l2
	cmp b[segregister],segr_fs
	jne >l2
	jmp createcode
l2:     cmp al,soo_gs           ;GS register?
	jne l1
	cmp b[segregister],segr_gs
	jne l1
	jmp createcode

	; Af
l1:     scdend
op1af:  getol1
	cmp al,soo_af
	jne l1
	jmp createcode
ny2:    mov ax,em_illegaloperand
	jmp error

; ---------------------------------------------------------------------------
	;Two operands/instruction
twoops: mov bl,cl
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_op2]

op2errc:mov ax,em_codest
	jmp error
op2errs:mov ax,em_s
	jmp error
op2errsys:
	mov ax,em_sysr
	jmp error
op2erraf:
	mov ax,em_af
	jmp error

op2r:   mov bl,ch
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_op2r]

op2e:   mov bl,ch
	xor bh,bh
	add bx,bx
	jmp w[bx+jmp_op2e]

	; Ea,Rx
l1:     scdend
ope2r:  getol2
	cmp al,der              ;EA & r + d-Bit ?
	je createcode           ;Yes, create code, clear d-bit.
	cmp al,ero              ;'ero' order ?
	je >c1
	cmp al,eao              ;ea, ?
	jne l1
	cmp b[opdata+1],_cl     ;CL register used??
	jne l1
	cmp b[si],_cl           ;CL in generation info?
	jne l1
	cmp b[operandsize],_b   ;Yes, check size.
	jne l1
c1:     jmp createcode

l1:     scdend
opr2e:  getol2
	cmp al,der              ;der ?
	jne >n3
	mov b[dbit],1           ;(direction bit)
	jmp createcode
n3:     cmp al,reo              ;reo ?
	jne l1
	jmp createcode

	;Rx,Rx ?
w2:     inc si
	jmp >w1
l1:     scdend
opr2r:  cmp b[si],_b+ol2+eao
	je w2                   ;eao found, r+r combination not permitted.
	getol2
	cmp al,der              ;Opcode with d/w-bit?
	jne >n3
l2:     mov al,b[opdata]        ;Convert regiser to effective address-
	create_ea al
	mov al,b[opdata+1]      ;Generate r-digit.
	create_r al
	mov al,b[ea_r]
	mov ah,b[ea_m]
	xor dl,dl
	keymac
	mov b[ea_r],al
	mov b[ea_m],ah
	mov b[dbit],dl
	jmp createcode
	;
n3:     cmp al,ero
	je l2
	cmp al,reo
	jne >n3
	mov al,b[opdata+1]      ;Rx,Ep
	create_ea al
	mov al,b[opdata]
	create_r al
	jmp createcode

n3:     cmp al,eao
	jne l1
w1:     mov cx,2                ;Compare 2 operands.
	xor bp,bp
lx2:    lodsb
	cmp al,-1               ;1st Operand ok.
	je >l22
	cmp al,ar               ;AL/AX/EAX?
	je ol2x_ar
	cmp al,_dx              ;DX?
	je ol2x_dx
	cmp al,_cl
	je ol2x_cl
	jmp l1

ol2x_ar:mov ah,b[opdata+bp]
	and ah,7
	jnz l1
l22:    inc bp
	loop lx2
	jmp >l23

ol2x_dx:cmp b[opdata+bp],12o
	jne l1
	inc bp
	loop lx2
	jmp >l23

ol2x_cl:cmp b[opdata+bp],1      ;cl
	jne n1
	inc bp
	loop lx2
	test b[opdata],8+16     ;1st register WORD/DWORD?
	jz >l23
	mov b[wbit],1           ;Yes, set w-bit.
l23:    mov al,b[opdata]
	create_ea al
	jmp createcode

	;Ea,constant ?          ;Ea,const?
l1:     scdend
ope2c:  getol2
	cmp al,eco
	jne >n3
	mov al,b[operandsize]
	mov b[constsize],al
	jmp createcode
n3:     cmp al,eao
	jne >n3
	cmp b[operandsize],_b
	jne >n3
	cmp b[si],_1
	jne >n3
	cmp b[constant],1
	jne >n3
	mov b[constflag],0
	inc si
	jmp createcode
n3:     cmp al,ecco             ;Ea/signed byte ?
	jne l1
	cmp d[constant],-80h
	jl l1
	cmp d[constant],07fh
	jg l1
	mov b[operandsize],_b   ;Omit a single byte on generation.
	jmp createcode
l20:    jmp l1

	;Rx,constant
l1:     scdend
opr2c:  getol2
	cmp al,rco
	jne >q1
	mov al,b[operandsize]
	mov b[constsize],al
	jmp createcode

q1:     cmp al,eco
	jne >l10
a1:     mov al,b[ea_r]          ;Convert register to effective address.
	create_ea al
	mov al,b[operandsize]
	mov b[constsize],al
	jmp createcode

;n1:     jmp l1

l10:    cmp al,eao
	je >n3
	cmp al,ecco             ;signed byte const ?
	jne l1
	cmp d[constant],-80h
	jl l1
	cmp d[constant],07fh
	jg l1
	mov b[operandsize],_b
	mov b[constsize],_b
	jmp a1          ;Convert register to effective address and generate opcode.
x1:     jmp l1

n3:     mov cx,2                ;Compare 2 operand.
	xor bp,bp
b2:     lodsb
	cmp al,-1
	je >l22
	cmp al,ar               ;AL/AX/EAX?
	je ol2_ar
	cmp al,co               ;Constant?
	je ol2_co
	cmp al,_dx              ;DX?
	je ol2_dx
	cmp al,_1               ;Constant value 1?
	je ol2_1
	jmp l1

;l21:    jmp b2

ol2_ar: mov ah,b[opdata+bp]
	and ah,7
	jnz l1
l22:    inc bp
	loop b2
	jmp createcode
n1:     jmp l1

ol2_co: cmp b[opdesc+bp],co
	jne l1
	inc bp
	mov al,b[operandsize]
	mov b[constsize],al
	loop b2
	jmp createcode

ol2_dx: cmp b[opdata+bp],12o
	jne l1
	inc bp
	loop b2
	jmp createcode

ol2_1:  cmp b[constsize],_b
	jne l1
	cmp w[constant],1
	jne l1
	mov b[constflag],0
	mov al,b[ea_r]
	create_ea al
	jmp createcode

l1:     scdend
op2c:   getol2
	cmp al,eao
	jne l1
	mov cx,2                ;Compare 2 operands.
	xor bp,bp
b2:     lodsb
	cmp al,-1
	je >l22
	cmp al,ar               ;AL/AX/EAX
	je xol2_ar
	cmp al,co
	je xol2_co
	cmp al,_dx              ;DX
	je xol2_dx
	cmp al,_1
	je xol2_1
	jmp l1

xol2_ar:mov ah,b[opdata+bp]
	and ah,7
	jnz >n1
l22:    inc bp
	loop b2
	jmp createcode
n1:     jmp l1

xol2_co:cmp b[opdesc+bp],co
	jne l1
	inc bp
	loop b2
	jmp createcode

xol2_dx:cmp b[opdata+bp],12o
	jne l1
	inc bp
	loop b2
	jmp createcode

xol2_1: cmp b[constsize],_b
	jne l1
	cmp w[constant],1
	jne l1
	mov b[constflag],0
	mov al,b[ea_r]
	create_ea al
	jmp createcode

; ---------------------------------------------------------------------------
	;Three operands/instruction.
l1:     scdend
threeops:
d1:     getol3
	cmp al,ero
	jne >n2
	cmp cx,r by e
	je >o1
	cmp cx,r by r
	jne l1
	mov al,b[opdata]
	create_ea al
	mov al,b[opdata+1]
	create_r al
	jmp >o1

n2:     cmp al,reo
	jne >n2                 ;Error: Invalid operands.
	cmp w[opdesc],e by r
	je >o1
	cmp w[opdesc],r by r
	jne l1
	mov al,b[opdata+1]
	create_ea al
	mov al,b[opdata]
	create_r al
o1:     mov bl,ah
	and bl,transbits
	cmp b[constflag],1
	jne >n3
	cmp bl,cc3
	jne >n2
	cmp d[constant],-80h
	jl l1
	cmp d[constant],7fh
	jg l1
	mov b[constsize],_b
	jmp createcode

n2:     cmp bl,cb3
	jne >n2
	cmp d[constant],0ffh
	ja l1
	mov b[constsize],_b
	jmp createcode

n2:     cmp bl,cw3
	jne >n3
	cmp d[constant],0ffffh
	ja l1
	mov b[constsize],_w
	jmp createcode

n3:     cmp bl,cl3
	jne >n2
	cmp b[opdesc+2],r
	jne l1
	cmp b[opdata+2],1       ;_cl
	jne l1
	jmp createcode
n2:     mov ax,em_illegaloperand
	jmp error

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

#if !_mdltiny
_text   segment para
#else
	.code
#endif

; Jump tables
jmp_mov:dw mov_r,mov_e,mov_err
	dw mov_s,6 dup mov_sysr

jmp_mov_r:
	dw mov_r2r,mov_r2e,mov_r2c
	dw mov_s,6 dup mov_sysr

jmp_mov_e:
	dw mov_e2r,mov_err,mov_e2c
	dw mov_s,6 dup mov_err

jmp_op1:dw op1r,op1e,op1c
	dw op1s,5 dup op1err,op1af

jmp_op2:dw op2r,op2e,op2c
	dw op2errs,5 dup op2errsys,op2erraf

jmp_op2r:
	dw opr2r,opr2e,opr2c
	dw op2errs,5 dup op2errsys,op2erraf

jmp_op2e:
	dw ope2r,?,ope2c
	dw op2errs,5 dup op2errsys,op2erraf

	code ends
; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
;END
