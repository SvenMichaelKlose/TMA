;TMA macro assembler
;Various string functions
; (c)1996-1997 Sven Klose

	.code

sortsize=20

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; >>> Search end of line at DS:SI
get_eol:
r1:	lodsb		     ;Ignore comment (scan end of line)
	cmp al,13
	je >a1
	cmp al,10
	jne r1
a1:	lodsb
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
copy_line:
r1:	lodsb		     ;Ignore comment (scan end of line)
	stosb
	cmp al,13
	je >a1
	cmp al,10
	jne r1
a1:	movsb
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; Copies parameters to a list of ASCIIZ strings
; Quelle: DS:SI Ziel: ES:DI
; Out:	CX = # of parameters-
get_parmlist:
l2:	delspace
l3:	ONEOL >n1
	lodsb
	cmp al,","
	je >l1
	cmp al," "
	je >l1
	stosb
	cmp al,"'"
	je >a1
	cmp al,'"'
	je >a1
	jmp l3
l1:	xor al,al
	stosb
	jmp l2
n1:	mov ax,_end by 0
	stosw 
	ret
a1:	mov ah,al
a2:	lodsb
	stosb
	cmp al,ah
	je >a3
	or al,al
	je >e1
	jmp a2
a3:	jmp l3

e1:	mov ax,em_strnotended
	jmp error

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getdirective:
	mov di,st_maindirs
	call getstring_fast
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getcontrolregister:
	cmp w[si],"rc"	;Check on "cr" string
	jne >n1
	mov al,b[si+2]
	cmp al,"0"
	jb >n1
	cmp al,"4"	;CR4 will be handled differently.
	ja >n1
	sub al,"0"
	cmp b[si+3],"/"
	ja >n1
	add si,3
	stc
	ret
n1:	clc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getdebugregister:
	cmp w[si],"rd"
	jne n1
	mov al,b[si+2]
	cmp al,"0"
	jb n1
	cmp al,"7"
	ja n1
	cmp al,"4"
	je n1
	cmp al,"5"
	je n1
	sub al,"0"
	cmp b[si+3],"/"
	ja n1
	add si,3
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

gettaskregister:
	cmp w[si],"rt"
	jne n1
	mov al,b[si+2]
	cmp al,"4"
	je >o1
	cmp al,"6"
	jb n1
	cmp al,"7"
	ja n1
o1:	sub al,"0"
	cmp b[si+3],"/"
	ja n1
	add si,3
	stc
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getsegregister:
	mov di,st_segregisters
	call getstring_fast
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getoverride:
	push si
	mov di,st_segregisters
	call getstring_fast
	jnc >n1
	cmp b[si],":"		;Segment/adress ?
	jne >n1
	inc si
	cmp b[override],-1	;Check if there is one already.
	jne >e1 		;Yes, Error: "double segment override"
	mov b[override],al	;Store override token.
	pop di
	stc
	ret
n1:	pop si
	clc
	ret
e1:	mov ax,em_dbloverride
	jmp errorhandler
	
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getregister:
	mov di,st_mainregisters
	call getstring_fast
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

getmnemonic:
	mov di,st_mnemonics
	call getstring_fast
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; ======================
; >>> get string no. <<<
; ======================
; Input:
; SI=input string
; DI=strings
; Output:
; AX=no.
Getstring:
	push es
	mov es,fs
	call getstring2es
	pop es
	ret

; You may set ES for the destination segment here
getstring2es:
	push ax,cx,bp,si,di
	mov bp,si		;Save origin of string
	xor cx,cx		;Nothing found yet.
	cmp b[es:di],_end	   ;no entry found in search list
	je >f5
	cmp b[si],0
	je >f5
l3:	mov si,bp
l1:	mov al,b[es:di] 	   ;Get character
	or al,al
	je >n3			;String maybe found...
	cmp al,"/"
	je >n3			;"
	cmp al,_end		;End of string table
	je >n3			;Maybe found...
	cmp b[si],al
	jne >l2 		;Not the same char...
	inc di
	inc si
	jmp l1

f5:	pop di,si,bp,cx,ax
	clc			;String not found.
	ret

n3:	dec di
	mov al,b[si]
	cmp al,"/"
	jbe >l4
	cmp al,":"
	jb >l5
	cmp al,"@"
	jb >l4
	cmp al,"["
	jb >l5
	cmp al,"_"
	je >l5
	cmp al,"/"
	je >l4
	cmp al,"`"
	ja >l5
l4:	pop di,ax
	mov ax,cx
	pop bp,cx
	add sp,2
	stc
	ret
l5:	cmp b[si],_end
	je f5
; update pointers to compare next string
l2:	inc di
l1:	mov al,b[es:di]
	inc di
	or al,al
	je >n1		;Get next string.
	cmp al,"/"	;Try alias string
	je l3
	cmp al,_end	;End of table, no string found
	je f5
	jmp l1

n1:	inc cx
	jmp l3

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; ======================
; >>> get string no. <<<
; ======================
; Input:
; BP=input string
; DI=strings
; Output:
; AX=no.
;
; Note: Can also handle filenames.
Getstring2:
	push cx,bp,si,di
	mov bp,si		;Save origin of string
	xor cx,cx		;Nothing found yet
	cmp b[di],_end
	je >f5			;End of table, string not found.
l3:	mov si,bp
l1:	mov al,b[di]		;Get character
	or al,al
	je >n3			;String maybe found...
	cmp al,"/"
	je >n3			;"
	cmp al,_end		;End of string table
	je >n3			;Maybe found...
	cmp b[si],al
	jne >l2 		;Not the same char...
	inc di
	inc si
	jmp l1

f5:	pop di,si,bp,cx
	clc			;No string found...
	ret

n3:	dec di
	mov al,b[si]
	cmp b[si],21h
	jae >l5
	pop di,ax
	mov ax,cx
	pop bp,cx
	stc
	ret
l5:	cmp b[si],_end		;Update pointers to compare next string...
	je f5
l2:	inc di
l1:	mov al,b[di]
	inc di
	or al,al
	je >n1		;Try next entry...
	cmp al,"/"
	je l3		;Try alias string...
	cmp al,_end
	je f5		;End of table, string not found
	jmp l1

n1:	inc cx
	jmp l3

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

init_symbols:
	mov w[symbolend],symbols
	mov si,mnemonics
	mov bp,st_mnemonics
	call init_string_table
	mov si,segregisters
	mov bp,st_segregisters
	call init_string_table
	mov si,mainregisters
	mov bp,st_mainregisters
	call init_string_table
	mov si,map_maindirs
	mov bp,st_maindirs
	call init_string_table
;	 mov si,map_dotdirs
;	 mov bp,st_dotdirs
;	 call init_string_table
	mov si,map_sizes
	mov bp,st_sizes
	call init_string_table
	mov si,map_pointers
	mov bp,st_pointers
	call init_string_table
	mov si,map_constants
	mov bp,st_constants
	call init_string_table
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Initialises string table for fast scan <<<
; SI=Sourcelist of keywords
; BP=Lookup table for 1st char
;
init_string_table:
	mov di,freespace
	xor eax,eax
	mov cx,sortsize*64
	rep stosd
	;
	mov di,freespace
	xor cx,cx
	xor bl,bl
l3:	push di
	cmp b[si],"-"
	jne >l5
	add si,2
	inc bl
	pop di
	jmp l3
l5:	inc cx
	mov al,bl		;Save string token #
	stosb
l2:	lodsb			;Copy string to table
	cmp al," "
	jb >l1
	stosb
	jmp l2
	;
l1:	mov ah,al
	xor al,al		;Set end-of-string marker (0)
	stosb
	pop di
	inc bl
	add di,sortsize
	cmp ah,_end		;End od sourcelist?
	jne l3			;Nope, go on...
	;
	mov si,freespace
	push cx 		;Save # of entries
	dec cx
	jz >n1
l1:	push si,cx
	mov di,si
	add di,sortsize
l2:	push cx,si,di
	mov cx,-1
	inc si
	inc di
	repe cmpsb
	pop di,si
	jb >l6
	mov ax,si
	mov dx,di
	mov di,buffer16
	mov cx,sortsize/4
	rep movsd
	mov si,dx
	mov di,ax
	mov cx,sortsize/4
	rep movsd
	mov si,buffer16
	mov di,dx
	mov cx,sortsize/4
	rep movsd
	mov si,ax
	mov di,dx
l6:	add di,sortsize
	pop cx
	loop l2
	pop cx,si
	add si,sortsize
	loop l1
	;
n1:	pop cx		;Remind # of entries
	mov di,bp
	push cx
	mov cx,26
	xor ax,ax	;Initialise lookup table with empty entries.
	rep stosw
	pop cx
	;
	mov di,w[symbolend]	;Get free space to save whole table
	mov si,freespace
	push si
	jmp >l2

l5:	push si
	cmp ah,b[si+1]
	je >l1
	dec di
	mov al,_end
	stosb			;Mark end of table
l2:	mov ah,b[si+1]
	mov bl,ah
	sub bl,"a"
	xor bh,bh
	add bx,bx
	add bx,bp
	mov w[bx],di		;Store position of table
l1:	movsb			;Copy current string to table
	inc si			;1st char is already known
l4:	lodsb
	or al,al
	je >l3
	stosb
	jmp l4
	;
l3:	stosb
	pop si
	add si,sortsize
	loop l5
	dec di
	mov al,_end		;Don't forget to mark end of last table
	stosb
	mov w[symbolend],di
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; ======================
; >>> get string no. <<<
; ======================
; Input:
; BP=input string
; DI=string lookup table
; Output:
; AX=no.
getstring_fast:
	push cx,bp,si,di,es,dx
	mov es,fs
	call getlabelln
	dec dx
	jz >f5
	mov bp,si		;save origin of string
	mov bl,b[bp]
	sub bl,"a"
	jc >f5
	cmp bl,"z"-"a"
	ja >f5
	xor bh,bh
	add bx,bx
	mov di,w[di+bx] 	;Get table adress from lookup table
	or di,di
	je >f5			;No entry for this character...
	inc bp
l3:	mov ah,b[di]		;Get string number
	inc di
l6:	mov si,bp
l1:	mov cx,dx
	repe cmpsb
	jne >l2
	jcxz >n3		;String found.
	
f5:	pop dx,es,di,si,bp,cx
	clc		;no string found
	ret

n3:	mov al,b[di]
	cmp al,":"
	jne >l7
	cmp b[si],al
	jne >l5
	inc si
	jmp >l4
l7:	cmp al," "
	jne >l7
	cmp b[si],al
	jne >l5
	inc si
	call getlabelln
	jmp l6
l7:	cmp al,"/"
	jbe >l4
	cmp al,":"
	jb >l5
	cmp al,"@"
	jb >l4
	cmp al,"["
	jb >l5
	cmp al,"_"
	je >l5
	cmp al,"`"
	ja >l5
l4:	pop dx,es,di
	add sp,2
	mov al,ah
	xor ah,ah
	pop bp,cx
	stc
	ret

l5:	cmp b[di+1],_end
	je f5

; update pointers to compare next string
l2:	dec di
l0:	mov al,b[di]
	inc di
	or al,al
	je l3		;Try next entry...
	cmp al,"/"
	je l6		;Try alias entry...
	cmp al,_end
	je f5		;End of table no string found...
	jmp l0

copy_stringlist:
	lodsb
	stosb
	cmp al,_end
	jne copy_stringlist
	ret

