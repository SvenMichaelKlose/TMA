;TMA macro assembler
;Global settings and macros
; (c)1996-98 Sven Klose

	.code
	p1=7	;p groups and their tokens.
	p2=11
	p3=19
	p4=23

	tab=9

;Skip spaces.
_delspace:
d100:	cmp b[si]," "
	jne >d101
	inc si
	jmp d100
d101:	ret

delspace macro
	call _delspace
	#em

;Jump if at end of line
ONEOL	macro dest
	cmp b[si],0
	je dest
	endm

; Decrement offset of segment:offset pair as much as possible
SEGOFSALIGN macro seg,off
	push eax,ebx
	mov eax,off
	xor ebx,ebx
	shr eax,4
	mov bx,seg
	add eax,ebx
	mov seg,ax
	pop ebx,eax
	and off,15
	endm

pushs	macro segm
	push segm
	endm

pops	macro segm
	pop segm
	endm

initstackd macro adr
	mov d fs:[adr],adr
	endm

pushstackd macro adr,value
	push eax,ebx
	mov ebx,adr
	mov eax,value
	mov fs:[ebx],eax
	pop ebx,eax
	endm

popstackd macro adr,reg
	push ebx
	mov ebx,adr
	xor reg,reg
	cmp [ebx],adr
	je >m1
	mov reg,fs:[ebx]
	sub d[adr],4
m1:	pop ebx
	endm

ISNUM	macro dest
	cmp al,"0"		;Is it a numeric constant?
	jb >y1
	cmp al,"9"
	jna dest	      ;Yes, skip rest of checks....
y1:	
	endm


