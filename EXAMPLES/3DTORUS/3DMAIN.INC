; Trifiller - (c)1997 ba'al shem

	.data
planelimit	dd ?
scrbuffer	dd ?	    

transformed_points: db (700*pointsize)	dup ?
new_planelist:	db (1500*planesize) dup ?
sortlist:	dw 2000 dup ?
	
	.code
tzoom	= 1200

pointstruc struc
	x      dw ?
	y      dw ?
	z      dw ?

	ends
pointsize=6

clip_struc struc
	dd ?
	cldt	dd ?	;delta
	clh	dw ?
	cll	db ?
	ends
c_size=8

planestruc struc
	pt1	dw ?	;Coordinates of the plane
	pt2	dw ?
	pt3	dw ?
	pc	dw ?	;color/texture #
	za	dw ?	;average of all Z coordinates
	ends
planesize=10

objectstruc struc
	objptr	dd ?	;Position of Object
	xr	dw ?	;Rotation
	yr	dw ?
	zr	dw ?
	xt	dw ?	;Translation
	yt	dw ?
	zt	dw ?
	ends

;Draws world
draw_world:
	call init_light
	mov es,cs		;transform all points of object
	mov si,pointlist
	mov di,transformed_points
	lodsw
	mov cx,ax
l1:	push cx
	call rotate
	pop cx
	dec cx
	jnz l1
	;
	mov si,planelist	;copy planelist which will be totally
	mov di,new_planelist	;transferred later on.
	lodsw			;# of planes
	cmp ax,planelimit
	if a mov ax,planelimit
	stosw
	mov bx,planesize/2
	cwd
	mul bx
	mov cx,ax
	rep movsw
	;
	mov si,new_planelist	    
	lodsw
	mov cx,ax
l1:	push cx
	mov bp,pt1[si]	;transfer #s of points to adresses
	mov di,pt2[si]
	mov bx,pt3[si]
	shl bp,3		
	shl di,3
	shl bx,3
	mov ax,transformed_points
	add bp,ax
	add di,ax
	add bx,ax
	mov pt1[si],bp
	mov pt2[si],di
	mov pt3[si],bx
	;
	ds:mov ax,z[bp] 	;get average Z coordinates
	add ax,z[di]		;(z1+z2+z3)/3
	add ax,z[bx]
	sar ax,2
	mov za[si],ax
	pop cx
	add si,planesize
	dec cx 
	jnz l1
	
	mov cx,[new_planelist]
	dec cx
	xor ebx,ebx
	mov ebx,new_planelist+2  ;Initialize BSP tree
	call get_light
	call initbsp
	add ebx,planesize
	;
	mov ax,cx		;get number of parts
	xor esi,esi
	mov si,16
l0:	rcl ax,1
	jc >l1
	dec si
	jmp l0
l1:	
	;
	xor eax,eax		;get number of 10th entries
	mov ax,cx
	mov ecx,esi
	xor edx,edx
	div ecx
	push dx 		;# of planes<10:do rest
	or ax,ax		;number of parts?
	jz >n1			;none, do rest of all...
	;
	push esi,ax
	mov eax,planesize
	xor edx,edx
	mov ecx,esi
	mul ecx
	mov ebp,eax		;get size of 10th
	pop dx			;return # of planes/part
	mov cx,si
l0:	push dx,cx,ebx
	mov cx,dx
l1:	push ebp,cx
;	 call get_light
	call put2bsp
	pop cx,ebp
	add ebx,ebp		;jump to next part
	dec cx
	jnz l1
	pop ebx,cx,dx
	add ebx,planesize	;get new offset for the first part
	dec cx
	jnz l0
	;
	pop esi
	xor eax,eax		;get address of rest
	xor ecx,ecx
	mov ax,dx
	mov cx,planesize
	xor edx,edx
	mul ecx
	xor edx,edx
	mov ecx,esi
	mul ecx
	add eax,new_planelist+2+planesize
	mov ebx,eax
	jmp >j1
	;
n1:	add ebx,planesize
j1:	pop cx
	or cx,cx
	jz >n1
	sub ebx,planesize
l1:	push cx
	call put2bsp
	pop cx
	add ebx,planesize
	dec cx
	jnz l1
	;
n1:	call get_bsp		;read out the whole tree
	;
	mov si,sortlist 	;draw the fuckers
	mov cx,[new_planelist]
;	 cmp cx,planelimit
;	 if a mov cx,planelimit
l1:	push cx
	lodsw
	push si
	mov si,ax
	mov ax,pc[si]
	mov poly_color,al

	mov di,pt2[si]
	mov bx,pt3[si]
	mov si,pt1[si]
	push w[si],w[di],w[bx]
	push si,di,bx
	call fill_poly_plain
	pop bx,di,si
	pop w[bx],w[di],w[si]
	pop si,cx
	dec cx 
	jnz l1
	ret			;sorted...

get_light:
	pushad
	xor esi,esi
	xor edi,edi
	xor eax,eax
	mov si,pt1[bx]
	mov ax,pt2[bx]
	mov di,pt3[bx]
	mov ebx,eax
	call vLight
	popad
	ret

	include"3darith.inc"
	include"bsp.inc"		;fast Z-sort
	include"trifillr.inc"		;fills plain triangle
	include"retrace.inc"
	include"rotate.inc"		;rotates 3d coordinates
	include"sine.inc"		;simple sine table

