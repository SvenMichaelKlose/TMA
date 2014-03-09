; FLIplayer. (c)1995 S.Klose

	.data
datastart:
SourceHandle	dw ?
SourceName:	db 128 dup ?
seg_input	dw ?
LoadSize	dw ?
ChunksizeW	dw ?
;
FLIHeader:	dw ?,?,?
Frames: 	dw 61 dup ?
;
FrameHeader:	dw ?,?,?
Chunks: 	dw 8 dup ?
dataend:

	.code
chunksize=6

FLI_play_v1_0:
	mov dx,msg_standard
	mov ah,9
	int 21h
	mov di,datastart	;Datenbereich lîschen
	mov cx,dataend-datastart
	xor al,al
	rep stosb
	mov ax,cs
	add ax,1000h
	mov seg_input,ax
	mov si,82h	;Dateinamen suchen
	mov di,SourceName
l1:	lodsb
	cmp al,' '
	je l1
	cmp al,13	;Ende der Zeile ?
	jne >l1 	;Nein,weiter ...
	mov dx,ERR_NoName
	jmp error
;	error(err_noname)
l1:	dec si
l1:	lodsb
	stosb
	cmp al," "
        ja l1
	dec di
	xor al,al	;Ende des Namens kennzeichnen
	stosb
	cld
	 
	mov ax,13h	      ;Graphikmodus anschalten
	int 10h

m1:	mov ax,3d00h	      ;FLI-Datei îffnen
	mov dx,sourcename
	int 21h
	jnc >l1
	mov dx,ERR_SourceNotOpen
	jmp error
l1:	mov SourceHandle,ax
	mov bx,ax
	mov dx,FLIHeader      ;FLI-Header einlesen
	mov cx,128
	mov ah,3fh
	int 21h

GetFrame:
	mov ds,cs	      ;Frame-Header und Chunk-Header laden
	mov es,0a000h
	mov dx,FrameHeader
	mov cx,16+ChunkSize
	mov bx,cs:SourceHandle
	mov ah,3fh
	int 21h
	mov si,FrameHeader+16 ;SI auf Chunk zeigen lassen
GetChunk:
	lodsw		      ;LÑnge der Daten holen
	mov cx,ax	      ;Daten und nÑchsten Chunk laden
	mov ax,cs:Chunks
	cmp ax,1
	jne LoadChunk
	sub cx,Chunksize      ;(keine Chunks mehr da)
LoadChunk:
	add si,2	      ;LÑnge der Daten
	lodsb		      ;Typ holen
	push ax
	mov ds,cs:seg_input
	xor dx,dx
	mov bx,cs:sourceHandle
	mov ah,3fh
	int 21h
	mov cs:ChunksizeW,ax
	xor si,si	      ;Zeiger auf neue Daten
	pop ax
	call Retrace
	sub al,11
	cmp al,4
	ja fli_error
	mov bl,al
	xor bh,bh
	add bx,bx
	jmp fli_jmp[bx]

fli_jmp:dw get_colors,decode_lines,fill_screen,fli_error,rle_decode,get_bitmap

	align 16
ChunkLoop:
	dec w[cs:Chunks]	 ;Ende der Chunks ?
	jnz GetChunk	      ;Nein,weiter
	dec w[cs:Frames]	 ;Ende der Frames ?
	jne GetFrame	      ;Nein,nÑchstes.

	push cs
	pop ds
	call close_file  
	mov ah,1
	int 16h
	jz m1
	mov ax,3
	int 10h
	mov ax,4c00h
	int 21h

FLI_error:
	mov dx,ERR_FLIerror
Error:	push cs
	pop ds
	mov ah,9
	int 21h
	call close_file
	mov ax,4c01h
	int 21h

close_file:
	mov bx,SourceHandle
	mov ah,3eh	;Datei schliessen
	int 21h
	ret

	align 16
; Liest neue Farbpalette ein
get_colors:
	lodsw	      ;Anzahl der Pakete
	mov cx,ax
	xor bl,bl
g1:	push cx
	lodsb	      ;Nummer der 1. Farbe
	add bl,al
	lodsb	      ;Anzahl der Farben
	mov cl,al
	xor ch,ch
	or al,al      ;0=256
	jnz >g2
	inc ch
g2:	mov dx,3c8h   ;VGA-Reg. Farbe anwÑhlen
g3:	mov al,bl
	out dx,al
	inc dx
	lodsw
	out dx,al
	mov al,ah
	out dx,al
	lodsb
	out dx,al
	inc bl
	dec dx
	loop g3
	pop cx
	loop g1
	jmp ChunkLoop
  

	align 16
; Decodiert Zeilen im Delta-Packverfahren
decode_lines:
	lodsw		      ;1.Zeile
	mov ah,al	      ;Zeilenadresse berechnen
	xor al,al
	mov di,ax
	shr ax,2
	add di,ax
	lodsw		      ;Anzahl der Zeilen in CX
	mov cx,ax
DLlineLoop:
	push di,cx
	lodsb
	xor dh,dh
	mov dl,al
	or al,al
	jz >d2
	xor ah,ah
DLblockLoop:
	lodsb			;Neue Position im Bildschirm
	add di,ax
	lodsb
	test al,128		;Block oder String ?
	jz >c1
	not al
	inc ax
	mov cx,ax
	lodsb			;Farbe holen
	mov al,ah
	push ax
	shl eax,16
	pop ax
	shr cx,1
	jnc >l1
	stosb
l1:	jcxz >d1
	shr cx,1
	jnc >l1
	stosw
l1:	jcxz >d1
	rep stosd		;String schreiben
	jmp >d1
	;
c1:	mov cx,ax		;Ausschnitt kopieren
	shr cx,1
	jnc >l1
	movsb
l1:	jcxz >d1
	shr cx,1
	jnc >l1
	movsw
l1:	jcxz >d1
	rep movsd

d1:	dec dx
	jnz DLblockLoop
d2:	pop cx,di
	add di,320		;Zeiger auf nÑchste Zeile
	loop DLlineLoop
	jmp ChunkLoop

	align 16
; RLE-Dekodierung von neuen Zeilen
RLE_decode:
	xor bp,bp     ;Anfangsadresse des Bildschirms
	xor bx,bx     ;Erste Zeile = 0
RLEloop1:
	mov di,bp     ;Adresszeiger retten
	lodsb
	or al,al
	jz RLEloop1end;Ende wenn Wert null ist
	xor dh,dh
	mov dl,al
	xor ah,ah
RLEloop2:
	lodsb
	test al,128
	jz RLEfill
	not al
	inc ax
	mov cx,ax
	shr cx,1
	jnc >l1
	movsb
l1:	jcxz >d1
	shr cx,1
	jnc >l1
	movsw
l1:	jcxz >d1
	rep movsd	       ;String schreiben
	jmp >d1
RLEfill:
	mov cx,ax
	lodsb
	shr cx,1
	jnc >l1
	stosb
l1:	jcxz >d1
	shr cx,1
	jnc >l1
	stosw
l1:	jcxz >d1
	rep stosd		;String schreiben
d1:	dec dx
	jz RLEloop2
RLEloop1end:
	add bp,320
	inc bx
	cmp bx,200
	jne RLEloop1
	push si
	sub si,w[cs:chunksizeW]
	jnc NoRest
	mov dx,si
	neg dx
	mov cx,-1
	mov ax,4201h
	int 21h
	mov bx,w[cs:SourceHandle]
	mov cx,Chunksize
	xor dx,dx
	mov ah,3fh
	mov ds,w[cs:seg_input]
	;int 21h
	;xor si,si
NoRest:
	pop si
	mov ax,si
	and al,1
	jz Nr2
	inc si
Nr2:
	jmp ChunkLoop

; LÑdt Bitmap direkt
get_bitmap:
	xor di,di	     ;Bild von Puffer in den Bildschirmspeicher
	mov cx,16000
	rep movsd	     ;kopieren.
	jmp ChunkLoop

; FÅllt Bitmap
fill_screen:
	xor di,di
	mov cx,16000
	xor eax,eax
	rep stosd
	jmp ChunkLoop

	include"..\lib\retrace.inc"

msg_standard:
	db"FLIPLAY (c)1995 S.Klose (sklose@post.uni-bielefeld.de)",10,13,"$"
ERR_noName:
	db"Type FLIPLAY <filename.FLI>",10,13,"$"
ERR_SourceNotOpen:
	db"Can't open file.",0ah,0dh,"$"
ERR_FLIerror:
	db"Wrong chunk type.",10,13,"$"
	

