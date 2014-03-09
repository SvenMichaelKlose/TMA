; =====================
; >>> Flammeneffekt <<<
; =====================

	.model small
dataseg group _text,data

	.code
        mov ds,data
	mov w[flameintensity],160
	mov di,newcolors
	mov cx,256
l1:	mov al,cl
	dec al
	xor al,-1
	shr al,2
	stosb
	xor ax,ax
	stosw
	loop l1
InitFlames:
	mov ax,13h	;VGA-Screen ein
	int 10h

	mov di,FlameBuffer
	mov cx,_psize
	mov si,prolog
	rep movsb
	mov cx,160
InFl2:	push cx
	mov si,FlamePart
	mov cx,FlamePrtEnd+1
	sub cx,FlamePart
	rep movsb
	pop cx
	loop InFl2
	mov cx,_esize
	mov si,epilog
	rep movsb
	;
	mov ax,ss
	add ax,1000h
	mov flamesegA,ax
	mov es,ax
	add ax,1000h
	mov flamesegB,ax
	xor ax,ax
	mov cx,8000h
	rep stosw
	mov w[Fades],768
	call getcolors
	call FullFade
FlameLoop:
	in al,60h
	cmp al,1
	je Ende
	call DoFlame
	jmp FlameLoop
Ende:	call ClearNewColors
	call FullFade
	mov ax,3
	int 10h
	mov ax,4c00h
	int 21h

dotline=64000-323
DoFlame:mov ds,data
	mov es,flamesegA
	mov cx,w[FlameIntensity]
	mov ds,40h
	mov si,w[6ch]

DoFl2:	lodsb			;Zuf„llig Punkte setzen
	mov bl,al
	xor al,b[6ch]
	xor bh,bh
	xor ah,ah
	shr ax,2
	add bx,ax
	add bx,dotline
	xor al,al
	mov b[es:bx+3],al
	mov ax,0fefeh
	mov w[es:bx-1],ax
	mov w[es:bx+1],ax
	add si,bx
	add si,cx
	loop DoFl2
	
	std
	mov di,320 ;99*320-1	     ;Neue Seite berechnen
	mov cx,400
	mov ds,data
	mov es,w[FlameSegB]
	mov ds,w[FlameSegA]
	xor ah,ah
	xor bh,bh
	mov bp,320
DoFl3:	call ds:FlameBuffer
	loop DoFl3
	cld
	mov ds,es		  ;Flammen darstellen
	mov es,0a000h
	mov si,640*4
	xor di,di
	mov cx,16000-640
	rep movsd
	mov ds,data
	push flamesegA,flamesegB
	pop flamesegA,flamesegB
	ret

	include"..\lib\fade.inc"
	include"..\lib\retrace.inc"

_text	segment

prolog: mov cs:oldss,ss
	mov dx,ds
	mov ss,dx
l1:
_psize=l1-prolog

epilog: mov ds,data
	mov ss,oldss
	ret
l1:
_esize=l1-epilog

FlamePart:
	mov bl,[bp+di-1]
	mov al,[bp+di]
	add ax,bx
	mov bl,[bp+di+1]
	add ax,bx
	mov bl,b[di]
	add ax,bx
	shr ax,2
FlamePrtEnd:
	stosb

	.data
oldss	dw ?
FlameIntensity: dw ?
FlameSegA	dw ?
FlameSegB	dw ?

	align 20h	;Cache page alignment fr mehr Saft
FlameBuffer:	db (flameprtend-flamepart+_psize+_esize) dup ?

