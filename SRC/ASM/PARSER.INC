;TMA macro assembler
;x86 syntax parser
; (c)1996-1998 Sven Klose

	.data
currmnemdesc:	dw ?		;Descriptor of last mnemonic.
actualmnemonic: db ?		;Token of last mnemonic.
operandsize:	db ?		;Operand size (b,w,d,bw,wb,dw)

register:	db ?		;last register code (no seg or sys)
opdesc: 	db ?,?,?	;Operand types (r,e,d,cc,sr,tr,dr,cr)
opdata: 	db ?,?,?	;Operand info (rg,co,ea)
constsize:	db ?		;Size of last constant.
constant:	dd ?		;Value of last constant.
constant_b:	dd ?		;2nd constant for ENTER.
pflag:		db ?		;Value of P-digit (class number).
ptr_macroparms	dw ?
macroparms:	dw 1k dup ?
stc_macros:	dw 64 dup ?	;Macrostack

struc_macrostack struc
	macro_parent:	dw ?
	macro_parms:	dw ?
	ends


; ---------------------------------------------------------------------------
lineinitstart:
flag_contline:	db ?	;true: The rest of the input line will be processed as
			;a new one.
lineinitstart2:
flag_lastlabel: db ?	;true: A label was found at line start
constflag:	db ?	;true: A constant was found.
randflag:	db ?	;true: rand: prefix 66h needed
addrflag:	db ?	;true: addr: prefix 67h  "
scndflag:	db ?
wbit:		db ?	;word bit in opcode 
dbit:		db ?	;direction bit in opcode
oplength:	db ?	;Number of operands for this instruction.
condition:	db ?	;Type of last conditional jump.

; Effective adress generation:
ea_r:		db ?	;r nibble in xrm byte.

eadesc: 	db ?,?	;Contains information about an parsed effective adress.
eascaling:	db ?	;Scaling in effective adress.
eaconstflag:	db ?	;true: Constant found in effective adress.
eaconstsize:	db ?	;      Size of the constant.
ealength:	db ?	;Number of registers in effective adress.

ea_xm:			;xrm byte.
ea_m:		db ?
ea_x:		db ?
lineinitend:	db ?

; ---------------------------------------------------------------------------

ea_sirflag:	db ?	;Scaling in 32bit effective adress
ea_scaling:	db ?
ea_r2:		db ?	;Base register
ea_i:		db ?	;Index register
ea_s:		db ?	;Segment register
eaconst:	dd ?	;Value of constant in effective adress.
eaflag: 	db ?	;true: An effective adress was found.
override:	db ?	;segment override prefix needed or 0.
eadescend:

segregister:	db ?	;Segment override prefix needed or 0.
sysrnum:	db ?	;control/debug/taskreg index
sysrtype:	db ?	;system register type (cr,cr4,dr,tr)
sysrcode:	db ?	;encoded register type
scndconst:	db ?
afseg:		dw ?	;Segment of absolute far adress

pass:		db ?	;Number of this pass.
newpass:	db ?	;true: Assemble again if this pass is done.
lines:		dw ?	;Number of currently assembled lines
lines_local:	dw ?	;	The same for the current file
line:		db 256 dup ?	;Line buffer
lineptr:	dw ?	;Points to parser's line position if a prefix was found.

lastlabel:	dw ?		;Adress of last label in line buffer
lastlabelln:	dw ?		;Length of last label

	.code

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
; Initialise a new pass
;
; No arguments.
; Returns nothing.
initpass:
	call free_segments
	push bx
	mov ax,offset label_data
	mov bx,offset label_update
	mov es,[seg_global]
	mov w[es:bx],ax
	mov es,[seg_local]
	mov w[es:bx],ax
	mov es,[seg_data]
	mov w[es:bx],ax
	mov es,[seg_struc]
	mov w[es:bx],ax
	pop bx
	xor eax,eax		;Reset pointer to start of source
	mov b[generation_mode],al ;(Code output)
	mov w[cnt_internal],ax
	mov w[inputptr],ax
	mov w[lines],ax
	mov w[lines_local],ax
	mov w[used_labels],ax
	mov w[used_labels_c],ax
	mov w[used_labels_l],ax
	mov w[used_labels_d],ax
	mov w[includecnt],ax
	mov w[reloc_items],ax
	mov w[ptr_strucsizes],ax
	mov w[ptr_eaoverrides],ax
	mov b[newpass],al	;Clear flag that forces another pass
	mov d[codeptr],100h	;Reset destination pointer for code segment
	ret

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
; >>> Extracts line from source buffer <<<
e1:	mov ax,em_noparm
	jmp error

;Insert macro parameter
p1:	mov bx,w[fs:stc_macros]
	sub bx,6
	push si  ;Save position in this macro
	mov si,[fs:bx+macro_parms] ;New pointer to macro parameters
	and al,127	;Clear MSB to get number of parameter
	je >z1
	xor cx,cx
	mov cl,al
z3:	fs:lodsb
	cmp al,_end
	je e1		;Error: this number isn't defined via MACRO directive
	or al,al	;Is there another parameter ?
	jne z3		;No, continue.
	loop z3

z1:	fs:lodsb	   ;Copy parameter to line buffer
	cmp al,_end
	je e1		;Error: Parameter# not defined
	or al,al
	je >z2
	stosb
	jmp z1
z2:	pop si		;Restore position pointer on original source...
	jmp >l1 	; and rave on.

o1:	pop es,ds
	mov w[fs:inputptr],0
	clc
	ret

GETLINE:push ds,es
	mov es,fs	;Load src/dest segments
	mov di,line
	cmp b[fs:flag_macro],1	   ;Assemble Macro?
	jne >n1
		mov si,w[fs:inputptr]
		mov ds,w[fs:seg_macrodata]
		jmp >l1
n1:	inc w[fs:lines] 	   ;(don't count lines within macros)
	inc w[fs:lines_local]
	mov si,w[fs:inputptr]
	mov ds,w[fs:seg_input]
l0:	;delspace		 ;Skip spaces.
	call isErrorLine
	if c fs:mov b[flg_removerr],-1
	call lnb	;Load new source block if needed.
        jc o1

	; main loop
l1:	lodsb		;Get a character.
	cmp al,tab	;Skip TABs
	jne >x1
		mov cx,di
		sub cx,line
		mov al," "
	v1:	inc cl
		stosb
		test cl,111b
		jnz v1
		jmp l1
x1:	cmp al,13h
	jb >e1
	cmp al,20h
	je >n1
		cmp al,";"
		je >r1		;Ignore comments...
		cmp al,'"'
		je >k1		;Copy "string" immediately...
		cmp al,"'"
		je >k1		; "

		test al,128
		jne p1		;Found macro parameter...
		cmp al,_end
		je >m1		;Found end of macro...
		cmp al,"A"
		jb >n1
		cmp al,"Z"
		ja >n1
		add al,"a"-"A"	;Convert char into capital letter
n1:	stosb
	jmp l1

	;return from a macro
m1:	mov bx,w[fs:stc_macros]
	sub bx,6
	mov w[fs:stc_macros],bx
	mov ax,w[fs:bx+macro_parms]	;Reset macro parm buffer to parameters of last macro
	mov w[fs:ptr_macroparms],ax
	mov si,w[fs:bx+macro_parent]	;Return pointer to parent macro source.
	mov w[fs:inputptr],si
	cmp bx,offset stc_macros+2
	jne l1		       ;Return to parent macro...
	mov b[fs:flag_macro],0 ;Return to file.
	mov ds,w[fs:seg_input]
	jmp >n20

k1:	mov ah,al
	stosb
l2:	lodsb
	cmp al,20h
	jb >e1
	stosb
	cmp al,ah
	je l1
	jmp l2

e1:	lodsb			;Sorted!
	cmp al,tab
	je >g1
	cmp al,13 ;20h
	jb e1
g1:	dec si
	jmp >n20

r1:	call get_eol
	jmp >n20

s1:	mov ah,al		;Copy "string" immediately
	stosb
l3:	lodsb
	stosb
	cmp al,ah
	jne l3
	jmp l1

n20:	mov w[fs:inputptr],si
	mov b[es:di],0
	pop es,ds
	mov si,line
	stc
	ret

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
; >>> Initalise data area needed to parse current line
init_line:
	mov di,lineinitstart
	mov cx,(lineinitend-lineinitstart)
init_line2:
	mov es,fs
	xor ax,ax
	rep stosb
	mov al,-1
	mov [operandsize],al   ;(no Operand)
	mov [segregister],al   ;(no segment register)
	mov [override],al      ;(no override)
	ret

init_continueline:
	mov di,lineinitstart2
	mov cx,(lineinitend-lineinitstart2)
	jmp init_line2
align 4
do_jcmd:mov w[currmnemdesc],t_j
	mov b[condition],al
	mov b[actualmnemonic],-1 ;dummy
	add sp,2		;Correct stack and get pointer operand.
	jmp get_pointer
align 4
do_dotdirs:
	mov bx,ax
	add bx,bx
	jmp w[JMP_dotdirs+bx]	;Yes, call its function...
align 4
do_gdirs:
	mov bx,ax
	add bx,bx
	jmp w[JMP_gdirs+bx]	;Yes, call its function...
align 4
do_equal:
	mov es,w[seg_global]
	cmp b[pass],1
	jne >l1
	cmp b[flag_lastlabel],0 ;Was there a label before it?
	je >e1			;No, this is an error.
	inc si
	delspace
	push si
	mov si,w[lastlabel]
	mov dx,w[lastlabelln]
;Im 2. Pass label_update auslesen
;	 call seeklabel 	 ;Search that label.
;	 jc >e2
	call definelabel  ;Define it if it isn't already.
;	 jmp >l2
;l1:	 mov bx,[es:label_update]
;	 push bx,si
;	 call update_defptr
;	 pop si,bx
l2:	pop si
	push es,bx
	call getconstant	;Get the following constant.
	pop bx,es
	mov [es:bx+label_adr],eax
	stc
	ret
l1:	call update_defptr
	stc
	ret

e1:	mov ax,em_labmissing
	jmp error

align 4
do_struc:
	delspace
	cmp b[si],"<"
	if e inc si
	mov bp,w[es:bx+label_adr] ;Get adress of STRUCs info entry
	add bp,4
	;
	push si
	mov si,bp
	xor cx,cx		;Get its size.
	xor ah,ah
l2:	lodsw
	cmp ax,8000h		;(Jump namesize-STRUCsize
	je l2
	or al,al
	jz >l1
	add cx,ax
	jmp l2
l1:	pop si
	;
l4:	cmp b[si],">"		;End of parameters?
	je >l2
l3:	cmp w[bp],8000h 	;STRING parameter?
	jne >l6
	delspace
	lodsb			;Yes, copy to code buffer
	cmp al,'"'
	je >l7
	cmp al,"'"
	jne >e4 		;Error:not a string
l7:	mov ah,al
l2:	lodsb
	cmp al,ah
	je >l4
	or al,al
	je >e1
	call writecode
	jmp l2

l4:	add bp,2
	jmp >l4
e1:	jmp >e1
;n1:	 jmp >n1

l6:	push cx
	call getconstant
	pop cx
	jnc >e2 		;Error: not a constant
	mov dl,b[bp]
	or dl,dl
	jz >e3			;Error: more paramters than entries.
	add bp,4
l5:	call writecode		;Write constant to code buffer
	;db 66h
	shr eax,8
	dec cx
	dec dl
	jnz l5
l4:	delspace
	lodsb
	cmp al,">"
	je >l2
	cmp al,","
	je l3
	or al,al
	je >l2
e2:	mov ax,em_nostruccon
	jmp error
l2:	jcxz >l1
l3:	xor al,al
	call writecode
	loop l3
l1:	stc
	ret

e1:	mov ax,em_dbldef	;Only one label definition per line
	jmp error
e2:	mov ax,em_nostrucval
	jmp error
e3:	mov ax,em_tmuchstruccon
	jmp error
e4:	mov ax,em_stringexpected
	jmp error

align 4
do_macro:
	pusha
	push es
	call update_label
	pop es
	popa
	;
	mov di,w[es:label_adr+bx] ;Get Adress of paramterlist from stack.
	mov bx,w[stc_macros]
	xchg [inputptr],di
	mov w[bx+macro_parent],di
	;
	mov es,fs
	mov di,[ptr_macroparms] ;Store parameters to stack.
	mov w[bx+macro_parms],di
	add bx,6
	mov [stc_macros],bx
	call get_parmlist	;Get paramters past the macro command.
	mov w[ptr_macroparms],di
	mov b[flag_macro],1	;Indicate that source is read from the macro buffer.
	stc
	ret
align 4
do_pcmd:cmp al,8
	jae >l2
	mov bx,pcommands1
	jmp >l3
l2:	cmp al,12
	jae >l2
	sub al,8
	mov bx,pcommands2
	jmp >l3
l2:	cmp al,20
	jae >l2
	sub al,12
	mov bx,pcommands3
	jmp >l3
l2:	sub al,20
	mov bx,pcommands4
l3:	add al,b[bx]
	mov b[pflag],al
	inc bx
	mov w[currmnemdesc],bx
	jmp continue_pcmd
align 4
do_register:
	mov b[opdata+bp],al	;Store number of register on operand info buffer
	mov b[opdesc+bp],r	;and indicate that this operand is a register.
	mov ah,al
	and ah,7		;Update EA info buffer.
	mov b[ea_r],ah
	cmp bp,2
	je >l1
	shr al,3		;Get size of register (number/8)
	call def_opsize 	;and you have the operation size.
l1:	inc bp
	inc b[oplength]
	jmp getnextoperand
align 4
do_segregister:
	mov b[opdata+bp],al
	mov b[segregister],al
	shl al,3		;Create code for XSM byte
				;(op seg,ea or vice versa)
	mov b[ea_s],al
	mov b[opdesc+bp],_sr
	mov al,_w
	call def_opsize
l2:	inc bp
	inc b[oplength]
	jmp getnextoperand
align 4
do_controlregister:
	mov b[opdesc+bp],sysr
	mov b[sysrnum],al
	cmp al,4
	je >l1
	mov b[sysrtype],_cr
	mov b[sysrcode],0
	jmp l2
l1:	mov b[sysrtype],_cr4
	jmp l2
align 4
do_debugregister:
	mov b[opdesc+bp],sysr
	mov b[sysrnum],al
	mov b[sysrtype],_dr
	mov b[sysrcode],1
	jmp l2
align 4
do_taskregister:
	mov b[opdesc+bp],sysr
	mov b[sysrnum],al
	mov b[sysrtype],_tr
	mov b[sysrcode],4
	jmp l2
align 4
do_constant:
	cmp b[si],":"		;absolute adress ?
	jne >n2 		;No, no special handling...
	inc si			;Jump over ':'
	cmp eax,0ffffh
	ja e2			;Error: DWORD segment constant.
	mov w[afseg],ax
	call getconstant	;Get adress part.
	jnc e3			;Error: we miss it...
	cmp eax,0ffffh
	jna >u1
	mov b[addrflag],1	;Force ADDR prefix for 48 bit far adress
u1:	mov [constant],eax
	mov b[constsize],af
	mov b[constflag],1
	inc b[oplength]
	mov b[opdesc+bp],_af
	mov al,_w		;Define AF word
	call def_opsize
	inc bp
	jmp getnextoperand
n10:	jmp >n1

n2:	cmp b[constflag],1
	je >l3			;This is the 2nd constant we found in this line.
	cmp b[varflag],_code
	je >f1

	;Datenlabel
h3:	test b[es:bx+label_flag],_struc ;STRUC label?
	je >a1			;No, check on something else...
	cmp b[si],"["		;Yes, followed	by register combination?
	je >a1			;Yes, don't check on override....
	mov bx,w[es:bx+label_seg] ;Get adress on override
	or bx,bx
	je >a1			;Not found...
	push si
	call initeaconst
	mov si,bx
	mov es,fs
	mov di,eadesc		;Store ea info buffer in STRUC info
	mov cx,eadescend-eadesc
	rep movsb
	pop si
	jmp >a2

l3:	jmp >l3

a1:	call initeaconst	;The label is a variable not a constant.
a2:	cmp b[override],-1
	jmp >k1
	mov cl,b[segments]	;scan ASSUME list for this segment
	or cl,cl
	je >k1			;Not found...
	xor ch,ch
	mov di,tab_assumes
	mov al,b[es:bx+label_seg]
	mov es,fs
	rep scasb
	jne >k1
	mov ax,di
	sub di,tab_assumes-1
	mov b[override],al	;Force override for it.
	;
k1:	mov [eaconst],eax      ;Convert label into a variable
	call eadefconstsize	;We need this code often.
	cmp b[si],"["	  ;Register combination indicated?
	jne >a2 		;No...
	inc si			;Yes, jump over bracket.
	push ea_end
	jmp continueEA		;...and parse register combination.

n1:	jmp >n1
l3:	jmp >l3 		;Some optimisation should kill this off.
				;Someone out there who wants to write that?

a2:	call getmodrmbyte
	jmp getnextoperand

f1:	cmp b[si],"["		;constant[rcombination] ?
	je a1
	or bp,bp		;1st Operand?
	jne >h1 		;No...
	mov dx,w[currmnemdesc] ;Yes, if this an instruction that only needs
	cmp dx,t_1constart	;a constant, create an effective adress for it.
	jb a1
	cmp dx,t_1conend
	ja a1
	;
h1:	mov [constant],eax	;Store constant
	xor dl,dl
	cmp eax,100h		;Check if it's byte size.
	jl >l1
	inc dl
	cmp eax,-80h
	jge >l1
	inc dl
l1:	mov b[constsize],dl
	or bp,bp
	if e mov b[operandsize],dl
	mov b[constflag],1
	mov b[opdesc+bp],co
l2:	inc bp
	inc b[oplength]
	jmp getnextoperand


l3:	inc b[constflag]	;2nd constant is of size BYTE
	mov b[opdesc+bp],co
	mov b[scndflag],1
	mov b[scndconst],al
	jmp l2

e4:	mov ax,em_toomuchregs
	jmp errorhandler

; Initialisiert EA mit Konstante
initeaconst:
	mov b[ealength],0	;# of registers in combination
	mov b[opdesc+bp],e	;Indicate that the operand is an EA
	mov b[eaconstflag],0	;No constant found yet.
	mov w[eadesc],0
	inc b[oplength]
	inc bp			;Pointer -> next operand info slot in buffer
	mov b[ea_sirflag],0	;No scaling
	mov b[ea_scaling],0
	ret

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
; >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; >>> Parses current line and forces code generation for it
;
; Out:	CS = There are more lines waiting...
;	CC = P0ass is finished.
n2:	stc
n1:	ret
eol2:	jmp mapoperands
parse:	mov w[fs:errorstack_sp],sp ;Save stack pointer so we may return
	mov w[fs:errorstack_ss],ss ;savely in case of an error.
	call getline		;Read a line.
	jnc n1			;There's nothing to be read...
parse1: delspace		;Skip first whitespaces
	ONEOL n2		;Blank line, get next one...
	call init_line
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
m1:	xor bp,bp		;Index pointer to info of 1st operand.
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	call getoverride	;Check on override (CS:,DS: and so on)
	call getmnemonic	;Is there a mnemonic?
	jc gotmnemonic		;Yes, parse it.

	cmp b[si],"j"		;is it a conditional jump?
	jne >l1 		;No, they all start with 'j'
	push si
	inc si
	mov di,conditionals	;Get condition code.
	call getstring
	jnc >l2 		;None found...this is no Jcc...
	jmp do_jcmd
l1:	cmp b[processormode],_ppro	;This part is for PPro instructions only
	jne >l1
	cmp w[si],"mc"	;CMOV ?
	jne >l1
	cmp w[si+2],"vo"
	jne >n1 	;No, and no SETcc anyway...
	push si
	add si,4
	mov di,conditionals
	call getstring
	jnc >l2
	mov w[currmnemdesc],t_cmov
	jmp >l3

l1:	cmp w[si],"es"	;SETcc instruction?
	jne >n1
	cmp b[si+2],"t"
	jne >n1
	push si
	add si,3
	mov di,conditionals	;Get condition code...
	call getstring
	jnc >l2 		;There is none...no SETcc...
	mov w[currmnemdesc],t_set
l3:	mov b[condition],al
	mov b[actualmnemonic],-1
	add sp,2
	jmp parse2

l2:	pop si			;No Jcc or SETcc found.
n1:	mov al,b[si]
	cmp al,"."
	je >l1			;Maybe directive starting with a dot...
	cmp al,"#"
	je >l2			;Maybe directive starting with hash mark...
	mov di,st_maindirs	;Standard directive?
	call getstring_fast
	jnc >n1 		;No, go on...
contdir:mov bx,ax
	add bx,bx
	jmp w[jmp_maindirs+bx]	 ;Call function handling the directive...

l1:	inc si
	mov di,map_dotdirs	;.directive ?
	call getstring
	jnc >n1 		;Nope...
	jmp do_dotdirs

l2:	inc si
	mov di,map_gdirs	;#directive ?
	call getstring
	jnc >n1 		;No...
	jmp do_gdirs

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;!!! This should be rewritten and text macros should be supported.
n1:	cmp b[si],"="	  ;Define constant
	jne >n1
	jmp do_equal

e2:	pop ax
	mov ax,em_dblpublic
	jmp error

e1:	mov ax,em_syntaxerror
	jmp error

y1:	mov dx,si
	pop si
	push ax,dx
	call keep_label 	;Save adress and other information about the
				;label we've found. It may be parsed later...
	pop si,ax
	jmp contdir

n1:	call getlabelln
	or dl,dl
	je e1
	push si
	add si,dx
	delspace
	mov di,st_maindirs	;Directive ?
	call getstring_fast
	jc y1			;Yes, call it's function...
	pop si

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov es,w[seg_struc]	;Is it a known STRUC?
	call seeklabel
	jnc >n1 		;No...
	jmp do_struc
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
; >>> Makro insertion
n1:	mov b[flag_local],0
	mov es,w[seg_macro]	;Is it a known macro?
	call seeklabel
	jnc >n1
	jmp do_macro

e2:	mov bx,w[lastlabel]
	add bx,w[lastlabelln]
	cmp b[bx],":"
	jne e1
e200:	cmp b[si-1]," "
	jne >u1
	dec si
	cmp si,line
	jne e200
u1:	mov ax,em_undefdkeyword
	jmp error

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
n1:	cmp b[flag_lastlabel],0 ;Was there a label definition before?
	je >o1			;No, verything ok...
	cmp b[pass],1		;Yes, is it the 1st pass?
	jne e2			;No, error...

o1:	call keep_label
	delspace
	ONEOL >l2		   ;End of line...
	jmp m1
	;
l2:	cmp b[flag_lastlabel],1
	jne >l3
	mov bx,w[lastlabel]
	add bx,w[lastlabelln]
	cmp b[bx],":"
	jne e200
	call update_label	 ;... store/update label entry.
l3:	stc
	ret
	;

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
; >>> Process Mnemonic
eol2b:	jmp eol2
gotmnemonic:
	push ax
	call update_label	;Store/update label definition.
	pop ax
	mov b[actualmnemonic],al ;Store mnemonic's token.
	cmp al,24		;Check if it's a command that has a p-flag in
	jae >l1 		;it opcode.
	jmp do_pcmd
	;
g1:	mov w[lineptr],si	;Prefix: Save current position in line.
	mov si,ax
	jmp createcode		;Generate code for instruction without any
				;operands.
	;
l1:	sub al,24		;Get pointer on generation info and save it.
	add ax,ax
	mov bx,ax
	add bx,tab_mnemonics
	mov ax,[bx]
	mov w[currmnemdesc],ax
	cmp ax,t_debug
	jne >d1
	nop
	nop
	nop
	stc			;We found the DEBUG directive and you can set
	ret			;a breakpoint here, because we just return to
				;the main loop and ignore it.

;a1:	 jmp eol2b

d1:	cmp ax,t_prefixstart	;If we found a prefix it token is saved and
	jb continue_pcmd	;we can continue with the rest of the line.
	cmp ax,t_prefixend
	jb g1
continue_pcmd:
	cmp ax,t_push		  ;PUSH/POP instructions may have multiple
	je >s1			;arguments and we'll have to generate a new
	cmp ax,t_pop		;instruction for each.
	jne >l1
s1:	mov b[flag_contline],1	;Indicate, that we have to save the pointer
				;on the generation info and continue with
				;the next operand.
;jmp contline

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
l1:	cmp ax,t_jmp
	je >l2
	cmp ax,t_j
	je >l2
	cmp ax,t_call
	jne contline

get_pointer:
l2:	delspace
	mov di,map_pointers
	call getstring
	jnc contline
	add ax,ax
	mov bx,ax
	cmp w[currmnemdesc],t_j
	jne >l3
	mov ax,tc_j
	jmp >l2
l3:	mov ax,tc_jmp
	cmp w[currmnemdesc],t_jmp
	je >l2
	mov ax,tc_call
l2:	add bx,ax
	mov ax,w[bx]
	cmp ax,1
	ja >l2
	add ax,em_shortcall
	jmp error

l2:	mov w[currmnemdesc],ax

contline:
	mov di,opdesc		;Intialise pointer on operand info buffer
parse2: delspace
	call getoverride
	mov ax,w[si]
	or al,al		;End of line?
	je eol2b
	ISNUM contco
a1:	
	cmp al,"\"		 ;Continue with next line for the current
				;instruction?
	if e call getline	;Yes, read it.
	;
	; Check for an effective adress like b/w/d[ea].
delspace
	cmp ah,"["	;?[ ?
	jne >n10
	cmp al,"b"
	jne >n1
	xor al,al	;b[
	push ea_end
	jmp parse_ea	;Parse effective adress...

n1:	cmp al,"w"
	jne >n1
	mov al,_w	;w[
	push ea_end
	jmp parse_ea

n1:	cmp al,"d"
	jne >n20
	mov al,_d	;d[
	push ea_end
	jmp parse_ea

n10:	cmp al,"["
	jne >n10
	inc si
	mov b[eaflag],-2	;Undefined size of effective adress.
	push ea_end
	jmp ea_of_unknown_size

n10:	cmp ah," "	;Chars 'b','w' and 'd' indicate the operation size.
	jne >n20	;No "<b/w/d> LABEL"...
	cmp al,"b"
	jne >n1
	mov al,_b
	jmp >k1
n1:	cmp al,"w"
	jne >n1
	mov al,_w
	jmp >k1
n1:	cmp al,"d"
	jne >n1
	mov al,_d
k1:	call def_opsize
	add si,2
	jmp parse2

n20:	call getlabelln
	cmp dl,2
	je >n21 		;Register
	cmp dl,3
	jne >n20
n21:	call getregister	;Check on register.
	jc do_register

	call getoverride	;* Smells like optimisation...
	call getsegregister	;Check on segment register.
	jc do_segregister

	call getcontrolregister
	jc do_controlregister

	call getdebugregister
	jc do_debugregister

	call gettaskregister
	jc do_taskregister

n20:
n1:	mov di,st_sizes
	call getstring_fast	;Check on "byte/word/... ptr"
	jnc >n1 		;No operand size definition like this found...
	call def_opsize
	delspace
	mov di,st_pointers
	call getstring_fast
	jnc >w1 		;Error: PTR keyword missing
	cmp al,_ptr
	jne >e1
w1:	delspace
	call getoverride
	jmp parse2

e1:	mov ax,em_noptr
	jmp errorhandler
e2:	mov ax,em_notawordseg
	jmp errorhandler
e3:	mov ax,em_adrmissing
	jmp errorhandler

contco:
n1:	call getconstant	;Check on contant/label pr갽en.
	jc do_constant		;syntax error
	mov ax,em_syntaxerror
	jmp errorhandler

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

; =================================
; >>> Parses effective adresses <<<
; =================================
; AX=Transfergr붳e
e1:	mov ax,em_twoeaonly	;Too much registers in combination.
	jmp errorhandler
parse_ea:
	call def_opsize
	add si,2		;Jum over '?['
	mov b[eaflag],1
EA_of_unknown_size:
	call getoverride
	call initeaconst
continueEA:                     ;Will start here if it's "constant[combination]"
	push bp
	xor bp,bp
	;
g1:	delspace
	call getregister
	jnc eagetconst          ;No register, must be a constant.
	cmp b[ealength],2       ;Already enough registers found?
	je e1                   ;Yep, time for an error message...
	inc b[ealength] 	;Indicate that we have one more register
	test al,8+16            ; which can't be of size BYTE.
	jz >p1
	mov b[eadesc+bp],al     ;Store register #
	inc bp			;+1 reg
	jmp >n10

p1:     mov ax,em_nobyteeareg   ;Es d걊fen keine BYTE-Register in effektiven
	jmp error               ;Adressen benutzt werden.
	;
eagetconst2:
	dec si                  ;Minuszeichen zur갷kholen
eagetconst:
	call getconstant
	jnc >n1                 ;Kein Label oder Konstante.
o1:     cmp b[eaconstflag],0
	je >d10                 ;Mehr als 1 Konstante ist nicht m봥lich
				;pro Zeile.
	add [eaconst],eax	;Neue Konstante addieren
	mov eax,[eaconst]
	jmp >d11

d10:                            ;Konstantenwert in EAinfo eintragen.
	mov [eaconst],eax
d11:    call eadefconstsize
	jmp >n10
	;
n1:	call getoverride	;Override vorhanden ?
	jc g1                   ;Gefunden, weiterparsen...

	;Auf das Ende der effektiven Adressen pr갽en.
n10:    delspace
	lodsb
	delspace
	cmp al,"]"
	je >x1                  ;end of ea, get ModRM byte
	cmp al,"+"              ;add sign
	je g1
	cmp al,"*"
	je getscaling           ;scaling wanted
	cmp al,"-"
	je eagetconst2		;Only a constant may follow...
	or al,al
	jne >e2
	mov ax,em_missingendbracket
	jmp errorhandler
e2:	dec si			;Correct line pointer for correct error message.
	mov ax,em_misplacedsymbol
	jmp errorhandler

x1:	pop bp
	ret

eadefconstsize:
	mov eax,[eaconst]       ;Check size of constant in effective adress.
	xor dl,dl
	cmp eax,-80h
	jl >w1
	cmp eax,0ffh
	jng >w2
w1:     inc dl
	cmp eax,-8000h
	jl >d1
	cmp eax,0ffffh
	jng >w2
d1:	inc dl
w2:     mov b[eaconstsize],dl
	mov b[eaconstflag],1
	ret

getscaling:
	call getconst2		;Get constant for scaling.
	xor dx,dx
	mov cl,8
l1:     shr al,1
	jnc >n1
	or dh,dh                ;More than one bit used?
	jnz >e5
	mov dh,dl
n1:     inc dl
	dec cl
	jnz l1
	shl dh,6                ;Create s bit of sir opcode byte.
	mov b[ea_scaling],dh
	mov b[ea_sirflag],1     ;Indicate scaling and sir generation.
	jmp n10
e5:     mov ax,em_wrongscaling
	jmp errorhandler

g2:     jmp e2

ea_end: call getModRMbyte       ;End of line, generate ModRM opcode byte.
getnextoperand:
	delspace
	ONEOL >c2		;end of line, create code
	lodsb
	cmp al,"\"
	jne >g0 
	call getline
	lodsb
g0:     cmp al,","              ;Operand separator?
	jne g2                  ;no: misplaced built-in symbol
	cmp b[flag_contline],1
	je >c3                  ;Generate code and continue with rest of line...
	ONEOL >e6
	cmp bp,3
	je >e7
	jmp parse2              ;ok, next operand
e7:     mov ax,em_toomuchops
	jmp errorhandler
e6:	mov ax,em_unexpectedeol ;Error: Unexpected end of line.
	jmp errorhandler

c2:     mov b[flag_contline],0
	jmp mapoperands
c3:	mov w[lineptr],si
	jmp mapoperands

; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

; Create operand size code from operand info buffer
; _b,_w,_d,wb,_dw & RAND-Flag
; In: AL=size
def_opsize:
	;write code to destination segment
	mov ah,b[operandsize]
	cmp ah,-1
	jne >l2
	mov b[operandsize],al   ;Single operand.
	ret
l2:     cmp ah,al               ;Same operand sizes.
	je >l3
	cmp ax,200h
	jne >l4
	mov b[randflag],1	;rd,rb
	mov b[operandsize],wb
	ret
l4:     cmp ax,100h
	jne >l1 		;rw,rb
	mov b[operandsize],wb
l3:	ret
l1:     cmp ax,201h
	jne >l1
	mov b[randflag],1
	mov b[operandsize],_dw  ;rd,rw
	ret
l1:     cmp ax,1h
	jne >l1
	mov b[operandsize],bw
	ret
l1:     mov ax,em_notsamesize ;Error: Operand size mismatch.
	jmp errorhandler


