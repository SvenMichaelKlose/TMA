; *******************************************
; *** LÑdt Datei komplett in den Speicher ***
; *******************************************
; max. Grî·e ist 32k
; Parameter:
; DS:DX=Zeiger auf Namen
; RÅckgaben:
; AX=Segmentadresse der geladenen Daten
; Labels:
; NewSeg=Freie Segmentadresse w
; SourceHandle=Handle der Datei w
; Inputsize=LÑnge des Datenblocks w

LoadFile:
  push w[cs:NewSeg]     ;Segment retten zur öbergaben
  mov ax,3d00h    ;Datei îffnen
  int 21h
  jnc >o1         ;Kein Fehler,weiter
  mov dx,ERR_SourceNotOpen
  jmp ErrorBye
o1:
  mov bx,ax       ;Handle Åbergeben
  mov w[SourceHandle],ax
        
  mov ah,3fh      ;Block einlesen
  mov cx,32k      ;Soviel wie mîglich laden
  push ds         ;Zielsegment Åbergeben
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

