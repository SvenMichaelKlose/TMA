; ****************************************
; *** Erkennt verschiedene SVGA-Karten ***
; *** und schaltet Videomodi ein.      ***
; ****************************************

GetSVGA:
  mov ds,0c000h    ;Nach Graphik-BIOS suchen
GetSVGA1:
  mov ax,w[0]
  cmp ax,0aa55h   ;VGA BIOS-Kennung
  je GetSVGA2
  mov ax,ds
  add ax,1000h
  mov ds,ax
  or ax,ax
  jnz GetSVGA1
  
  mov ds,cs
  mov dx,TXT_noBIOS
  mov ah,9
  int 21h
  ret
GetSVGA2:
  mov es,ds
  mov si,SVGAchips
  mov cx,19
GetSVGA3:
  mov bp,si     ;Adresse des Namens zwischenspeichern
  mov ds,cs
  xor di,di
  lodsb         ;1. Zeichen holen
  xchg es,ds
  xchg si,di
  mov bl,al
GetSVGA4:
  lodsb
  cmp al,bl     ;Zeichen gleich ?
  je GetSVGA6   ;Ja
  or si,si
  jnz GetSVGA4
  xchg ds,es
  xchg si,di
GetSVGA5:
  lodsb         ;nÑchsten Chipnamen suchen
  or al,al
  jnz GetSVGA5
  add si,4      ;Videomode Åberspringen
  loop GetSVGA3
  mov ds,cs
  mov dx,TXT_noSVGA
  mov ah,9
  int 21h
  ret
GetSVGA6:
  push si,di,ax
  xchg ds,es
  xchg si,di
GetSVGA7:
  lodsb
  or al,al
  jz FoundSVGA
  mov ah,b[es:di]
  inc di
  cmp al,ah
  je GetSVGA7
  xchg ds,es
  pop ax,di,si
  jmp GetSVGA4
FoundSVGA:
  push si
  mov al,"$"
  mov b[si-1],al
  pop si
  mov dx,bp             ;Namen des Chipsets ausgeben
  mov ah,9
  int 21h
  mov dx,TXT_svgafound  ;Message ausgeben
  mov ah,9
  int 21h
  lodsw
  mov bx,ax
  lodsw
  mov w[SVGAmodeA],bx
  mov w[SVGAmodeB],ax
  pop ax,di,si
  ret

SVGAmodeA:dw 0
SVGAmodeB:dw 0

TXT_noBIOS:
  db"Sorry, can't find any VGA-BIOS ...",0ah,0dh,"$"
TXT_noSVGA:
  db"Time to upgrade: No SVGA found.",0ah,0dh,"$"
TXT_SVGAfound:
  db" chipset found and used.",10,13,"$"
SVGAchips:
  db"Ahead A",0
  dw 60h,0
  db"Ahead B",0
  dw 60h,0
  db"ATI Technologies",0
  dw 61h,0
  db"Cirrus",0
  dw 78h,0
  db"Chips",0
  dw 78h,0
  db"Genoa",0
  dw 2eh,0
  db"Headland",0
  dw 6f05h,66h
  db"OAK VGA BIOS",0
  dw 53h,0
  db"SpeedSTAR 24",0
  dw 0,0
  db"SpeedSTAR",0
  dw 0,0
  db"Stealth VRAM",0
  db 0,0
  db"Paradise",0
  dw 5eh,0
  db"Trident",0
  dw 5ch,0
  db"TRIDENT",0
  dw 5ch,0
  db"Tseng ET 4000",0
  dw 78h,0
  db"Video 7",0
  dw 6f05h,66h
  db"VESA",0
  dw -1,100h
  db"NCR",0
  dw 5eh,0
  db"S3",0
  dw 4f02h,201h

ERR_noSVGAdvr:
  db"No Super VGA drivers found.",10,13,"$"