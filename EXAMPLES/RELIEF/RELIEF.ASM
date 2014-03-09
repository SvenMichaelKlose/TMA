; Bump mapper - (c)1997 S.Klose
;
; Assemble with TMA macro assembler:
; TMA -e RELIEF.ASM
	.model small
	.stack 1k
dataseg group _text,data

	.data
xrange=319
yrange=199
waverange=31
msk_noheight=127
msk_height=128

movestruc struc [si]
	mx	dw ?	;actual location
	my	dw ?
	mdx	dw ?	;destination
	mdy	dw ?
	mxdi	dd ?	;Direction
	mydi	dd ?
	mspeed	dw ?	;speed of location
	msize:
ends

datastart:
	align 4
seg_light	dw ?
seg_bumpmap	dw ?
seg_screen	dw ?
seg_waves	dw ?
seg_colors	dw ?
seg_scrbuffer	dw ?
waveoffset	dw ?
seg_finalmap	dw ?
wavespeed	db ?

circlecol	db ?
next_buffer	dw ?
drawn_buffers	dw ?
r_xstart	dw ?
r_ystart	dw ?
r_xsize 	dw ?
r_ysize 	dw ?
r_xofs		dw ?
r_difl		dw ?
r_difr		dw ?
r_yjump 	dw ?
savedss 	dw ?
cursorpos	dw ?
charsetadr	dw ?
charsetseg	dw ?
printcolorb	db ?

lp:		db msize dup ?
ylookup:	dw 256 dup ?
yolookup:	dw 256 dup ?
dataend:

	.code

start:	mov ax,_text ;data
	mov ds,ax
	mov es,ax
	mov ax,_text
	mov di,datastart
	mov cx,dataend-datastart
	xor al,al
	rep stosb
	;
	mov ax,1130h
	mov bh,3
	int 10h
	mov w[CharsetAdr],bp
	mov w[CharsetSeg],es
	;
	mov ax,ss
	add ax,400h
	mov seg_light,ax
	add ax,1000h
	mov seg_screen,ax	;Memory for triple buffering
	add ax,1000h
	mov seg_waves,ax
	add ax,1000h
	mov seg_finalmap,ax
	add ax,1000h
	mov seg_colors,ax
	add ax,1000h
	mov seg_scrbuffer,ax
	add ax,1000h
	mov seg_bumpmap,ax	;Memory for surface data
	mov es,ax
	xor di,di
	mov cx,32k
	xor ax,ax
	rep stosw
	mov dx,nme_bumps
	call loadbmpimage

	push ds,es
	xor di,di
	mov ds,_text ;data
	mov es,seg_colors
	mov ds,seg_bumpmap
	mov cx,64000
l1:	xor ah,ah
	cmp b[di],0
	if e mov ah,64
	mov [es:di],ah
	inc di
	loop l1

	mov ds,_text
	mov w cursorpos,13 by 13
	mov b printcolorb,128
	mov si,txt_this
	call printtext
	mov es,seg_bumpmap
	mov w cursorpos,13 by 13
	mov b printcolorb,6
	mov si,txt_this
	call printtext
	
	pop es,ds

	call create_ylookup

	xor si,si		;raise surface contrast a bit
	xor di,di
	mov cx,64000
l1:	es:lodsb
	shl al,2
	stosb
	loop l1

	xor ax,ax
	mov next_buffer,ax	;segmented pointer to first buffer
	mov drawn_buffers,al	;No drawn buffers available now
	mov w fades,768 	;Fade all colors
	mov b fadespeed,1	;  (very slow)
	call clearnewcolors	; Fade colors to black
	call getcolors		; read in actual colors
	call fullfade		; fade...
	mov ax,13h		;set mode 13h, 320*200*256.
	int 10h
	call blankscreen	;Sets all colors to black
	call create_light	;Draw bitmap of light source
	call create_light_colors ;Create color table
	call time_retrace	;Time retrace with interrupt
	call cr_waves		;Create wave map
	mov ds,_text ;data
	mov b wavespeed,1

	;Main loop
jmp >f1
	xor ax,ax
	mov si,lp
	mov mx,ax
	mov my,ax
	mov ax,100h
	mov mdx,ax
	mov mdy,ax
	mov mxdi,ax
	mov ax,-1
	mov mydi,ax
;l1:	call move_location
	mov ax,mx
	mov r_xstart,ax
	mov ax,my
	mov r_ystart,ax
	call dr_bumpmap
	call screen_out
	mov ah,1
	int 16h
	jz l1
	jmp >o1

f1:	mov cx,150
l1:	push cx
	mov ds,_text ;data
	mov ax,cx
	add cx,20h
	mov r_xstart,cx
	sub ax,60
	mov r_ystart,ax
	call fade
	call dr_waves
	pop cx
	loop l1

	mov cx,130
l1:	sub w r_xstart,2
	call dr_waves
	loop l1
	
m1:	mov cx,320-60
l1:	add w r_xstart,2
	mov ah,1
	int 16h
	jnz >o1
	call dr_waves
	loop l1

	mov cx,320-60
l1:	sub w r_xstart,2
	mov ah,1
	int 16h
	jnz >o1
;	 call smear_bump
	call dr_waves
	loop l1
	jmp m1
	;
o1:	xor ax,ax		;read key from buffer; if we don't we'll have
	int 16h 		;it in the command line.
	mov ax,3		;Textmodus 80*25 einschalten.
	int 10h
	mov ax,4c00h		;exit.
	int 21h

; Create reference map for waves
cr_waves:
	push ds,es,fs
	pusha
	mov ds,_text ;data
	mov es,seg_waves
	mov w posy,100
	mov w posx,160
	mov cx,400
l1:	push cx
	mov ax,400
	sub ax,cx
	mov [radius],ax
	shr ax,2
	and al,waverange
	mov [circlecol],al
	call DrawCircleB
	dec w[radius]
	call drawcircleB
	pop cx
	loop l1
	popa
	pop fs,es,ds
	ret

; Draw waves
dr_waves:
	push ds,es,fs
	pusha
	; create new wave frame
	mov ds,_text ;data
	mov dl,waveoffset
	mov dh,waverange
	mov gs,_text
	mov es,[seg_finalmap] ;mixed map
	mov fs,[seg_waves]   ;wave offsets
	mov ds,[seg_bumpmap] ;image
	mov cx,32000
	xor di,di
	xor bh,bh
l3:	mov bl,[fs:di]		;wavetable index
	add bl,dl
	and bl,dh		;clip table
	mov al,gs:wavetable[bx]
	add al,[di]		;add existing map
	mov bl,[fs:di]		;wavetable index
	add bl,dl
	and bl,dh		;clip table
	mov ah,gs:wavetable[bx]
	add ah,[di+1]		;add existing map
	stosw
	loop l3
	;
	mov ds,_text ;data
	mov es,_text ;data
	push seg_bumpmap,seg_finalmap
	push seg_bumpmap,seg_finalmap
	pop seg_bumpmap,seg_finalmap
	push gs
	call dr_bumpmap
	pop gs
	mov ds,gs:seg_bumpmap
	mov es,gs:seg_scrbuffer
	mov fs,gs:seg_screen
;	 call dr_fsmap

	call retrace
	call screen_out
	mov ds,_text ;data
	pop seg_finalmap,seg_bumpmap
	mov al,wavespeed
	sub b[waveoffset],al
	and b[waveoffset],waverange
	popa
	pop fs,es,ds
	ret

smear_bump:
	pusha
	mov ds,_text ;data
	mov ds,seg_bumpmap
	mov es,ds
	xor si,si
	xor di,di
	mov cx,64000
l1:	lodsb
	or al,al
	if nz dec al
	stosb
	loop l1
	mov ds,_text ;data
	popa
	ret
	
; >>> Draws bump map
;
; This part draws the light and its reflections. If you don't know how
; it works, try to go through it or read the bumpmap tutorial.
;
; Variables:
; x_start/y_start=Upper left position of drawing area which has always
;		  the size of 256*256 pixels. Left screen will be cleared.
	align 4
dr_bumpmap:
	pusha
	mov ds,_text
	mov es,seg_screen	;the screen buffer we use.
	mov fs,seg_light	;segment of the light bitmap
	mov gs,seg_colors
	xor si,si		;bitmap origin (light)
	xor bp,bp		;bumpmap origin
	xor di,di		;start of screen buffer
	mov r_yjump,di		;this variable contains the number of lines
				; we have to clear first

	; Clipping of lines
	mov dx,256		;Height of bitmap
	mov ax,r_ystart 	;get screen line to start at
	or ax,ax		;negative?
	je >l1			;no, zero, we've got the first line...
	js >l2			;yes, start above first line of bitmap

	mov cx,ax		;below first line, draw gap
	shl ax,6		;ax contains the number of lines which we
	shl cx,4		; multiply by 80
	add cx,ax
	mov ax,cx		;new start within the bumpmap
	shl ax,2
	mov bp,ax
	xor eax,eax
	rep stosd		;writes lines*80 dwords of zeroes
	jmp >l1

l2:	neg ax			;above first line, get top lines to clip
	sub dx,ax		;correct height.
	mov r_yjump,ax
	shl ax,8
	mov si,ax		;jump in bitmap
l1:	mov ax,r_ystart
	add ax,56		;draw everything ?
	or ax,ax
	js >l1
	sub dx,ax		;no, clip height of bitmap
	if c xor dx,dx
l1:	mov r_ysize,dx		;save height of bitmap

	; Clipping of columns
	mov dx,256		;get columns to start at
	xor ax,ax
	mov r_xofs,ax
	mov r_difl,ax
	mov ax,r_xstart
	or ax,ax
	je >l1
	js >l2

	mov r_difl,ax		;distance to the screen border
	jmp >l1

l2:	neg ax			;beyond screen border, jump in bumpmap
	mov r_xofs,ax
	sub dx,ax

l1:	mov ax,r_xstart 	;clip line on left border
	sub ax,320-256
	or ax,ax
	if ns sub dx,ax
	mov r_xsize,dx

	mov ax,320		;distance to right border
	sub ax,dx
	sub ax,r_difl
	mov r_difr,ax

	mov cx,r_ysize		;Y-loop
	or cx,cx
	jz >n2
m1:	push cx 		;line counter
	push si 		;Position in Bumpmap
	push bp 		;Position in Bitmap
	mov cx,r_difl		;fill left gap
	jcxz >n1
	;
	xor eax,eax		;This part tries to perform a DWORD
	shr cx,1		; sized STOS. Alignment is not implemented yet,
	if c stosb		; but it'd speed up things a bit.
	jcxz >n1
	shr cx,1
	if c stosw
	jcxz >n1
	rep stosd
	;
n1:	mov cx,r_xsize		;length of the line we are going to draw
	add si,r_xofs		;Start in Bitmap
	add bp,r_xstart 	;Start in Bumpmap
	add bp,r_xofs
	or cx,cx
	jz >n3
	cli
	mov savedss,ss
	mov ss,seg_bumpmap 
	;
	; >>> inner loop
	dec si
l1:	mov bl,[bp]	       ;get height of actual position
	mov bh,bl
	sub bl,[bp+1]		;get X slope
	sub bh,[bp+319] 	;get Y slope
	inc si			;we have BL=X
	mov al,[fs:bx+si]	;grab the color out of the bitmap
	inc bp
	or al,[gs:di]
	stosb
	dec cx
	jnz l1

	mov ax,_text ;data
	mov ds,ax
	mov ss,savedss
	sti
n3:	mov cx,r_difr		;fill right gap
	jcxz >n1
	;
	xor eax,eax		;fast stosd
	shr cx,1
	if c stosb
	jcxz >n1
	shr cx,1
	if c stosw
	jcxz >n1
	rep stosd
	;
n1:	pop bp,si,cx
	add si,256		;move pointers to next line
	add bp,320
	dec cx
	jnz m1

n2:	mov cx,200		;fill bottom of screen if neccesary
	sub cx,r_ysize
	or cx,cx
	jz >n1
	js >n1
	mov ax,cx
	shl ax,6
	shl cx,4
	add cx,ax
	xor eax,eax
	rep stosd
n1:	popa
	ret			;sorted

; ds=map
; fs=source
; es=dest
dr_fsmap:
	cli
	mov ax,ds
	mov gs,_text ;data
	mov savedss,ss
	mov ss,ax

	mov cx,64000
	xor bp,bp
	xor di,di
l1:	mov bl,[bp]	       ;get height of actual position
	mov al,bl
	sub bl,[bp+319]
	xor bh,bh
	add bx,bx
	gs:mov bx,yolookup[bx]
	sub al,[bp+1]		;get X slope
	cbw
	add bx,ax
	mov al,[fs:bx+di]	;grab the color out of the bitmap
	inc bp
	stosb
	dec cx
	jnz l1
	mov ax,_text ;data
	mov ds,ax
	mov ss,savedss
	sti
	ret

screen_out:
	push ds,es
	xor si,si
	xor di,di
	mov es,0a000h
	mov ds,_text ;data
	mov ds,seg_screen
	mov cx,16000
	rep movsd
	pop es,ds
	ret

; >>> Draws bitmap for reflections...
;
; ...which is done by drawing circles with different colors.
create_light:
	mov es,seg_light
	xor di,di
	mov cx,256*256/4
	xor eax,eax
	rep stosd		;(clear segment first)

	mov cx,256
	mov ax,128
	mov posx,ax
	mov posy,ax
l1:	push cx
	mov [radius],cx
	shr cx,2
	mov al,64
	sub al,cl
	mov [circlecol],al
	call drawcircle
	dec w[radius]
	call drawcircle
	pop cx
	loop l1
	mov eax,3f3f3f3fh
	mov [es:127*256+126],eax
	mov [es:128*256+126],eax
	mov [es:129*256+126],eax
	ret

; >>> Create the right colors for the light
;
; Well, this is quite easy...
create_light_colors:
	mov ax,_text ;data
	mov ds,ax
	mov es,ax
	mov di,newcolors
	mov cx,64
l1:	push cx
	xor ax,ax
	cmp cl,24
	jae >l2
	mov al,23
	sub al,cl
	shl al,1
	mov ah,al
l2:	stosw
	mov al,64
	sub al,cl
	stosb
	pop cx
	loop l1

	mov cx,64
l1:	push cx
	mov al,64
	sub al,cl
	stosb
	xor ax,ax
	cmp cl,24
	jae >l2
	mov al,23
	sub al,cl
	shl al,1
	mov ah,al
l2:	stosw
	pop cx
	loop l1

	mov cx,64
l1:	push cx
	mov al,64
	sub al,cl
	stosb
	stosb
	xor ax,ax
	cmp cl,24
	jae >l2
	mov al,23
	sub al,cl
	shl al,1
l2:	stosb
	pop cx
	loop l1
	ret

time_retrace:
	ret

; ***********************************************************
; *** Draws circle using Bresenham's (nice lad) algorithm ***
; ***********************************************************
; ES=Zielsegment (256*256) Punkte
; RADIUS is klar
; POSX & POY = Zentrum
	.code
DrawCircleB:
; ----- Initialisierung
	mov ds,_text ;data
	mov ax,[radius] 	;y=radius
	mov di,ax		;y
	xor si,si		;x
	mov bp,3		;d=3-2*radius
	add ax,ax
	sub bp,ax
; ----- Hauptteil
d3:	call dcpointB
	mov ax,si
	cmp ax,di
	jge >d100
	test bp,32k
	jz >d6
	mov ax,si		;d=4x+6
	shl ax,2
	add ax,6
	add bp,ax
	inc si
	jmp d3
d6:	mov ax,si
	sub ax,di
	shl ax,2
	add ax,10
	add bp,ax
	dec di
	inc si
	jmp d3

; ----- Punkte in den Quadranten zeichnen
d100:	mov ax,si
	cmp ax,di
	jne >d200
	call dcpointB
d200:	ret

dcpointB:
	mov bx,posx
	mov cx,si
	shr cx,1
	mov dx,di
	shr dx,1
	mov ax,bx
	add ax,cx
	mov xxcp,ax
	mov ax,bx
	sub ax,cx
	mov xxcm,ax
	mov ax,bx
	add ax,dx
	mov xycp,ax
	mov ax,bx
	sub ax,dx
	mov xycm,ax
	mov bx,posy
	mov ax,bx
	add ax,cx
	mov yxcp,ax
	mov ax,bx
	sub ax,cx
	mov yxcm,ax
	mov ax,bx
	add ax,dx
	mov yycp,ax
	mov ax,bx
	sub ax,dx
	mov yycm,ax
	mov al,[CircleCol]

	push di
	mov bx,xxcp
	cmp bx,xrange
	ja >d400
	push bx
	mov di,yycp
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al

d300:	pop bx
	mov di,yycm
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al
d300:
d400:	mov bx,xxcm
	cmp bx,xrange
	ja >d400
	push bx    
	mov di,yycp
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al

d300:	pop bx
	mov di,yycm
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al
d300:
d400:	mov bx,xycp
	cmp bx,xrange
	ja >d400
	push bx
	mov di,yxcp
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al

d300:	pop bx
	mov di,yxcm
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al
d300:
d400:	mov bx,xycm
	cmp bx,xrange
	ja >d400
	push bx
	mov di,yxcp
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al

d300:	pop bx
	mov di,yxcm
	cmp di,yrange
	ja >d300
	add di,di
	add bx,ylookup[di]
	mov b[es:bx],al
d300:
d400:	pop di
	ret

; Create table of screen offsets
create_ylookup:
	pusha
	push es
	mov es,_text ;data
	mov di,ylookup
	xor ax,ax
	mov cx,200
l1:	stosw
	add ax,320
	loop l1
	mov cx,56
	reps stosw

	mov di,yolookup
	xor ax,ax
	mov cx,128
l1:	stosw
	add ax,320
	loop l1
	mov cx,128
	mov ax,-(128*320)
l1:	stosw
	add ax,320
	loop l1
	pop es
	popa
	ret

PrintText:
	push ds
	mov fs,_text
PrintPlane3:
	fs:lodsb
	or al,al
	jz PrintEnd
	cmp al,1
	jne PPnoNewLine
	mov b[CursorPos],0
	inc b[CursorPos+1]
	jmp PrintPlane3
PPnoNewLine:
	call PrintChar
	jmp PrintPlane3
printEnd:
	pop ds
	ret

PrintChar:
	pusha
	push ds,es
	push ax
	mov ax,_text ;data
	mov ds,ax
	mov gs,ax
	mov ax,w[CharSetSeg]
	mov ds,ax
PrintPlane2:
	mov al,b[gs:CursorPos+1]
	xor ah,ah
	shl ax,9
	mov di,ax
	shl ax,2
	add di,ax
	mov al,b[gs:CursorPos]
	xor ah,ah
	shl ax,3
	add di,ax
	pop ax
	xor ah,ah
	shl ax,3
	add ax,w[gs:CharSetAdr]
	mov bp,ax
	mov bx,w[gs:PrintColorB]
	mov cx,8		;we'll have to draw 8 lines
pp2:	push cx
	mov cx,8		; with 8 pixels each
	mov al,[ds:bp]		;Get line from charset
pp3:	mov dl,bl
	shl al,1		;Bit set?
	jc Back 		;No, use background color.
	mov dl,bh		;Yes, foreground...
back:	mov b[es:di],dl
	inc di			;next pixel
	loop pp3
	inc bp			;next line
	add di,320-8
	pop cx
	loop pp2
	inc b[gs:CursorPos]
	add di,8-(8*320)
	pop es,ds
	popa
	ret

	include"..\lib\retrace.inc"
	include"..\lib\fade.inc"
	include"..\lib\loadbmp.inc"
	include"..\lib\brescirc.inc"

	.code

_text	segment

nme_bumps:db"5byte.bmp",0
txt_this: db" software ",0
wavetable:
db 32,38,44,49,54,58,61,63,63,63,61,58,54,49,44 
db 38,31,25,19,14,9,5,2,0,0,0,2,5,9,14,19,25 

