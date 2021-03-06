; **************************
; * Genesis Resourcer V1.0 *
; * (c) 1995 Sven Klose    *
; **************************
        org 100h
        .code

	include"constant.inc"
; *****************
; * Hauptschleife *
; *****************
Genesis_Resourcer:      
	mov dx,TXT_standard           ;Copyright-Meldung
	mov ah,9
	int 21h
	call Init
	call OpenFiles
	mov ax,w[endadr]
	mov dx,TXT_analysing
	mov ah,9
	int 21h
	call TestOnSysline
	mov es,w[MapSeg]      ;Tabelle f�r Labelmarkierungen
	mov ds,w[SourceSeg]   ;Quellcode

	mov si,w[cs:StartAdr]
;        mov bp,BranchStack    ;Stapel f�r Jump/Branchadressen

pass1:  mov b[cs:pass],1
	call ShowAdr
	cmp si,w[cs:endadr]   ;Ende des Codes ?
	jae pass2             ;Ja,zu Pass 2
	call getcommand       ;n�chstes Byte reassemblieren
	jc pass2
	;cl=jumpflag al=Adressierung
	;bl=Mnemonic dx=Adresse
	cmp cl,jumpflag       ;Sprung gefunden ?
	jne NoBranch          ;nein
	push bx               ;Ja,angepeilte Adresse im Mapsegment markieren
	mov bx,dx
	test b[es:bx],datalabel
	jnz Nowr1
	mov al,LocLabel
	or b[es:bx],al
NoWr1:  pop bx
	jmp pass1             ;weiter

NoBranch:
	and al,10011111b      ;Datenadresse ?
	and al,indirect+zeropage+absolute
	jne NoData
	push bx               ;Ja,Datenlabel im Mapsegment markieren
	mov al,Datalabel
	mov bx,dx
	test b[es:bx],loclabel
	jnz Nowr2
	or b[es:bx],al
NoWr2:  pop bx
NoData: jmp pass1

pass2:  push cs               ;Message an User
	pop ds                ;das geschrieben wird
	mov b[pass],2
	mov dx,TXT_writing
	mov ah,9
	int 21h
	mov ah,3ch            ;Zieldatei �ffnen
	xor cx,cx
	mov dx,Destname
	int 21h
	jnc NoDestErr
	mov dx,ERR_nodest
	jmp ErrorBye
d1:     pop si
Done2:
	jmp Done
NoDestErr:
	mov w[DestHandle],ax
	mov si,w[cs:StartAdr] ;Zeiger auf Anfang des Codes
	mov di,LineBuffer
pass2b:
	mov w[cs:LineStart],di
	mov es,w[cs:MapSeg]   ;Tabelle f�r Labelmarkierungen
	cmp si,w[cs:endadr]   ;Ende des Codes ?
	jae Done2             ;Ja,Ende
pass2l1a:
	call ShowAdr
	push si,si            ;Code analysieren
	call GetCommand
	jc d1
	pop si
	push bx,cx            ;Daten retten
	mov cl,ah	      ;Befehlsl�nge von AH nach CL
	xor ch,ch
pass2l1b:
	mov al,b[es:si]       ;Karte auf Markierungen untersuchen
	test al,Loclabel      ;Codelabel
	jz NoLoclabel
	mov dx,TXT_loc        ;Ja, "LOC" in Zieldatei schreiben
	jmp Printadress
NoLocLabel:
	test al,Datalabel      ;Datenlabel ?
	jz NoDatalabel
	mov dx,TXT_dat        ;Ja, "DAT" in Zielsegment schreiben
Printadress:
	call PrintMnemonic    ;Labelname ausgeben
	mov ax,si
	call PrintWord        ;Adresse im Labelnamen
	jmp LabelOk
NoDatalabel:
	inc si
	loop pass2l1b

LabelOk:pop cx,bx             ;Daten wiederholen
	pop si                ;Adresse des Codes wiederholen
	mov es,cs	      ;Zielsegment f�r Zeilenpuffer
	call Tab              ;In Mnemonicspalte springen
	mov b[Mnemonic],bl
	xor bh,bh             ;Mnemonic ausgeben
	mov ax,bx             ;Mnemonicadresse=Nummer*3+Mnemonics
	shl bx,1
	add bx,ax
	add bx,Mnemonics
	mov dx,bx
	call PrintMnemonic
	call Tab
	call GetCommand       ;Adressierung holen
	mov es,cs
	push ax,cx,dx         ;Adressierung retten
	and al,10011111b      ;Indexregister ausmaskieren
	or al,al              ;Implied ?
	je NextCom2	      ;Ja, kein Operand oder Kommentar
	cmp al,immediate      ;Immediate ?
	jne NotImmediate
	mov al,"#"            ;Ja, "#" ausgeben
	jmp SignOut
NotImmediate:
	cmp al,Indirect       ;Indirect ?
	jne Operand
	cmp cl,Jumpflag       ;Bei Sprungbefehl keine K                                                                                                                                                                                                                              
                     
                                                                                                                                                                                                                                                                         el schreiben
	ja Nolab
	mov bp,TXT_loc
	mov bx,dx
	mov es,w[cs:MapSeg]
	test b[es:bx],loclabel
	jnz itsloc
	test b[es:bx],datalabel
	jz nolab
	mov bp,TXT_dat
itsloc: push dx
	mov dx,bp
	call PrintMnemonic
	pop dx
	jmp waslabel
NoLab:  mov es,cs
	push ax               ;Zahlen im Operanden hexadezimal ausgeben
	mov al,"$"
	stosb
	pop ax
waslabel:
	mov es,cs
	dec ah                ;Ist Operand ein Byte ?
	jnz ItsWord           ;Nein, es ist ein Wort
	mov ax,dx             ;Adresse nach AX
	call PrintByte        ;und ausgeben.
	jmp OpEnd             ;Zum Ende der Operandenanalyse ...
NoOp2:  jmp noop
Nextcom2:       
	jmp Nextcom
ItsWord:
	mov ax,dx             ;Adresse nach AX
	call PrintWord        ;und ausgeben
OpEnd:
	pop bx,ax             ;Adressierung wiederholen
	mov bl,al
	and al,xreg+yreg      ;Nur auf Indexregister testen
	and bl,indirect
	or al,al              ;Sind Indexregister vorhanden ?
	jz NoIndex            ;Nein,weiter
	cmp al,xreg           ;,X ?
	jne NoXdirInd         ;Nein,kein Indexregister
	push ax
	mov ax,"X" by ","
	stosw
	pop ax
	jmp NoIndex

NoXdirInd:                    ;Y-Register
	or bl,bl              ;indirect
	jz NoIndY
	call InEndOut
NoIndY: push ax
	mov ax,"Y" by ","
	stosw
	pop ax
	jmp LineOut
NoIndex:or bl,bl              ;indirect ?
	jz Lineout
	cmp cl,jumpflag       ;Keine Klammer bei Sprungbefehlen
	je Noindir
	mov al,")"
	stosb
NoIndir:jmp LineOut

NextCom:pop dx
NoOp:   pop ax

LineOut:mov ax,0a0dh
	stosw         ;Carriage return
	cmp b[Mnemonic],_BRK
	je itsBRK
	cmp b[Mnemonic],_RTS
	jne NoRTS
ItsBRK: mov ax," " by ";"
	stosw
	mov al,"*"
	mov cx,70
	rep stosb
	mov ax,0a0dh
	stosw
NoRTS:  cmp di,7000h
	jb NoSave
	call SaveLines
NoSave: jmp Pass2b

SaveLines:
	pusha         ;Zeile ausgeben
	mov cx,di
	mov ds,cs
	sub cx,LineBuffer
	mov dx,linebuffer
	mov bx,w[cs:DestHandle]
	mov ah,40h
	mov ds,cs
	int 21h
	popa
	mov di,Linebuffer
	ret

InEndOut:
	push ax
	mov al,")"
	stosb
	pop ax
	ret

; Erfolgsmeldung ausgeben und Programm beenden
Done:
	call SaveLines
	mov ds,cs
	mov dx,TXT_success
	mov ah,9
	int 21h
	mov dx,TXT_line
	mov ah,9
	int 21h
	mov ax,4c00h
	int 21h

PrintByte:
	push ax,es,cs
	pop es
	push ax
	shr al,4
	call PrintNibble
	pop ax
	call PrintNibble
	pop es,ax
	ret

PrintNibble:
	and al,15
	cmp al,9
	ja PrintNibble2
	add al,"0"
	jmp PrintNibble3
PrintNibble2:
	add al,"A"-10
PrintNibble3:
	stosb
	ret

PrintWord:
	xchg ah,al
	call PrintByte
	mov al,ah
	call PrintByte
	ret

PrintMnemonic:  
	push ds,es,si,cx,cs,cs
	pop es,ds
	mov si,dx
	mov cx,3
	rep movsb
	pop cx,si,es,ds
	ret

LineStart:
	dw 0
Tab:    push ax,cx,bp
	mov bp,w[cs:LineStart]
	add bp,8
	cmp di,bp
	ja Tab2
	mov cx,bp
	jmp DoTab
Tab2:
	add bp,4
	cmp di,bp
	ja tab3
	mov cx,bp
	jmp DoTab
Tab3:
	add bp,10
	cmp di,bp
	ja Tabend
	mov cx,bp
DoTab:
	sub cx,di
	mov al," "
	cmp cl,0
	je Tabend
	rep stosb
Tabend:
	pop bp,cx,ax
	ret

; *************************************************
; *** Zerlegt Byte in Adressierung und Funktion ***
; *************************************************
;R�ckgabe: AL=Adressierung, CL=Jumpflag, AH=L�nge, BL=Befehlsname
GetCommand:
	push ds
	mov ax,w[cs:SourceSeg]
	mov ds,ax
	mov es,w[cs:MapSeg]

	or b[es:si],1         ;Flag setzen im Mapseg
	lodsb                 ;Adressierung holen
	xor ah,ah
	shl ax,1
	mov bx,ax
	add bx,codes
	mov cx,w[cs:bx]
	or cx,cx
	jz illegalCode
	mov bl,cl       ;Befehlsname nach BL
	mov cl,ch       ;Adressierung nach CL und
	mov al,cl       ;nach al
	and al,11101111b ;Jumpflag aus AX ausblenden
	push ax               ;Register retten f�r R�ckgabe
	and al,10001111b ;indizierung und jumpflag ausblenden
	and cl,jumpflag ;Jumpflag nach cl
	mov bh,1
	cmp al,implied
	je GetC1
	mov bh,2
	cmp al,zeropage
	jne Getc2
getc3b: lodsb
	mov dl,al       ;Adresse nach DX
	xor dh,dh
	mov bh,2
	jmp getc1
getc2:
	cmp al,immediate
	jne getc3
	lodsb
	mov dl,al
	jmp getc1
getc3:
	mov bh,3
	cmp al,absolute
	jne getc4
	cmp si,w[cs:endadr2]
	jae gc2
	lodsw
	mov dx,ax
	mov bh,3
	jmp getc1
getc4:
	cmp al,indirect
	jne getc1
	cmp cl,jumpflag
	jne getc3b
	mov dx,si       ;Indirekte Adresse berechnen
	lodsb
	cbw
	add dx,ax
	inc dx
	mov bh,2
GetC1:  
	pop ax
	mov ah,bh     ;Codel�nge
	pop ds
	clc
	ret
gc2:    pop ax,ds
	stc
	ret

illegalCode:
	mov bx,zeropage by _byt
	mov ax,2 by zeropage
	cmp b[cs:pass],1
	jne illcod2
	mov ah,1
illcod2:mov cx,zeropage
	mov dl,b[si-1]
	pop ds
	ret

TestOnSysline:
	xor ax,ax ;mov ax,801h
	mov w[StartAdr],ax
	ret

init:   mov ds,cs
	mov ax,cs
	mov bx,1000h
	add ax,bx
	mov w[SourceSeg],ax
	add ax,bx
	mov w[MapSeg],ax
	add ax,bx
	mov w[DestSeg],ax
	ret

; ********************************************************
; *** Holt Dateinamen aus Kommandozeile und �ffnet Sie ***
; ********************************************************
OpenFiles:
  mov si,82h      ;Dateinamen suchen
  mov di,SourceName
  call GetEnvName
  mov w[SourceLength],cx
  mov di,DestName
  call GetEnvName
  mov w[DestLength],cx
  mov dx,SourceName
  mov ax,3d00h      ;Datei �ffnen
  int 21h
  jnc Open4       ;Kein Fehler,weiter
  mov dx,ERR_SourceNotOpen
  call ErrorBye
Open4:
  mov bx,ax       ;Handle �bergeben
  mov w[SourceHandle],ax
  
  xor cx,cx       ;Gr�sse der Datei holen
  xor dx,dx
  mov ax,4202h
  int 21h
  push dx,ax
  xor dx,dx
  mov ax,4200h
  int 21h
  pop ax,dx
  mov w[FileSize],ax

; *** Liest Block aus Datei in den Puffer
ReadSourceBlock:
  mov bx,w[SourceHandle]
  mov ah,3fh      ;Block einlesen
  mov cx,-1       ;Soviel wie m�glich laden
  push ds,w[SourceSeg];Zielsegment �bergeben
  pop ds
  xor dx,dx       ;Zieladresse
  int 21h         ;Daten laden
  pop es
  mov ds,cs
  mov bx,ax
  mov w[InputSize],ax
  mov cx,ax       ;Map aufr�umen
  push cx
  xor di,di
  xor al,al
  mov cx,32768
  mov es,w[MapSeg]
  rep stosb
  pop ax 
  add ax,w[StartAdr]
mov ax,-1
  mov w[EndAdr],ax
  sub ax,2
  mov w[endadr2],ax
  ret

; ******************************************************************
; *** Holt Dateinamen aus String und pr�ft dessen Endung auf T64 ***
; ******************************************************************
; Aufruf: DS:SI=Quellstring DS:DI=Ziel
GetEnvName:
  push cs,cs
  pop ds,es
OpenLoop0:
  es:lodsb
  cmp al,' '
  je OpenLoop0
  or al,al
  jz noname
  cmp al,0dh      ;Ende der Zeile ?
  jne Open2	  ;Nein,weiter ...
Noname:
  mov dx,ERR_NoName
  jmp ErrorBye
Open2:
  dec si
  xor cx,cx
OpenLoop3:
  es:lodsb
  es:stosb
  inc cx
  cmp al,' '
  je Open3
  cmp al,'.'
  je gotExtension
  cmp al,0dh
  jne OpenLoop3
Open3:
  dec cx
  dec si
  dec di
  xor al,al       ;Ende des Namens kennzeichnen
  stosb
  ret
GotExtension:
  es:lodsw
  es:stosw
  cmp ax,'6' by 'T'
  je GotT6
  cmp ax,'6' by 't'
  jne NoT6
GotT6:
  inc b[TapeMode]
NoT6:
  mov b[FileExtension],ax
  es:lodsb
  es:stosb
  cmp al,'4'
  jne No4
  inc b[TapeMode]
No4:
  mov b[FileExtension+2],al
  inc si
  inc di
  add cx,4
  jmp Open3

; ******************************
; *** Schliesst alle Dateien ***
; ******************************
CloseFiles:
  mov bx,w[SourceHandle]
  mov ah,3eh      ;Datei schliessen
  int 21h
  mov bx,w[DestHandle]
  mov ah,3eh
  int 21h
  ret

ShowAdr:inc b[cs:ShowCnt]
	cmp b[cs:ShowCnt],128
	jnz Show2
	pusha
	push ds,es
	mov es,cs
	mov ds,cs
	mov di,TXT_adr2
	mov ax,si
	call PrintWord
	mov dx,TXT_adr
	mov ah,9
	int 21h
	pop es,ds
	popa
Show2:  ret
ShowCnt:db 0

TXT_adr:db"Processing "
TXT_adr2:db"----hex",13,"$"
ERR_noName:     
  db"Hello hacker ! Note that this is just an alpha version of Genres !!!",10,13
  db"You'll have to type in source and destination. For example :",0ah,0dh,0ah
  db"GENRES source.bin (or source.t64) dest.a65",0ah,0dh,10,13
  db"The code you want to process is supposed to start at 0801hex anyway.",10,13
  db"There may be some problems disassembling SYS lines because GENRES doesn't"
  db"take care of using adressing modes it shouldn't use.",10,13
  db"The real version will follow soon.",10,13,"$"

ERR_SourceNotOpen:
  db"Sorry, I cannot open your desired sourcefile. Is it the right name ?",0ah,0dh,"$"

TXT_standard:
  db"Genesis Resourcer 6510 V0.1a - This is free software and part of the",10,13
  db"TMA project. Please read COPYING for details.",0ah,0dh
TXT_line:
  db"======================================================================",0ah,0dh,"$"
TXT_analysing:
  db"Analysing ...",10,13,"$"
TXT_remapping:
  db"Remapping ...",10,13,"$"
TXT_writing:
  db"Disassembling to disk ...",10,13,"$"
TXT_success:
  db"Resourcing has been successful. Thank you for using Deepware products.",10,13,"$"

TXT_dest:
  db"; *************************************************",10,13
  db"; *** >> GENESIS RESOURCER V1.0 CREATED FILE << ***",10,13
  db"; ***         A part of the TMA project         ***",10,13
  db"; *************************************************",10,13
TXT_dest2:
ERR_nodest:
  db"Sorry, can't write sourcefile. Disk could be full.",10,13,"$"
TXT_dat:
  db"DAT"
TXT_loc:
  db"LOC"
CMDs:
  db 8,"ABSOLUTE",3,"ABS",3,"BIT",3,"END",3,"FIX",5,"GROUP",3,"GRP"
  db 6,"OFFSET",3,"OFS",3,"REM",0


; **********************************************************
; *** Verl�sst Programm mit Errorlevel und Fehlermeldung ***
; **********************************************************
ErrorBye:
  mov ah,9
  int 21h
  mov ax,4c01h
  int 21h

	include"instruct.inc"

mnemonic:db 0
pass:db 0
SourceHandle:dw 0
DestHandle:dw 0
FileSize:dw 0,0
CodeAdr:dw 0
StartAdr:dw 801h
EndAdr:dw 0
endadr2:dw ?
MapSeg:dw 0
SourceSeg:dw 0
DestSeg:dw 0
TapeMode:db 0
Sourcelength:dw 0,0
DestLength:dw 0,0
SourceName:db 32 dup 0
DestName:db 32 dup 0
inputsize:dw 0
inputpointer:dw 0
fileextension:db "T64"
Locations:dw 0          ;Anzahl der gefundenen Spr�nge
datas:dw 0
includ:dw 0
LineBuffer:db ?
