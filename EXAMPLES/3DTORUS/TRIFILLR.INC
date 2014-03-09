;(c)1997 S.Klose

;    \	  top			The polygon is splitted into halves as shown
;     \ \			in the picture. Each half is drawn from top
;      \   \			to bottom and from left to right.
;	\     \ 		DWORD writes are aligned.
;	 \_______\ middle
;	  \	 |
;	   \	 |
;	    \	 |
;	     \	 |
;	      \  |
;	       \ |
;		\| bottom
;
	.data
delta_left	dd ?	;x delta of left and right side
delta_right	dd ?

delta_start	dd ?	;temporary delta values for each side
delta_end	dd ?	; needed while filling a half

pointbuffer:	dd ?,?
poly_y		dd ?	;Counter for Y clipping
c_height	dd ?	;Height of half
c_sub		dd ?
poly_color	dd ?	;Color of the polygon
chelp		dd ?	;Needed to remind what's the left and right edge
			; when drawing the bottom half

left		dd ?
clip_bufferL:	dd 5 dup ?    ;Clipping buffer
right		dd ?
clip_bufferR:	dd 5 dup ?    ;Clipping buffer
flg_hide	db ?

	.code
; DS:SI/DI/BI=Position 1-3
; poly_color
fill_poly_plain:
	; First of all we sort the three points of the polygon on the
	; Y axis to make things easier.
	cld
	mov es,[scrbuffer]
	xor edx,edx
	mov ax,y[si]
	cmp ax,y[di]
	if g xchg si,di
	mov ax,y[di]
	cmp ax,y[bx]
	if g xchg di,bx
	mov ax,y[si]
	cmp ax,y[di]
	if g xchg si,di

	mov flg_hide,al
	; Now we have SI=top, DI=middle, BX=bottom

	cmp w y[si],199
	jg >e1		;Object is not visible
	push bx
			
	mov ax,x[si]	;initialize X-start (top point)
	shl eax,16
	mov left,eax
	mov right,eax
			;Get delta of the left edge (top to middle)
	; ---
	
	xor ebp,ebp	;delta=height/xdiff
	mov bp,y[di]
	sub bp,y[si]
	push bp
	if z inc bp	;In case the edge's height is 0, we assume 1
	mov ax,x[di]
	sub ax,x[si]
	shl eax,16
	cdq
	idiv ebp
	mov delta_left,eax    ;(fixed int 16:16)

	mov ax,x[bx]		;now get right delta
	xor ebp,ebp
	mov bp,y[bx]
	sub bp,y[si]
	mov c_height,bp 	;Height for X-clipping
	if z inc bp
	sub ax,x[si]
	shl eax,16
	cdq
	idiv ebp
	mov delta_right,eax
	mov ecx,eax

	; ---

	mov ebp,[si]	;initialize entry point of right edge
	mov pointbuffer,ebp
	
	mov dx,y[di]	;get height of half
	sub dx,y[si]
	mov ax,y[si]	;# of 1st screeh line
	push di
	mov di,pointbuffer
	;SI: left start DI: right start
	call clip_y	;get line entry points
	mov bp,c_sub
	sub c_height,bp
	pop di
	pop bp
	push ax,dx
	push bp

	push si,di,bx	;clip each side
	mov di,ofs right
	mov si,pointbuffer
	call clip_x
	pop bx,di,si
	pop ax
	;mov ax,y[di]
	;sub ax,y[si]
	sub ax,c_sub
	mov c_height,ax
	push si,di,bx
	mov ecx,delta_left
	mov bx,di
	mov di,ofs left
	call clip_x
	pop bx,di,si

	; ---

	pop dx,ax
	mov poly_y,ax		;Y clipping (bottom position)
	mov cx,ax
	add cx,dx
	cmp cx,199
	jle >n1
	sub cx,200
	sub dx,cx
n1:	add poly_y,dx

	mov cx,x[si]	;initialize X-start (top point)
	shl ecx,16
	mov left,ecx
	mov cx,x[pointbuffer]
	shl ecx,16
	mov right,ecx
	pop bx

	; ---

	push di,bx
	mov si,ofs left 	;exchange left and right side if needed to draw from
	mov di,ofs right	;left to right. Each structure contains the X position
	mov ecx,delta_left	;and the X delta for one side.
	cmp ecx,delta_right
	mov b chelp,0
	jle >n1
	xchg si,di
	mov b chelp,1
n1:	call half_poly_plain	 ;draw top half of polygon
	mov edx,ebx
	mov ax,di
	pop bx,di

	; ---

	cmp w poly_y,200	;Already at end of screen?
	je >e1			;Yes, break
	push si,ax,ebp,edx	;save pointers on sides and screen positions

	sub d delta_right,320 shl 16
	mov ax,x[bx]		;get delta of left edge (middle to bottom)
	push bx
	xor ebp,ebp
	mov bp,y[bx]
	sub bp,y[di]
	if z inc bp
	sub ax,x[di]
	push bp
	shl eax,16
	cdq
	idiv ebp
	mov ecx,eax
	mov [delta_left],eax

	; ---
	
	pop dx
	push eax
	mov ax,poly_y
	push si,di,bx
	mov si,di
	mov di,bx
	call clip_y
	pop bx,di,si
	add poly_y,dx
	cmp w poly_y,199
	jle >n1
	mov cx,poly_y
	sub cx,200
	sub dx,cx
n1:	
	pop ecx,bx
	sub dx,c_sub
	jz >e2
	jc >e2		;war...
	push dx 	;hinter diesem...
	mov c_height,dx
	mov si,di
	mov di,ofs left
	push ax
	call clip_x
	pop ax

	pop dx,ebx,ebp
	pop di,si
	cmp b chelp,0
	jnz >n3
	mov si,ofs left ;exchange left and right side if needed to draw from
			;left to right
	jmp >n2
n3:	mov di,ofs left
n2:	jmp half_poly_plain_D  ;draw bottom half of polygon
e2:	add sp,12
e1:	ret
b1:	add sp,4
	ret

; DX=Height of polygon part
; AX=starting line
; SI=left side
; DI=right side
half_poly_plain:
	mov bp,ax		;compute address of starting line
	shl ax,8
	shl bp,6
	add bp,ax

	;Now we have	ESI=left
	;		EDI=right
	;		BP=Destination addr on screen
	;Load screen position pointers for each side
	mov ebx,[si]	;start of 1st line
	mov eax,[di]	;end of 1st line
	shl ebp,16
	add ebx,ebp
	add eax,ebp
	mov ebp,eax
	add ebp,10000h	;Always draw 1 point at the start

	cmp w poly_y,0
	jng >e1 	 ;Section is not visible...but EBX and EBP is set
			;to the new positions...

half_poly_plain_d:
	;save the deltas for each side
	mov eax,cldt[si]
	mov delta_start,eax
	mov eax,cldt[di]
	mov delta_end,eax
	mov al,poly_color	;and draw the f*cker.
	mov ah,al
	push ax,ax
	pop eax

l2:	push dx
	mov dx,clh[si]		;get section from list and... 
	mov cx,clh[di]
	cmp dx,cx
	if a mov dx,cx
	pop cx
	push cx
	cmp dx,cx
	if a mov dx,cx
	
	or dx,dx
	jnz >j1
	mov eax,320 shl 16
	add ebx,delta_start	;update position for next line
	add ebp,delta_end
	sub ebx,eax
	sub ebp,eax
	pop ax
	ret

j1:
	sub clh[si],dx		;...update pointers
	if z add si,c_size
	sub clh[di],dx
	if z add di,c_size
	
	push dx,si,di
l1:	mov edi,ebx		;Load line start and its length
	mov ecx,ebp
	shr edi,16
	shr ecx,16
	;
	sub cx,di		;store color AL CX times
	jc >n3
	mov si,cx		;(aligns dword writes)
	mov cx,di
	neg cx
	and cx,3
	cmp cx,si
	if a mov cx,si
	sub si,cx
	rep stosb
	;
	mov cx,si
	and cl,11111100b
	sub si,cx
	shr cx,2
	rep stosd
	;
	mov cx,si
	rep stosb
	
n1:	add ebx,delta_start	;update position for next line
	add ebp,delta_end
	;
	dec dx
	js >x1
	jnz l1
x1:
	pop di,si,cx,dx
	sub dx,cx
	jz >e1
	push eax
	mov eax,cldt[si]	;get deltas of next section
	mov delta_start,eax
	mov eax,cldt[di]
	mov delta_end,eax
	cmp b cll[si],-1	;both sides invisible?
	jne >n1
	cmp b cll[di],-1
	jne >n1
	pop eax 		;yes, break
	ret

n3:	jmp n1

n1:	pop eax
	jmp l2

n2:	pop ax
e1:	ret

; Computes new line entry points, if they start above the first screen line.
; Special care is taken for invisible parts with height 0.
; In this occurs, the delta is added once and the section is skipped.
; In/Out: AX=starting line
;	  DX=height
clip_y: mov w c_sub,0
	or ax,ax	;Y clipping (top)
	jns >n1
	neg ax		;cut first lines
	mov c_sub,ax
skip_y: push bx
	xor ebx,ebx
	mov bx,ax
	sub dx,ax	;subtract clipped lines from wanted height
	push dx,ebx
	mov eax,[delta_left]
	cdq
	imul ebx
	shr eax,16
	add x[si],ax
	pop ebx
	mov eax,[delta_right]
	cdq
	imul ebx
	shr eax,16
	add x[di],ax
	pop dx
n2:	pop bx
	xor ax,ax
n1:	ret

; ECX=delta
; SI=top point
; BX=bottom point
; DI=clip_struc
clip_x: mov ax,x[si]
	cmp ax,319		;x>319?
	jg >t1			;no...
	or ax,ax		;x<0?
	js >t2
i1:	mov ax,x[bx]
	cmp ax,319
	jg >t3
	or ax,ax
	js >t4
	xor ax,ax
	jmp >o3
	ret

t1:	or ecx,ecx		;x>319
	jns >c1 		;Line is not visible, clear side
	sub ax,319		;get first visible Y
	mov w x[si],319 	;New X start
o1:	shl eax,16
	mov ebp,ecx
	cdq
	idiv ebp
	if z inc ax
o2:	mov clh[di],ax		;height of part
	mov d cldt[di],320 shl 16 ;delta for this part
	mov b cll[di],-1	;(not visible)
	add di,c_size		;pointer to next delta
	sub c_height,ax 	;Y left of line?
	jnz i1			;clip 2nd position of line
	ret			;line done, break

c1:	mov ax,c_height
	mov w [si],319		;New X start
	jmp o2

t2:	or ecx,ecx		;x<0
	js c1			;line isn't visible, clear side
	neg ax			;get first visible Y
	mov w [si],0		;New X start
	jmp o1

				;x>319
t3:	sub ax,319		;get height of invisible part
o1:	shl eax,16
	mov ebp,ecx
	cdq
	idiv ebp
	if z inc ax
o3:	mov bp,c_height
	sub bp,ax
	mov clh[di],bp		;height of part
	add ecx,320 shl 16
	mov cldt[di],ecx	;delta for this part
	mov b cll[di],0 	;(visible)
	add di,c_size		;pointer to next delta
	sub c_height,bp 	;Y left of line?
	jz >e1			;line done, break
	;store invisible part
	mov clh[di],ax
	mov d cldt[di],320 shl 16
	mov b cll[di],-1	 ;(invisible)
e1:	ret
				;x<0
t4:	neg ax			;get first visible Y
	jmp o1

