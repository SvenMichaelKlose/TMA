;TMA macro assembler
;Constants and arithmetics.
; (c)1998 S.Klose

	.data
brackets:	dw ?	;Open brackets.
varflag:	dw ?	;true: A constant label was found (not only a constant)
	.code

; Arguments:
; DS:SI Address to parse.
; Returns:
; EAX   Value
;
; Note:
; ES:BX must be safed if variable was found.
e1:     mov ax,em_strnotended
	jmp error

; >>> Parse an expression
GETCONSTANT:
	call getconstant1       ;Get term.
	pushf
	cmp w[brackets],0
	je >l1
	mov w[brackets],0
	mov ax,em_bracketop     ;Error: Bracket not ended.
	jmp error
l1:	popf
	ret

getconstant1:
	push edi
	mov b[varflag],_code
	xor edi,edi
	cmp b[si],"-"
	je >m1
	cmp b[si],"("           ;Klammer ?
	jne >n1
	inc w[brackets]
	inc si
	call getconstant1       ;for this bracket.
	mov edi,eax
	jmp >l1
n1:     call getconst2          ;Get first constant
	jnc >x2
	mov edi,eax
	ONEOL >x3
	;
l1:     delspace
	lodsb
	cmp al,"+"	  ;Add
	jne >n1
	call getconst2
	jnc >x1
	add edi,eax
	jmp l1

m1:     lodsb
	cmp al,"("
	jne >n1
	inc w[brackets]
	call getconstant1
	mov edi,eax
	lodsb
n1:     cmp al,"-"        ;Subtract
	jne >n1
	call getconst2
	jnc >x1
	sub edi,eax
	jmp l1

x2:     jmp >x2

x1:	dec si
x3:     jmp >x1

n1:     cmp al,"*"        ;Multipliy
	jne >n1
	call getconst2
	jnc >x1
	mov ebx,edi
	xor edx,edx
	mul ebx
	mov edi,eax
	jmp l1

n1:     cmp al,"/"
	jne >n1
	call getconst2
	jnc >x1
	mov ebx,eax
	mov eax,edi
	xor edx,edx
	div ebx
	mov edi,eax
	jmp l1

n1:     cmp al,")"      ;Ending bracket?
	jne >n1
	dec w[brackets]
	mov ax,w[brackets]
	cmp ax,-1
	je >e2          ;Error: Ending brackets missing.
	jmp >x1

n1:     dec si
	push di
	mov di,map_arith2
	call getstring
	pop di
	jnc >n1
	mov bx,ax
	add bx,bx
	call w[bx+jmp_arith2]
	jmp l1

n1:     cmp w[si],"y" by "b"        ;x by x ?
	jne >n1
	add si,2                ;Skip "by"
	shl edi,8
	call getconst2
	jnc >x1
	add di,ax
	jmp l1

n1:
x1:     mov eax,edi
	stc
x2:     pop edi
	ret

e10:    jmp e1

e2:     mov w[brackets],0
	mov ax,em_tmbracket
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Parse single value only.
getconst2:
	push di
	delspace
	mov al,b[si]
	cmp al,"("
	jne >n1
	inc w[brackets]
	inc si
	call getconstant1
	pop di
	stc
	ret
n1:     cmp al,34       ;ASCII definition?
	je >g1
	cmp al,"'"
	jne >l1         ;No, continue...
g1:     mov ah,al
	inc si
	xor ebx,ebx
l2:     lodsb
	or al,al
	je e10
	cmp al,ah
	je >l3
	shl ebx,8
	mov bl,al
	jmp l2

l3:     mov eax,ebx
	pop di
	stc
	ret

n1:     xor eax,eax       ;Eine 0 eintragen...
	inc si
	pop di
	stc
	ret

l1:     cmp al,"?"
	je n1           ;Same as 0
	cmp al,"0"      ;Is it a digit?
	jb >l2
	cmp al,"9"
	jbe >l1         ;Yes...
l2:     cmp al,"$"      ;Code position?
	jne >l3
	inc si          ;Yes, skip dollar.
	mov eax,[codeptr]
	jmp >l2
l3:     mov di,st_constants    ;OFFSET direktive
	call getstring_fast
	jnc >n1
	delspace
	call getlabelln
	call getlabelmode
	push w[varflag]
	call getconst2
	pop w[varflag]
	jmp >l2
l1:     jmp near >l1
	;
n1:     cmp b[si],"!"           ;NOT ?
	jne >n1
	inc si
	mov bx,4                ;Token of NOT
	jmp >g1
n1:     mov di,map_arith1       ;arithmethic function ?
	call getstring
	jnc >n1
	mov bx,ax
	add bx,bx
g1:     call w[bx+jmp_arith1]
	pop di
	ret
	;
n1:     call getlabelln
	call getlabelmode
	cmp b[pass],1
	jne >l20
	cmp b[flag_local],1     ;Pass 1, predict local label...
	je >l40
;jmp >l4
	mov es,w[seg_global]
	call seeklabel          ;Search label.
	jc >l2                  ;Found...
	mov es,w[seg_data]      ;Search variable.
	call seeklabel
	jc >l3
l4:     mov eax,[codeptr]       ;1st pass: predict label.
;        cmp w[currmnemdesc],t_jmp
;        if e add ax,130         ;Predict word distance on any jump-
	add si,dx
l2:     pop di
	stc
	ret

s1:     push bx,edx
	mov bx,[es:label_seg+bx] ;Offset from group base.
	mov edx,[seg_pos+bx]
	mov bx,[seg_groupbase+bx]
	or bx,bx
	je >s2          ;keine Gruppe
	sub edx,[seg_pos+bx]
	add eax,edx
s2:     pop edx,bx,di
	stc
	ret
	;
l3:     test b[es:bx+label_flag],_struc ;Does label belong to a structure?
	jne l2                  ;Ja, Wert ist richtig.
	cmp b[actualmodel],_tiny ;Model tiny ?
	jne s1                  ;No, Segments addressed separately
	db 66h                  ;Yes, Data label + sizeof(code segment)
	add ax,w[segflags+seg_max]  ;Start of data segment
	jmp l2

l40:	cmp b[si],">"		;Jump over direction sign.
	jne l4
	inc si
	jmp l4

t1:     call t_seg2
	jmp l2

l20:    call seek_addr          ;public/local Label?
	jc l2
	mov es,w[seg_data]
	call seeklabel          ;Data label?
	jc l3
	mov di,tab_segments     ;Segment name?
	call getstring
	jc t1
	push si                 ;Label not found.
	call getregister
	jnc >l7
	pop si,di               ;It's a register, don't exit.
	clc
	ret
	;
l7:	mov ax,em_labnotfound
	jmp error
	
l1:     push bp,si
	mov bp,si       ;save start of string
	xor cx,cx
	mov ah,b[si]
g1:     lodsb           ;get string length
	cmp al,"0"
	jb >c1
	cmp al,"9"
	ja >c2
	inc cl
	xor ch,ch
	jmp g1
c2:     cmp al,"a"
	jb >c2
	cmp al,"f"
	ja >c2
	add cx,101h
	jmp g1
c2:     xor ch,ch
c1:     sub cl,ch
	mov al,cl
	mov cl,ch
	xor ch,ch
	sub si,cx
	dec si
	mov cl,al
	or cl,cl
	jz >n1          ;no digit found
l1:     mov al,b[si]
	cmp al,"x"
	je >x1
	cmp al,"h"      ;hexadecimal ?
	je >h1
	cmp al,"b"      ;binary ?
	je >h3
	cmp al,"o"      ;octal ?
	je >h2
	cmp al,"q"
	je >h2
	cmp al,"k"      ;Kilobyte
	jne >d1
	cmp ah,"0"      ;Hexadezimal ? (start with zero)
	je >h1
	call getdec	;Wert holen
	shl eax,10      ;*1024
	inc si
	pop dx,bp,di
	stc
	ret

d1:     call getdec

n1:     pop dx,bp,di
	stc
	ret

x1:     inc si
	jmp l1

h3:     jmp >h3
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Read hexadecimal value
h1:     xchg si,bp
	xor di,di
	xor eax,eax
	xor edx,edx
l1:     shl eax,4
	mov dl,[si]
	cmp dl,"a"
	jb >l2
	sub dl,"a"-10-"0"
l2:     sub dl,"0"
	add eax,edx
	inc si
	dec cl
	jnz l1
	cmp b[si],"x"
	if e inc si
	cmp b[si],"h"
	if e inc si
	pop dx,bp,di
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Read octal value
h2:     xchg si,bp
	xor di,di
	xor eax,eax
	xor edx,edx
l1:     shl eax,3        ;converts decimal string to EAX
	mov dl,[si]
	sub dl,"0"
	add eax,edx
	inc si
	dec cl
	jnz l1
	cmp b[si],"x"
	if e inc si
	inc si
	pop dx,bp,di
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Read binary value
h3:     xchg si,bp
	xor edx,edx
	xor eax,eax
	xor edx,edx
	cmp cl,32
	ja >e1                  ;Error: Constant is too large...
l1:     add eax,eax
	mov dl,b[si]
	inc si
	sub dl,"0"
	add eax,edx
	dec cl
	jnz l1
	cmp b[si],"x"
	if e inc si
	inc si                  ;Skip marker.
	pop dx,bp,di
	stc
	ret

e1:     mov ax,em_const2big
	jmp error


; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Read decimal value
getdec: xchg si,bp
	xor di,di
	xor eax,eax
	xor edx,edx
l1:     mov ebx,eax       ;converts decimal string to EAX
	add eax,eax
	shl ebx,3
	add eax,ebx
	mov dl,[si]
	sub dl,"0"
	add eax,edx
	inc si
	dec cl
	jnz l1
	ret

t_low:  call getconstant
	jnc >l1
	push ax
	xor eax,eax
	pop ax
	xor ah,ah
	stc
l1:     ret
	
t_high: call getconstant
	jnc >l1
	push ax
	xor eax,eax
	pop ax
	shr ax,8
	stc
l1:     ret

t_dnot: call getconstant
	jnc >l1
	not eax
	stc
l1:     ret

t_dbit: call getconstant
	jnc >l1
	mov cl,al
	xor eax,eax
	mov al,1
	shl eax,cl
	stc
l1:     ret

t_seg:  delspace
	mov di,tab_segments      ;Segmentname?
	call getstring
	jnc >v1
t_seg2: call getsegflags         ;Ja, relative Startposition holen
	mov eax,seg_pos[bx]
	mov di,seg_groupbase[bx]
	or di,di
	if nz mov eax,seg_pos[di]
	shr eax,4
	mov b[flg_set_reloc_addr],1
	stc
v1:     ret
	;
t_and:  call getconstant
	jnc >l1
	and edi,eax
	stc
l1:     ret

t_or:   call getconstant
	jnc >l1
	or edi,eax
	stc
l1:     ret

t_xor:  call getconstant
	jnc >l1
	xor edi,eax
	stc
l1:     ret

t_shl:  call getconstant
	jnc >l1
	mov cl,al
	shl edi,cl
	stc
l1:     ret

t_shr:  call getconstant
	jnc >l1
	mov cl,al
	shr edi,cl
	stc
l1:     ret

#if !_mdltiny
_text   segment para
#else
	.code
#endif
; Arithmetic keywords
map_arith1:
	db"low",0
	db"high",0
	db"not",0
	db"bit",0
	db"seg",_end

map_arith2:
	db"and",0
	db"or",0
	db"xor",0
	db"shl",0
	db"shr",_end

jmp_arith1:
	dw t_low
	dw t_high
	dw t_dnot
	dw t_dbit
	dw t_seg

jmp_arith2:
	dw t_and
	dw t_or
	dw t_xor
	dw t_shl
	dw t_shr

