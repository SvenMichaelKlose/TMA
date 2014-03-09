; ******************
; * LÑdt BMP-Datei *
; ******************
; CS:DX=Name der Datei
; ES=Zielsegment
	.code
FileError2:
  ret

LoadBMPimage:
  push es,ds
  mov ds,_text
  mov ax,3d00h	    ;Datei îffnen
  int 21h
  jc FileError2
  mov bx,ax
  push bx
  
  mov ah,3fh	  ;BMP-Header laden
  mov cx,436h
  mov dx,BMPheader
  int 21h
  
  mov si,36h+BMPheader
  mov di,newcolors
  xor bx,bx	  ;Ab Farbe 0
  mov cx,256	  ;256 Farben setzen
  push es
  mov es,data
LoBMPcol2:
  lodsw
  shr al,2
  shr ah,2
  push ax
  lodsb
  shr al,2
  stosb;Rot-Wert setzen
  pop ax
  xchg al,ah
  stosb;GrÅn-Wert setzen
  xchg al,ah
  stosb;Blau-Wert setzen
  inc si
  loop LoBMPcol2
  pop es

LoadBMPnb:
  pop bx
  push bx
  push es	  ;Zielsegment Åbergeben
  pop ds
LoadBMPloop:
  mov ah,3fh	  ;Graphikdaten holen
  mov cx,8000h
  xor dx,dx
  push ds
  int 21h
  pop ds
  or ax,ax	  ;Nix geladen,Ende
  jz LoadBMPend
  shr ax,4
  mov dx,ax
  push ds
  pop ax
  add ax,dx
  push ax
  pop ds
  jmp LoadBMPloop
LoadBMPend:
  pop bx
  mov ah,3eh	  ;Datei schliessen
  int 21h
  pop ds,es
  ret

	.data
BMPheight:	dw 0
BMPwidth:	dw 0
bmpheader:	db 436h dup ?
