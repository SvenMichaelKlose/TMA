	.code

bspStruc struc
	bspLeft 	dd ?
	bspRight	dd ?
	bspPlane	dd ?
	ends

;Initialize tree with root plane
;EBX=new plane
initbsp:mov edi,bspTree
	xor edx,edx
	jmp >l3

;Add plane to tree
put2bsp:mov esi,bspTree 	;get root address
	mov edi,bspTreeEnd	;get pointer to end of tree
	mov cx,za[ebx]
l1:	mov ebp,bspPlane[esi]
	ds:mov ax,za[ebp]
	cmp ax,cx
	jl >l2
	mov edx,bspLeft[esi]
	or edx,edx
	jz >w1
	mov esi,edx
	jmp l1

l2:	mov edx,bspRight[esi]
	or edx,edx
	jz >w2
	mov esi,edx
	jmp l1

w2:	mov bspRight[esi],edi
	jmp >l3

w1:	mov bspLeft[esi],edi
l3:	xor eax,eax
	stosd
	stosd
	mov eax,ebx
	stosd
	mov bspTreeEnd,edi
	ret

;Reading the tree:
;
;After storing the planes into the tree, sorting them still isn't done.
;The next procedure has to read out the tree the right way, which I'll
;just explain:
;
;1. Get the root of the tree
;2. Recursively call this procedure at "m1" for the left child node of the
;   tree, in case there's something stored. (Usually a plane in front of
;   the caller's one.
;3. Store our plane into our the destination list.
;4. Recursively call this procedure like we did in 2.
;5. Done.
;
;Differently to "put2bsp" this code always needs the same amount of time
;to read out the tree, even if we sort in the same planes in a totally
;different order.
	align 4
get_bsp:mov esi,bspTree
	mov edi,sortlist
	;
m1:	mov eax,bspRight[esi]
	or eax,eax
	jz >n1		;Ende des Baumes erreicht, linken Zweig versuchen
	push esi
	mov esi,eax
	call m1
	pop esi
	;
n1:	mov ax,bspPlane[esi]   ;Fl„che in Liste eintraegn
	stosw
	;
	mov eax,bspLeft[esi]
	or eax,eax
	jz >n1		;Ende des Baumes erreicht, linken Zweig versuchen
	mov esi,eax
	call m1
	;
n1:	ret

	.data
	align 4
bspTreeEnd	dd ?
bspTree:	dd (1300*3) dup ?

