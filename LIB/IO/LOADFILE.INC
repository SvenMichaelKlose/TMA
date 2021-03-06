; *******************************************
; *** L�dt Datei komplett in den Speicher ***
; *******************************************
; max. Gr��e ist 32k
; Parameter:
; DS:DX=Zeiger auf Namen
; R�ckgaben:
; AX=Segmentadresse der geladenen Daten
; Labels:
; NewSeg=Freie Segmentadresse w
; SourceHandle=Handle der Datei w
; Inputsize=L�nge des Datenblocks w

LoadFile:
  push w[cs:NewSeg]     ;Segment retten zur �bergaben
  mov ax,3d00h    ;Datei �ffnen
  int 21h
  jnc >o1         ;Kein Fehler,weiter
  mov dx,ERR_SourceNotOpen
  jmp ErrorBye
o1:
  mov bx,ax       ;Handle �bergeben
  mov w[SourceHandle],ax
        
  mov ah,3fh      ;Block einlesen
  mov cx,32k      ;Soviel wie m�glich laden
  push ds         ;Zielsegment �bergeben
  mov ds,w[cs:NewSeg]
  xor dx,dx       ;Zieladresse
  int 21h         ;Daten laden
  pop ds
  mov ah,3eh      ;Datei schliessen
  int 21h
  mov w[InputSize],ax
  push ax
  shr ax,4      
  inc ax
  add w[NewSeg],ax
  pop ax,ax       ;Segmentadresse steht in AX
  ret

