;TMA macro assembler
;Functions handling directives.
; (c)1996-1998 S.Klose

	include"asm\notthere.inc" ;Commands that aren't implemented.

	.data
orgoffset       dd ?    ;will relocate all label addresses

	.code
; DB <Werteliste>
j_db:   call update_label
	delspace
	ONEOL >e1 ;jne >l8
jmp >l8
	xor al,al       ;DB without parameters
	call writecode
	stc
	ret
l8:     delspace
	mov al,b[si]
	cmp al,","
	jne >l1
	inc si
	jmp l8
l1:     cmp al,34
	je >l3
	cmp al,"'"
	jne >l4
l3:     inc si
	mov ah,al
l2:     lodsb
	cmp al,ah
	je >l4
	or al,al
	je >e1
	call writecode
	jmp l2
l1:     stc
	ret

e1:     mov ax,em_unexpectedeol
	jmp error

l4:     delspace
	mov al,b[si]
	or al,al
	je l1
	cmp al,","
	if e inc si
l5:     call getconstant
	delspace                ;Check on 'dup'.

	cmp b[si],"d"
	jne >l6
	cmp w[si+1],"p" by "u"
	jne >l6
	add si,3
	push ax
	delspace
	call getconstant
	pop cx
l7:     call writecode
	loop l7
	jmp l8
l6:     call writecode
	jmp l8

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_dw:   call update_label
l8:     delspace
l4:     mov al,b[si]
	or al,al
	je l1
	cmp al,","
	if e inc si

l5:     call getconstant
	delspace                ;Check on 'dup'
	cmp b[si],"d"
	jne >l6
	cmp w[si+1],"p" by "u"
	jne >l6
	add si,3
	push ax
	delspace
	call getconstant
	pop cx
	mov bx,ax
l7:     mov al,bl
	call writecode
	mov al,bh
	call writecode
	loop l7
	jmp l8

l6:     call writecode
	mov al,ah
	call writecode
	jmp l8

l1:     stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_dd:   call update_label
l8:     delspace
l4:     mov al,b[si]
	or al,al 
	je l1
	cmp al,","
	if e inc si

l5:	call getconstant
	delspace                ;Check on 'dup'
	cmp b[si],"d"
	jne >l6
	cmp w[si+1],"p" by "u"
	jne >l6
	add si,3
	push ax
	delspace
	call getconstant
	pop cx
l7:     push eax
	call store_dd
	pop eax
	loop l7
	jmp l8

l6:     call store_dd
	jmp l8

store_dd:
	call writecode
	shr eax,8
	call writecode
	shr eax,8
	call writecode
	shr eax,8
	call writecode
	ret

j_dq:   call update_label
l8:     delspace
l4:     mov al,b[si]
	or al,al
	je l1
	cmp al,","
	if e inc si

l5:     call getconstant
	delspace                ;Check on 'dup'
	cmp b[si],"d"
	jne >l6
	cmp w[si+1],"p" by "u"
	jne >l6
	add si,3
	push ax
	delspace
	call getconstant
	pop cx
l7:     push eax
	call store_dq
	pop eax
	loop l7
	jmp l8

l6:     call store_dq
	jmp l8

store_dq:
	call writecode
	db 66h
	shr ax,8
	call writecode
	db 66h
	shr ax,8
	call writecode
	db 66h
	shr ax,8
	call writecode
	xor al,al
	call writecode
	call writecode
	call writecode
	call writecode
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; .DATA
j_pdata:call update_label
	mov bx,segflags+seg_size
	jmp set_new_segment

; .CODE
j_pcode: call update_label
	mov bx,segflags
	jmp set_new_segment

;.STACK <size in bytes>
J_PSTACK:
j_stack:call getconstant
	jnc >s1
	mov stack_size,ax
	stc
	ret
s1:     mov w stack_size,1024
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_org:  delspace
	call getconstant
	mov [codeptr],eax
	delspace
	cmp b[si],","
	jne >n1
	inc si
	call getconstant
	mov [orgoffset],eax
n1:	stc
	ret

e1:	mov ax,em_unexpectedeol
	jmp error

r1:	ret

j_enum: xor cx,cx
m1:	delspace
	mov al,[si]
	cmp al,"\"
	jne >b1
	push cx
	call getline
	pop cx
	jmp m1
b1:	cmp al,","
	jne >b2
	inc si
	inc cx
	jmp m1
b2:	call getlabelln
	or dx,dx
	je >n1
	cmp b[pass],1
	je >l10
        add si,dx
	mov es,w[seg_global]
	call update_defptr
	jmp >l2
l10:	mov es,w[seg_global]
	push cx
	call seeklabel
	jc >n2
	call definelabel
	pop cx
	mov w[es:bx+label_adr],cx
l2:	delspace
	lodsb
	cmp al,"\"
	jne >l1
	push cx
	call getline
	pop cx
	jnc r1
	jmp l2
l1:	or al,al
	je >e1
	cmp al,","
	jne >n1
	inc cx
	jmp m1
e1:	stc
	ret
n2:	pop cx
	cmp b[pass],1
	jne l2
	mov ax,em_dblpublic
	jmp error
n1:	mov ax,em_unexpectedeol
	jmp error


;Set label's PUBLIC flag.
j_public:
	delspace
	call getlabelln
	mov es,w[seg_global]
	call seeklabel
	jc >l1
	mov es,w[seg_data]
	call seeklabel
	jc >l1
	cmp b[pass],1           ;Label not found.
	je >o1                  ;Ok in the 1st pass.
	mov ax,em_labnotfound
	jmp error
l1:     or b[es:bx+label_flag],_public
o1:	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; SEGMENT directive

	.data
segments:	dw ?		;current # of segments
code_segments	dw ?
segflags:	db 256 dup ?	;Adresses and flags of segments
tab_segments:	db 256 dup ?	;Segment names
lastsegflag:	dw ?

classes:	dw ?		;# of classes
tab_classes:	db 256 dup ?	;Class names

stack_size:	dw ?		;Size of STACK segment
arranged_segments: dw 64 dup ?	;Segment order
actualmodel:	db ?		;Currently used memory model

	.code
segdesc struc
	seg_xmshandle:	 dw ?	;Buffer handle, will be allocated if 0.
	seg_attr:	db ?	;Segment type.
	seg_ptr:	dd ?	;Last position in segment.
	seg_max:	dd ?	;Last segment size.
	seg_pos:	dd ?	;Relative position of segment.
	seg_index:	db ?
	seg_class:	db ?	;Index of class name.
	seg_groupbase:	dw ?	;Base segment of group.
	seg_overlay:	db ?
	seg_at: 	dd ?	;Absolute position if defined.
			dw ?
	seg_info:	db ?
	seg_size:
	ends

; *** We miss the check for matching definitions, if they are double. FIXME!
j_segment:
	push si
	mov si,[lastlabel]      ;Check, if segment is already defined this way.
	mov b[flag_lastlabel],0
	mov di,tab_segments
	call getstring
	jc >l1                  ;Yes, number in AX.
	;
	call label2list
	mov al,b[segments]
	inc b[segments]
	;
l1:     call getsegflags
	pop si
	;
m1:     delspace
	ONEOL set_new_segment	;Break, end of line...
	mov di,map_segment      ;Get instruction.
	cmp b[si],"'"           ;Class name?
	je >s2                  ;Yes...
	call getstring
	jc >s1                  ;Found...
	mov ax,em_segment       ;Display help on 'SEGMENT' parameters.
	jmp error
	;
s1:     cmp ax,_at              ;Segment on absolute address?
	jne >n1                 ;No, continue.,,
	or b[bx+seg_attr],sa_at ;Set flags.
	call getconstant        ;Get absolute address...
	shl eax,4               ;...and save it.
	mov [bx+seg_at],eax
	jmp m1
	;
	;Convert instruction token to segment flag.
	;May be alignment or combination type.
n1:     push bx
	mov bx,tab_segattr
	xlat
	pop bx
	or b[bx+seg_attr],al
	jmp m1
	;
	;Handling of class names.
s2:     inc si
	mov di,tab_classes
	call getstring
	jc >h1
	call label2list
	mov al,b[classes]
	inc b[classes]
h1:     mov [bx+seg_class],al
	lodsb
	cmp al,"'"
	jne >e2         ;Error: Class name not ended.
	jmp m1
	;

getsegflags:
	mov bx,seg_size  ;Get pointer on segment info.
	xor dx,dx
	xor ah,ah
	mul bx
	mov bx,ax
	add bx,segflags
	ret

; Handles segment caching

; BX=Pointer on segment info.
set_new_segment:
o1:	mov bp,w[lastsegflag]	;Position of last segment info.
	mov eax,[codeptr]	;Store new position in it.
	mov [bp+seg_ptr],eax
	cmp seg_max[bp],eax	;Store new end position of segment if possible.
	if b mov seg_max[bp],eax
	;
	mov eax,[bx+seg_ptr]	;Address of code pointer within new segment.
	;
	push bx,eax
	mov al,b[bx+seg_attr]	;Segment mode.
	xor ebx,ebx
	mov bl,al
	and bl,mask_align
	if z mov bl,sa_para	;PARA ist default alignment.
	dec bl
	add bx,bx
	mov bx,align_values[bx]
	and al,_data
	mov b[generation_mode],al
	pop eax
	call align_eax_addr
	pop bx
	mov [codeptr],eax
	mov w[lastsegflag],bx

	;Get new XMS page.
	push ds,es
	pushad
	;
	mov dx,seg_xmshandle[bp]    ;Get old segment handle.
	test b[bp+seg_attr],_data   ;Is it a data segment?
	je >t2		       ;Yes, ignore it.
	push dx
	jmp >g2
t2:
	or dx,dx
        jnz >n3
	mov dx,64		;Handle is 0, allocate memory for it.
	call rmb_allocate
	mov seg_xmshandle[bp],dx;Save the new handle.
	;
n3:	mov [rmb_desthandle],dx ;Copy generated code to XMS-
	push dx
	mov ax,[seg_code]
	mov [rmb_source+2],ax
	xor eax,eax
	mov [rmb_source],ax
	mov [rmb_srchandle],ax
	mov [rmb_dest],eax
	mov eax,seg_ptr[bp]
	and al,-4
	add eax,4
	mov d[rmb_copylen],eax
	push bx
	call xmscopy
	pop bx
	;
g2:	mov ax,seg_xmshandle[bx] ;Get info about new segment.
	pop dx
	test b[ds:bx+seg_attr],_data
	jne >g2
;	 cmp ax,dx
;	 je >g2
	or ax,ax
	jz >g1			;No cached data for it...
	mov [rmb_srchandle],ax
	xor eax,eax
	mov [rmb_desthandle],ax
	mov [rmb_source],eax
	mov [rmb_dest],ax
	mov ax,[seg_code]
	mov [rmb_dest+2],ax
	mov eax,seg_ptr[bx]
	and al,-4
	add eax,4
	mov d[rmb_copylen],eax
	call xmscopy
	jmp >g2
	;
g1:	mov es,[seg_code]	;Clear new segment contents.
	mov cx,8000h
	xor ax,ax
	xor di,di
	rep stosw
g2:	popad
	pop es,ds
	stc
	ret

e1:	mov ax,em_noclass
	jmp error
e2:	mov ax,em_strnotended
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.data
groups: 	dw ?		;# of groups
groupflags:	db 256 dup ?	;Group combinations
tab_groups:	db 256 dup ?	;Group names

	.code
j_group:
	mov b[flag_lastlabel],0
	cmp b[pass],1
	je >n1
;	 push si
;	 mov si,[lastlabel]	 ;Check if segment name is already used.
;	 mov di,tab_groups
;	 call getstring
;	 jnc >n2
;	 cmp b[pass],1
;	 jne >n1
;	 pop si
;	 mov ax,em_dblgroup
;	 jmp error
;n2:	 call label2list
;     pop si
	delspace
	mov di,tab_segments	;Base segment.
	call getstring
	jnc >e1
	call getsegflags
	mov cx,bx
l1:	delspace		;Linked segment.
	lodsb
	or al,al
	jz >n1
	cmp al,","
	jne >e1
	mov di,tab_segments
	call getstring
	jnc >n1
	call getsegflags
	mov [seg_groupbase+bx],cx ;Save base segment #
	jmp l1
	
n1:	stc
	ret
e1:	mov ax,em_segmentexpected
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

	.data
tab_assumes:	db 7 dup ?	;ASSUMEs

	.code
j_assume:
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.data
strucofs:	dd ?		;Current STRUC OFFSET
ptr_strucsize	dw ?
ptr_strucsizes	dw ?
ptr_eaoverrides dw ?
actual_eaadr:	dw ?		;Currently used STRUC override parameter

	.code
jump_over_struc:		;Skip structure in passes >1
	mov es,[seg_struc]
        mov bx,[es:label_update]
	call update_defptr

	mov cx,[bx]
	mov es,[seg_global]
l1:	call update_defptr
	loop l1

	mov cx,[bx+2]
	mov es,[seg_data]
l1:	call update_defptr
	loop l1

	mov b[flag_lastlabel],0
l3:	call getline
l2:	delspace
	ONEOL l3
	mov di,st_maindirs
	call getstring_fast
	jc >l1
	call getlabelln
	add si,dx
	cmp b[si],":"
	jne l2
	inc si
	jmp l2
l1:	cmp ax,_ends
	jne l3
	stc
	ret

	;Place structure names in dedicate memory area.
	;Store overrides.
	;Define labels as usual but with STRUC flags.
	;Add override #.
	;Assemble 'Dx' and sub structures until ENDS, omit error on file end.
	;Store structure size after its name.
	;Done.
j_struc:cmp b[pass],1
	jne jump_over_struc
	mov gs,[seg_strucsizes]
	mov al,b[flag_lastlabel]
	or al,al
	je >n1			;kein Strukturname vorhanden
	mov w[strucofs],0
	mov es,w[seg_struc]
	push si
	mov si,w[lastlabel]
	call definelabel	;Store label.
	mov ax,w[ptr_strucsizes] ;Table of structure sizes.
	mov w[es:bx+label_adr],ax
	pop si

n1:	mov w[actual_eaadr],0	;0=no override used.
	delspace
	cmp b[si],"["		;override ?
	je >x1
	mov di,map_constants	;OFFSET ?
	call getstring
	jnc >n1
	delspace		;Yes, get it.
	call getconstant
	jnc >e1
	mov w[strucofs],ax
	delspace
	cmp b[si],"["
	je >x1
	jmp >n1

e1:	mov ax,em_constmissing
	jmp error

x1:	inc si
	mov ax,w[ptr_eaoverrides]
	mov w[es:bx+label_adr+2],ax
	mov w[actual_eaadr],ax
	call ea_of_unknown_size ;Get predefined effective address as argument
	push es
	mov es,[seg_eaoverrides]
	mov di,w[ptr_eaoverrides]
	mov si,eadesc
	mov cx,eadescend-eadesc
	rep movsb
	pop es
	mov w[ptr_eaoverrides],di

n1:	push w[generation_mode]
	push w[codeptr]
	mov w[generation_mode],_data ;Indicate data mode.
				;Pointer will e restored later on.
	mov b[ac_label_flag],_struc ;Lock addition of code segment size while
				;using TINY memory model.
	mov ax,w[strucofs]
	mov w[codeptr],ax	;Structure start
	push w[used_labels_c],w[used_labels_d],w[ptr_strucsizes]
	add w[ptr_strucsizes],4
l1:	call getline		;Get line from source.
	jnc >n2 		;Last line must contain an ENDS directive.
n0:	delspace
	ONEOL l1
	mov di,st_maindirs	;Directive?
	call getstring_fast
	jc >n1			;Yes...
	push si,es
	call getlabelln
	mov es,w[seg_struc]
	call seeklabel		;Variable or structure ?
	pop es,si
	jc >d1			;Yes, assemble line.
	call keep_label 	;Define buffered label.
	call update_struc
	mov ax,w[actual_eaadr]	;Store override if there is one.
	mov w[es:bx+label_seg],ax
	delspace
	mov di,st_maindirs
	call getstring_fast
	jc >n1
	push si,es
	mov es,w[seg_struc]
	call getlabelln
	call seeklabel
	pop es,si
	jc >d1
	jmp l1

n2:	jmp >n2
n1:	cmp ax,_ends		;ENDS ?
	je >n3			;Yes, end of structure definition.
	cmp ax,_string		;String ?
	jne >n5
	mov bx,w[ptr_strucsizes]
	mov w[gs:bx],8000h	   ;Entry size.
	add bx,2
	mov w[ptr_strucsizes],bx
	jmp l1			;Next entry...

d1:	mov ax,w[codeptr]
	push ax
	call parse1
	jmp >d2

n5:	mov bx,ax
	add bx,bx
	mov ax,w[codeptr]
	push ax
	call w[jmp_maindirs+bx]   ;Call directive's function...

d2:	pop cx
	mov ax,w[codeptr]
	sub ax,cx
	neg bp
	sub bp,w[used_labels_d]
	mov bx,w[ptr_strucsizes]
	mov w[gs:bx],ax 	  ;Entry size.
	add bx,2
	mov w[ptr_strucsizes],bx
	jmp l1

n2:	mov di,st_maindirs	;Directiv ?
	call getstring_fast
	jnc >e1 		;No, ENDS missing.
	cmp ax,_ends
	jne >e1
n3:	xor ax,ax
	mov bx,w[ptr_strucsizes]
	mov w[gs:bx],ax 	   ;End of entry.
	add bx,2
	mov w[ptr_strucsizes],bx
        mov b[ac_label_flag],0

	pop bx,ax
	mov cx,[used_labels_d]
	sub cx,ax
        mov [gs:bx+2],cx

	pop ax
	mov cx,[used_labels_c]
	sub cx,ax
	mov [gs:bx],cx
	pop w[codeptr]
	pop w[generation_mode]
	stc
n1:	ret

e1:	mov ax,em_endsmissing
	jmp error

j_ends: mov b[flag_lastlabel],0
	stc
	ret

update_struc:
	cmp b[flag_lastlabel],0 ;Label at start of line?
	je n1			;No, return.
	push si,bp		;Save current line position.
	mov si,w[lastlabel]	;Get position of buffered label.
	mov b[flag_lastlabel],0
	mov dx,w[lastlabelln]

; >>> Datenlabel eintragen <<<
	mov es,w[seg_global]	;Check on double definition but except local
	call seeklabel		; labels.
	jc >e2
	mov es,w[seg_data]
	call seeklabel
	jc >e1
n2:	mov b[flag_local],0	;***V
	call definelabel	;No, define label.
	mov eax,[codeptr]
	mov [es:bx+label_adr],eax
	cmp b[si],":"		;Recognize label type by occurence of following
	jne >l1 		;':'
	mov b[es:bx+label_flag],_code or _struc
l4:	pop bp,si
	ret
l1:	mov b[es:bx+label_flag],_data or _struc ;set STRUC flag
	jmp l4

e1:	cmp b[pass],1
	je >e2
	mov eax,[codeptr]	;Redefine label in Pass>1.
	mov [es:bx+label_adr],eax
	jmp l4

e2:	mov ax,em_alreadypublic ;Error: Label already defined as PUBLIC.
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

e1:	mov ax,em_modelexpected
	jmp error

	; MODEL direktive
	;
	; MODEL <memory modell>[,language]
j_model:delspace
	mov di,map_models	;TINY,SMALL,COMPACT,LARGE,HUGE
	call getstring
	jnc e1
	;
	cmp b[pass],1
	jne >o1
	cmp al,_small
	ja >n1
	jb >n2
	cmp b[output_mode],_com
	je >e1
	; Initialise memory model (TINY)
n2:     call init_std_segments
n1:
o1:     stc
	ret
e1:     mov ax,em_nocommdl
	jmp error

init_std_segments:
	mov w[stack_size],8k
	mov b[actualmodel],al
	mov al,_end
	mov [tab_groups],al
	mov [tab_classes],al
	mov w[lastsegflag],offset segflags
	mov di,tab_segments     ;intialise code and data segment
	mov si,segments_std
	call copy_stringlist
	mov di,tab_classes
	mov si,classes_std
	call copy_stringlist
	mov bx,segflags
	mov b[bx+seg_attr],sa_public+sa_para
	mov b[bx+seg_size+seg_attr],sa_public+sa_para+_data
	mov b[bx+seg_class+seg_size],1
	mov b[bx+(seg_size*2)+seg_attr],sa_public+sa_stack+sa_para
	mov w[segments],3
	mov w[classes],1
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_if:   call update_label
	delspace
	mov di,conditionals     ;Get condition.
	call getstring
	jnc >l2                 ;None...
	xor al,1                ;Negate condition.
	mov b[condition],al
	mov b[actualmnemonic],-1 ;dummy
	mov w[currmnemdesc],t_j
	mov b[oplength],1
	mov b[opdesc],co
	mov b[constflag],1
	mov bx,w[cnt_internal]
	mov es,w[seg_internal]
	shl bx,3
	push es,bx,si
	mov eax,[es:bx]
	cmp b[pass],1           ;Don't do anything in pass 1.
	jne >n1
	mov eax,[codeptr]
n1:	mov [constant],eax
	call mapoperands
	pop si
	call parse1
	pop bx,es		;Define internal variable.
	mov eax,[codeptr]
	cmp [es:bx],eax 	;New pass if it changed.
	je >n1
	mov [es:bx],eax
	mov b[newpass],1
n1:	inc w[cnt_internal]
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.data
ptr_nextmacro	dw ?		;Pointer to free space in macro buffer
flag_macro	dw ?		;true: A macro source is currently assembled
macroparmnames: db 1k dup ?	;Macro parameter names

	.code
jump_over_macro:		;Skip macro definition in pass>1
l3:	call getline
	mov es,fs
l2:	delspace
	mov al,b[si]
	or al,al
	je l3
	cmp al,"#"
	jne >l4
	inc si
	mov di,map_endm2
	call getstring
	jc >l1
	dec si
	jmp >l5
l4:	mov di,map_endm1
	call getstring
	jc >l1
l5:	call getlabelln
	add si,dx
	or dx,dx
	jne >b1
	inc si
b1:	cmp b[si],":"
	jne l2
	inc si
	jmp l2
l1:	stc
	ret

e2:	mov ax,em_dblmacro
	jmp error
e10:	mov ax,em_parm2big
	jmp error

db"MACRO!"
j_macro:mov ds,fs
	cmp b[pass],1
	jne jump_over_macro	;Pass 2+: Label is already defined.
	cmp b[flag_lastlabel],0
	je >e1			;Error: Macro needs a name.
	push si
	mov es,w[seg_global]
	mov si,w[lastlabel]
	call seek_addr		;Check if macro is already defined.
	jc e2			;Error: It is...
	mov es,w[seg_macro]	;Define macro.
	call definelabel
	mov di,w[ptr_nextmacro] ;Store position of macro data.
	mov w[es:bx+label_adr],di
	pop si
	push di 		;Generate parameter list to compress macro
	mov es,fs		; source.
	mov di,macroparmnames
	call get_parmlist
	pop di
	
; Parameters are converted into tokens 128+parameter#(0..127) here.
	mov es,[seg_macrodata]
l0:	push es,di
	call getline		;Read new input line.
	pop di,es
	jnc >e3 		;EOF: Error: ENDM missing.
	delspace
l2:	cmp b[si]," "		;Copy single space only.
	jne >l5
	movsb
	delspace
l5:	cmp b[si],">"
	if e movsb		;Copy it before we crash
	cmp b[si],"#"		 ;Is it #EM?
	je >k2
	call getlabelln 	;Length of following string.
	or dx,dx
	jz >l5			;No string there...
	push di,es
	mov es,fs
	mov di,map_endm1	;ENDM ?
	call getstring
	pop es,di
	jc >l1			;Yes, that's all.
	push di,es
	mov es,fs
	mov di,macroparmnames  ;Parameter?
	cmp b[di],0
	je >o1
	call getstring
	jnc >o1
	pop es,di
	jmp store_macrotoken
o1:	pop es,di
	;Nichts gefunden
l4:	or dx,dx
	je l2
l110:	lodsb	;Copy string to macro.
	stosb
	dec dx
	jnz l4
	jmp l2

k2:	inc si
g1:	push di,es
	mov es,fs
	mov di,map_endm2
	call getstring
	pop es,di
	jc >l1			;Yes, that all...
	mov al,b[si]
	cmp al,"0"		;Parameter token?
	jb >n2
	cmp al,"9"
	ja >n2
	call getconst2
	jnc >n2
	dec ax
store_macrotoken:
	cmp ax,127
	ja e10			;Token too big.
	or al,128		;Store token.
	stosb
	jmp l2
n2:	dec si			;No, return previous character.
l5:	lodsb			;Copy single character
	stosb
	or al,al
	je l0
	cmp al,'"'
	je >l3
	cmp al,"'"
	jne l2
	;
l3:	mov ah,al
l6:	lodsb			;Don't search ENDM withing ASCII strings.
	stosb
	cmp al,ah
	je l2
	or al,al
	jne l6
	jmp l0
	;
l1:	mov ax,_end by 0	   ;Mark end of macro definition.
	stosw
	mov w[ptr_nextmacro],di
	stc
	ret

e1:	mov ax,em_nomacroname
	jmp error
e3:	mov ax,em_endmmissing
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.data
		dw ?
stc_includes:	dd 64 dup ?	;Include file stack
includes:	dw ?		;Current # of include files
includecnt:	dw ?
flag_includes:	db ?		;true: include files were used

	.code
j_include:
	call update_label		;Define buffered label if there is one.
	call get_include
	mov bx,w[stc_includes-2]	;N# of current input file.
	mov eax,[rmb_fileaddr]		 ;Address of current page.
	xor edx,edx
	mov dx,w[inputsize]
	sub eax,edx
	mov [bx+i_xmspos],eax
	mov eax,[rmb_filerest]		 ;Size of rest.
	add eax,edx
	mov [bx+i_len],eax
	mov ax,w[inputptr]              ;Current position in page.
	mov w[bx+i_pgpos],ax
	xor ax,ax
	xchg ax,[lines_local]           ;Set current line counter
	inc ax
	mov i_lines_local[bx],ax
	mov al,[current_filenr]
	mov [bx+i_current_filenr],al
	add bx,16
	mov w[stc_includes-2],bx
	inc w[includes]
	inc w[includecnt]
	mov dx,buffer16
	call open_file
	mov es,w[seg_input]
	push di
	xor di,di
	mov cx,maxbuf
	call readsource
	pop di
	mov w[inputptr],0
	stc
	ret

;e1:     mov ax,em_namemissing
;        jmp error

; Copy filename to BUFFER16 and check it.
get_include:
	mov di,buffer16
	delspace
	mov es,fs
	lodsb
	cmp al,"'"
	je >l1
	cmp al,'"'
	je >l1
	;
	dec si
h1:     lodsb           ;Copy file name without qoutes
	cmp al," "
	jle >l3
	stosb
	jmp h1
	;
l1:     mov ah,al
l2:     lodsb
	cmp al,ah
	je >l3
	stosb
	jmp l2
l3:     xor al,al
	stosb
	;
	cmp b[pass],1
	jne >l1
	cmp b[flag_includes],0
	jne >l2
	mov si,txt_external     ;Display "Other sources:" 
	call asciiz
	mov b[flag_includes],1
l2:     mov si,txt_include
	call asciiz
	;
	mov cx,[includes]
	shl cx,2
	mov al," "
	jcxz >d2
d1:     call cout
	loop d1
d2:
	;
	mov si,buffer16
	call asciiz
	call cret
	call showpass
	ret
	;
l1:     call showpass3
	mov b stdoutmode,1
	mov al," "
	call cout
	mov al,"-"
	call cout
	mov al," "
	call cout
	mov si,buffer16
	call asciiz
	mov cx,20
	mov al," "
l1:	call cout
	loop l1
	mov al,13
	call cout
	mov b stdoutmode,0
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_086:	mov b[processormode],_086
	stc
	ret
j_186:	mov b[processormode],_186
	stc
	ret
j_286:	mov b[processormode],_286
	stc
	ret
j_386:	mov b[processormode],_386
	stc
	ret
j_486:	mov b[processormode],_486
j_pentium:
	mov b[processormode],_586
	stc
	ret
j_ppro: mov b[processormode],_ppro
	stc
	ret
j_mmx:	mov b[processormode],_mmx
	stc
	ret
j_nec:	mov b[processormode],_nec
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

j_page: mov bx,256
	call align_addr
	stc
	ret

j_even: mov bx,2
	call align_addr
	stc
	ret

j_align:delspace
	call getconstant
	jnc >l1
	mov bx,ax
	jmp >l2
l1:	mov bx,4
l2:	call align_addr
	stc
	ret

j_para: mov bx,16
	call align_addr
	stc
	ret

align_addr:
	mov eax,[codeptr]
	call >l2
	mov [codeptr],eax
	stc
	ret
	;
; Align address in EAX
; EAX = Adresse
; BX  = Aignment
align_eax_addr:
	push bx
	xor ebx,ebx
	pop bx
l2:     xor edx,edx
	div ebx
	or edx,edx
	jz >l3
	call >l3
	add eax,ebx
	ret
l3:	xor edx,edx
	mul ebx
	ret

e1:	mov ax,em_constmissing
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.data
ptr_gifstack	dw ?		;Stack for multiple #IFs
stc_gif:	db 256 dup ?

	.code
j_gif:	call update_label
	delspace
	mov al,b[si]
	or al,al
	je e1		;Error: Condition missing.
	xor cl,cl
	cmp al,"!"
	jne >l1
	inc si
	dec cl		;Negate condition.
l1:	push cx
	call getlabelln
	call getlabelmode
	call seek_addr
	pop cx
	mov al,0
	jnc >l1
	dec al
l1:     xor al,cl               ;Negate conition if needed.
	mov bx,w[ptr_gifstack]
	mov b[stc_gif+bx],al
	inc w[ptr_gifstack]     ;Indicate that one more ENDIF is missing.
	or al,al
	jnz >l3         ;Condition true, continue as usual.
l4:     call getline
l2:     delspace
	mov al,b[si]
	or al,al
	je l4
	cmp al,"#"
	jne >l5
	inc si
	mov di,map_gif
	call getstring
	jc >l1
	delspace
l5:     call getlabelln
	or dx,dx
	if e inc dx
	add si,dx
	cmp b[si],":"
	jne l2
	inc si
	jmp l2
l3:     stc
	ret
l1:     cmp ax,gif_end
	je j_gend
	cmp ax,gif_elseif
	je j_gelseif
j_gelse:call update_label
	mov bx,w[ptr_gifstack]
	add bx,stc_gif-1
	mov al,b[bx]
	xor b[bx],-1
	or al,al
	jnz l4
	stc
	ret

j_gelseif:
	call update_label
	mov bx,w[ptr_gifstack]
	or bx,bx
	je >e1
	call j_gend
	add bx,stc_gif-1
	mov al,b[bx]
	xor b[bx],-1
	or al,al
	jnz l4
	jmp j_if

e1:     mov ax,em_unexpectedelseif
	jmp error
e1:     mov ax,em_unexpectedendif
	jmp error

j_gend: call update_label
	dec w[ptr_gifstack]
	jc e1
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

