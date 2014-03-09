;TMA macro assembler
;Identifier handling
; (c)1996-1998 Sven Klose

	.data
used_labels_c:	dw ?	;Number of currently defined code labels
used_labels_l:	dw ?	; "                          local labels
used_labels_d:	dw ?	; "                          data labels
used_labels:	dw ?	;			     total labels.
flag_local:	db ?		;0=public 1=local
ac_label_flag:	db ?		;Special label flags from other modules 
				;(f.e.STRUC)
cnt_internal:	dw ?		;# of internal labels
getlost:	dw ?

current_namespace	dd ?	;Pointer on current namespace info record
next_namespace		dd ?	;The same for the next one
ptr_namespace		dw ?
seg_namespace		dw ?

	align 4
map_legalchars: db 256 dup ?
key_hash1:	db 256 dup ?
key_hash2:	db 256 dup ?

	.code
;Header of label table
lab_header struc
			dd ?	;Segment size
	label_starts:	dw 256 dup ?
	label_redefpos: 	;Last stored local label.
	label_ends:	dw 256 dup ?
	label_update:	dw ?	;Pointer on next label that will be updated.
				;NOT used in passes 2+ to avoid a full search.
	label_end:	dd ?	;Pointer of free space for label definition.
	label_cnt:	dd ?	;# of currently defined labels.
	label_data:		;Position of first entry:
	ends

;Header of a label entry
lab_main struc
	label_adr:	dd ?	;absolute 32 bit address.
	label_seg:	dw ?	;Segment #,0 if undefined.

	label_last:	dw ?	;Alphabetically linked
	label_next:	dw ?

	label_nstart:	dw ?	;Info record of this namespace

	label_line:	dw ?	;Line # of definition.
	label_flag:	db ?	;Status:
				;0=Code label
				;1=Data label
				;2=STRUC name
				;3=namespace info record
				;128=public
				;64=extern

	label_nameln:	db ?	;Name length
	label_name:		;Name
	label_localref: dw ?	;Last label with same name (only local)
				;0=This is the first entry.
	ends

namespace_record struc
	starting_line	dw ?
	ending_line	dw ?
	childs_export	db ?	;Chids export identifiers, so we'll have to
				;search there, too.
	ends

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; Label search:
;
; Double definitions will be checked in the first pass where forward
; references will be faked.
; Due to the fact that the order within the table won't change, new searches
; are not needed anymore when processing symbol definitions.
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; Local labels:
;
; The first local label of a multiple name contains a pointer on the last
; entry with this name. If there is none, the pointer is 0.
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

enum	undefined,defined

; Labels are linked in alphabetical order. This macro gets the address of the
; first label that starts with the character in AL.
get_tabstart macro
	xor bh,bh
	mov bl,al
	add bx,bx
	#em

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; There are three different flavours of labels:
;	local labels LOCAL
;	public labels PUBLIC
;	public data labels/variables DATA

keep_label:
	mov w[lastlabel],si	;Store the position of the label we found.
	mov b[flag_lastlabel],1 ;Indicate its occurence at line's start.
	call getlabelln
	mov w[lastlabelln],dx	;Store its length.
	add si,dx		;Jump over it
	cmp b[si],":"		;(we'll analyze this later on)
	if e inc si
	ret			;The label will be stored when we call
				;update_label. It's done this way, because
				;we don't know what follows and how to analyze
				;it

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
n1:	ret

db "BOOLSHIT"
boolshit:
	cmp bx,[es:label_update]
	if ne nop
	ret

; ===================================================
; >>> Stores new label or updates its information <<<
; ===================================================
update_label:
	cmp b[flag_lastlabel],0 ;Is there a definition?
	je n1			;No, return...
	inc w[used_labels]
	push si,bp		;Save position in line.
	mov si,w[lastlabel]	;Get name.
	mov b[flag_lastlabel],0
	mov dx,w[lastlabelln]
	cmp b[generation_mode],_data
	jne >n1

; >>> store variable <<<
	inc w[used_labels_d]
	cmp b[pass],1
	jne >g1
	;
	mov es,w[seg_global]	;Check if label is already defined.
	call seeklabel		;Local labels won't be checked.
	jc >e2
	mov es,w[seg_data]
	call seeklabel
	jc >e3
	;
n2:	call definelabel	;No, define it.
	mov eax,[codeptr]	;Store its code position.
	mov [es:bx+label_adr],eax
	mov ax,[lastsegflag]
	mov [es:bx+label_seg],ax
	;
l4:	cmp b[si],":"		;':' indicates a code label
	jne >l1
	mov b[es:bx+label_flag],_code
	jmp >l6
	;
l1:	mov al,_data		;It's a data label.
	or al,b[ac_label_flag]	;(unknows flags)
	mov b[es:bx+label_flag],al
	jmp >l6

	;Pass!=1
g1:	mov es,w[seg_data]	;Search label
	mov bx,[es:label_update]
	;add si,dx
        call seeklabel
	call boolshit
	cmp [codeptr],eax
	je >l6
	mov ax,[lastsegflag]
	mov [es:bx+label_seg],ax
	mov eax,[codeptr]
	mov [es:bx+label_adr],eax
	jmp l4

l6:	pop bp,si
update_defptr:
	cmp b[pass],1
	je >l7
	push bx,dx,ds
	mov ds,fs
	xor dx,dx
	mov dl,[es:bx+label_nameln]
	mov eax,[es:bx+label_adr]
	add bx,dx		;Update pointer -> next label
	add bx,offset label_name
	cmp b[generation_mode],_data
	je >l5
	cmp b[flag_local],0
	if ne add bx,2
	mov w[es:label_update],bx
l5:	pop ds,dx,bx
l7:	ret

e2:	mov ax,em_alreadypublic ;Label is already defined.
	jmp error
e3:	mov ax,em_alreadydata
	jmp error

; >>> Store location <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
n1:	mov ax,w[codeptr]
	call getlabelmode	;Get label's scope (public/local)
	cmp b[flag_local],1
	jne >l1
	inc w[used_labels_l]
	jmp >l2
l1:	inc w[used_labels_c]
l2:	cmp b[pass],1
	je >l1

	;2.-x. Pass
	mov bx,[es:label_update]
	;add si,dx
	call seeklabel
	call boolshit

	cmp [codeptr],eax
	je l6
	mov eax,[codeptr]
	mov [es:bx+label_adr],eax
	mov b[newpass],1
	jmp l6

; Store location (public/local) in 1st Pass
l1:	call seeklabel		;Search label (public/local).
				;Local labels are not found in the first label.
	jc >e1			;Error: Label is already defined.
	call definelabel
	delspace
	mov b[es:bx+label_flag],_code ;Code label.
	mov ax,[lastsegflag]
	mov [es:bx+label_seg],ax
	pop bp,si
	ret

e1:	mov ax,em_dblpublic
	jmp error

e2:	mov ax,em_labnotfound	;Label not found
	jmp error		;(Internal error, send a postcard)

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Initalize label table <<<
;
; In:	AX	Segment of table
;	CX	Size in bytes
clearvarseg:
	push ax
	mov es,ax
	xor di,di
	mov ax,cx
	stosw
	xor eax,eax
	mov cx,label_end/2-1
	rep stosw
	mov ax,label_data ;Pointer to free space.
	stosd
	xor ax,ax
	stosd
	pop ax
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Defines label <<<
;
; In:	ES	Label segment
;	DS:ESI	Name
;	DX	Name length
; Out:	BX	Address of next entry
definelabel:
	mov di,w[es:label_end]
	mov bp,di
	mov eax,[codeptr]
	stosd
	mov ax,w[lastsegflag]
	stosw
	;
	lodsb				;Start of first entry -> BP
	dec dl
	get_tabstart
	mov ax,w[es:label_ends+bx]	;Pointer to last entry.
	or ax,ax			;1st entry?
	jnz >h1 			;No...
	mov w[es:label_starts+bx],bp	;Store this address in header.
	jmp >h2
h1:	push bx
	mov bx,ax			;Link this entry to the last one.
	mov w[es:label_next+bx],bp
	pop bx
h2:	stosw				;Pointer on last entry.
	xor ax,ax
	stosw				;Pointer on next one (there is none)
	mov w[es:label_ends+bx],bp
	;
	xor ax,ax			;Pointer on namespace info record
	stosw
	;
	mov ax,w[line]			;Line #
	stosw
	mov al,b[ac_label_flag]
	mov ah,dl
	stosw				;defined/name length
	mov cx,dx			;Copy name string.
	rep movsb
h3:	cmp b[flag_local],1		;Local label?
	jne >l1 			;No...
	xor ax,ax			;(label_localref)
	stosw
l1:	mov bx,w[es:label_end]		;Save pointer for EQU directive.
#if _labalign
	test di,11b
	jz >a1
	and di,-4
	add di,4
a1:
#endif
	mov w[es:label_end],di		 ;New position of free space.
	inc w[es:label_cnt]
	cmp di,[es:0]
	jb >l1
	mov ax,em_labbuffull		;Errro: Out of memory.
	jmp error			;Not a user friendly check...
l1:	inc dl
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Gets length of label name <<<
;
; In:	DS:SI	Name, end will be recognized.
; Out:	DX	length in bytes
getlabelln:
	push ax,si,bx
	mov bx,map_legalchars
	cmp b[si],">"
	if e inc si
	mov dx,si
l1:	lodsb
	xlat
	or al,al
	jz l1
	xchg dx,si
	sub dx,si
	dec dx
	pop bx,si,ax
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Checks scope of label (public/local) <<<
; Will set flag_local.
getlabelmode:
	mov al,b[si]
	cmp al,">"
	je >l3
l4:	cmp al,"a"
	jb >n1
	cmp al,"z"
	ja >n1
	mov cx,dx
	dec cx		
	jz >n1		;Label is 1 char...public
	push si 	;Check if only digits follow.
	inc si
l2:	lodsb
	cmp al,"0"
	jb >n2		;No...
	cmp al,"9"
	ja >n2
	loop l2
	pop si
l1:	mov es,w[seg_local]
	mov b[flag_local],1
	ret

n2:	pop si
n1:	mov es,w[seg_global]
	mov b[flag_local],0
	ret

l3:	inc si		;'>label' must be local!
	mov al,b[si]	;Check that.
	call l4
	dec si
	cmp b[flag_local],1
	if e ret	;Ok, return.
	mov ax,em_labnotlocal ;Error: ">" is reserved for local labels.
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Searches label (public/local) <<<
;
seek_addr:
	cmp b[flag_local],1
	jne seeklabel
	;
	cmp b[si],">"		;Local label (A86 like). Check it's direction.
	jne >n1
	inc si
	call seeknextlocal
	ret
n1:	call seeklastlocal
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Searches label <<<
;
; In:	DS:SI	Label name
;	ES	Segment of table we want to search.
; Out:	EAX	Address
;	BX	Entry position in table or pointer to next entry if label
;		wasn't found.
seeklabel:
	cmp b[flag_local],1
	je seeklocal
seeklabelb:
	mov cx,dx
	mov ax,w[es:label_cnt]
	or ax,ax
	jz >n2
	dec cx
	lodsb			;Get start of table for this character.
	get_tabstart
	mov bx,w[es:label_starts+bx]
	or bx,bx
	jz >n3			;Label not found (no table)
	;First entry gefunden, address in BX
seeknextlabel2:
l2:	push si,cx
	mov al,b[es:bx+label_nameln]
	cmp cl,al	;Same length?
	jne >l1 	;No, try next entry...
	or al,al
	jz >f1
	mov w[fs:getlost],bx
	mov di,bx
	add di,label_name
	repe cmpsb	;Compare names.
	jne >l1
	jcxz >f1
l1:	pop cx,si	;Names are different.
	mov bx,w[es:label_next+bx] ;Address of next entry.
	or bx,bx
	jnz l2		;Try next entry...
n3:	dec si
n2:	clc		;Label not found.
	ret

f1:	pop ax,ax		;Label found.
	test b[es:bx+label_flag],_data
	if ne mov b[varflag],1	;It's a data label.
	mov eax,[es:bx+label_adr]
	stc
	ret

seeknextlabel:
	mov cx,dx
	dec cx
	inc si
	jmp seeknextlabel2

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Search local label <<<
;
seeklocal:
	cmp b[pass],1		;Don't find anything in the first pass.
	je n2
	call seeklabelb 	;Get first occurence.
	jnc n2			;Label not found...
	add bx,dx
	mov ax,w[es:bx+label_localref-1] ;Get reference pointer 
	sub bx,dx
	mov di,bx
	or ax,ax		;Find this first label?
	je >l1			;Yes, store the entry position so we can
				;clear its 'localref' before next pass.

	mov bx,ax
	mov bx,w[es:label_next+bx] ;No, find label past LOCAL_REF.
	or bx,bx
	jz >n2			;No more labels.

	sub si,dx		;Point to label name
	push di
	call seeknextlabel	;Get next label with this name.
	jnc >n2 		;Error: Label not found...
	pop di
l10:	mov eax,[es:bx+label_adr]
	add di,dx
	mov w[es:di+label_localref-1],bx ;Save new pointer on free space.
	stc
	ret

l1:	push es,bx		;Store address of entry in table 'localclr'
	mov ax,bx		;because its 'local_ref' must e cleared 
	add ax,dx		;before the next pass.
	add ax,label_localref-1
	mov es,[seg_localclr]
	mov bx,[ptr_localclr]
	mov w[es:bx],ax
	add bx,2
	cmp bx,size_localclr*16
	je >e1
	mov w[ptr_localclr],bx
	pop bx,es
	jmp l10

n2:	pop di
n2:	clc
	ret

e1:	mov ax,em_full_localclr
	jmp break

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Search last local label <<<
;
seeklastlocal:
	call seeklabelb 	;Get first occurence
	jnc n2			;Label not found...
	add bx,dx
	mov ax,w[es:bx+label_localref-1] ;Position of last label.
	sub bx,dx
	mov di,bx
	or ax,ax
	je >l10 		;Firt element is valid enough...;-)

	mov bx,ax		;Skip this label.
l10:	mov eax,[es:bx+label_adr]
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Search next local label <<<
;
seeknextlocal:
;	 mov es,[seg_local]
	cmp b[pass],1		;Don't find anything in the first pass.
	je n2
	call seeklabelb
	jnc n2			;Label not found...
	add bx,dx
	mov ax,w[es:bx+label_localref-1]
	sub bx,dx
	mov di,bx
	or ax,ax		;Find this one?
	je >l11 		;Error: no more labels...

	mov bx,ax
	mov bx,w[es:label_next+bx] ;No, get next...
	or bx,bx
	jz n2

	sub si,dx		;(pointer to name position)
	call seeknextlabel
	jnc n2			;Error: Label not found.
l10:	mov eax,[es:bx+label_adr]
	stc
	ret

l11:	mov ax,w[es:di+label_adr] ;First label is located before current
	stc			;code position.
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Clears 'local_ref's of all entries store in 'local_clr' <<<
;
clearlocals:
	mov cx,w[ptr_localclr]
	shr cx,1
	jcxz >d1		;nowt found...
	push ds,es
	mov es,w[seg_local]
	mov ds,w[seg_localclr]
	xor ax,ax
	xor si,si
l1:	mov bx,w[si]
	add si,2
	mov w[es:bx],ax
	loop l1
	pop es,ds
	mov w[ptr_localclr],ax
d1:	ret

init_hash:
	mov si,key_hash1
	mov di,key_hash2	;Create hash key.
	mov bp,map_legalchars
	mov cx,256
	xor bl,bl
l1:	cmp b[bp],-1
	je >l3
	mov b[si],bl
	mov dx,128 by 8
	xor al,al
	mov bh,bl
l2:	shr bh,1
	if c or al,dh
	shr dh,1
	dec dl
	jnz l2
	mov b[di],al
	inc bl
l3:	inc bp
	inc di
	inc si
	loop l1
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Stores label name to a stringlist <<<
;
; In:	SI	Label name position
;	DI	Position of list.
label2list:
	mov al,_end
	mov cx,-1
	repne scasb
	mov b[di-1],0
	call getlabelln
	mov cx,dx
	rep movsb
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ


