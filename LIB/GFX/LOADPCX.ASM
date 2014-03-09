; *************************************
; *** LÑdt PCX-Bild und Farbpalette ***
; *************************************
; DS = freies Segment zum îlen, kann nach dem Laden
; wieder belegt werden...
; ES = Zielsegment
; DS:DX=Name der Datei  Farben kommen nach NewColours
LoadPCXimage:
  mov w[cs:LoadSeg],ds
  push es,ds,cs
  pop ds
  mov ax,3d00h      ;Datei îffnen
  int 21h
  ;jc FileError
  mov bx,ax
  push bx
  
  mov ah,3fh      ;PCX-Header laden
  mov cx,128
  mov dx,PCXheader
  int 21h

  mov ax,w[PCXheader+8] ;Breite der Graphik holen
  sub ax,w[PCXheader+5]
  inc ax
  mov w[PCXwidth],ax
  
  mov ax,w[PCXheader+10] ;Hoehe der Graphik holen
  sub ax,w[PCXheader+6]
  inc ax
  mov w[PCXheight],ax

  xor di,di
LoadPCXnb:
  mov ah,3fh      ;Graphikdaten holen
  mov cx,-1
  push w[cs:LoadSeg]  ;Zielsegment Åbergeben
  pop ds
  xor dx,dx
  int 21h
  inc ax
  mov w[cs:LoadSize],ax
  xor si,si
  xor di,di
  mov cx,w[cs:PCXheight]
PCXhloop:
  push cx
  mov bx,w[cs:PCXwidth] ;BX ist BreitenzÑhler
PCXwloop:
  lodsb           ;Farbkette ?
  test al,0c0h
  je PCXnoString       ;Nein,weiter ...
  sub al,0c0h
  mov cl,al
  xor ch,ch
  sub bx,cx
  lodsb
  rep stosb       ;Farbkette schreiben
  jmp PCXwloop2   ;Weiter ...
PCXnoString:
  stosb           ;Farbwert schreiben
  dec bx
PCXwloop2:
  cmp bx,0        ;Ende der Zeile ?
  ja PCXwLoop     ;Nein,weiter
  pop cx          ;NÑchste Zeile ...        
  loop PCXhLoop

  ;Farbtabelle ab DS:SI
PCX3:
  lodsb           ;12 ueberspringen
  cmp al,0ch
  jne PCX3
  
  mov dx,3c8h   ;ab farbe 0 wÑhlen...
  xor al,al
  out dx,al
  inc dl
  mov cx,256      ;256 Farben setzen
  mov es,cs
LoPCXcol2:
  push cx
  mov cx,3
lopfusch:
  lodsb
  shr al,2
  out dx,al
  loop lopfusch
  pop cx
  loop LoPCXcol2
  
  pop bx
  mov ah,3eh      ;Datei schliessen
  int 21h
  pop ds,es
  ret

LoadSeg:dw 0
LoadSize:dw 0
PCXheight:dw 0
PCXwidth:dw 0
PCXheader:db 128 dup ?

