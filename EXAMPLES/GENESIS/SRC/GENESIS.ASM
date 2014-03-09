; ************************************************
; ************************************************
; ***					       ***
; ***	  GENESIS 6510 V2.2 Crossassembler     ***
; ***	 (c)1994-1996 by Sven Michael Klose    ***
; ***	 Email: sven@devcon.net 	       ***
; *** This file is a part of the TMA project.  ***
; ***					       ***
; ************************************************
; ************************************************

; Sorry, the comments are not translated. FIXME.

	.code
english=1

; ************************************************************
; *** Codes fÅr die Beschreibung legaler Adressierungsmodi ***
; ************************************************************

ByteOp	   = 1
WordOp	   = 2
Implied    = 0 ;(implied always stands alone)
Immediate  = 1
ZeroPage   = 2
ZeroPageX  = 4
Absolute   = 8
AbsoluteX  = 16
AbsoluteY  = 32
IndirectXY = 64+128
Accu	   = 64 		 
Indirect   = 128
uall	   = indirectXY+zeropage+absolute+zeropageX+absoluteY+absoluteX
ucor	   = AMindirectX

; *************************************
; *** Adressierungsbits fÅr Opcodes ***
; *************************************
AMImplied	= 8
AMImmediate	= 8
AMZeroPage	= 4
AMZeroPageX	= 14h
AMAbsolute	= 0ch
AMAbsoluteX	= 1ch
AMAbsoluteY	= 18h
AMIndirectX	= 0
AMIndirectY	= 10h
AMIndirect	= 20h

; **************************************************
; *** Haupsteuerung fÅr Dateihandling und Passes ***
; **************************************************
	code segment
	org 100h
A6510:	push cs,cs
	pop ds,es
	mov dx,TXT_standard
	mov ah,9
	int 21h
	mov di,Datastart ;Datenbereich sÑubern
	mov cx,DataEnd-DataStart
	xor al,al
	rep stosb
	mov ax,cs
	add ax,1000h  ;Segment fÅr Labeltabelle berechnen
	mov w[LabelSeg],ax
	add ax,1000h	;Segment fÅr fertigen Code berechnen
	mov w[CodeSeg],ax
	add ax,1000h	;Platz fÅr Macrotabelle
	mov w[MacroSeg],ax
	add ax,1000h	;Segment fÅr Dateneingabe berechnen
	mov w[InputSeg],ax
	
	mov cx,-1
	xor di,di
	push es,W[CodeSeg]
	pop es
FillCode2:
	mov si,TXT_fill ;Sharewarebemerkung ins Codesegment schreiben
FillCode:
	lodsb
	or al,al
	je FillCode2
	stosb
	loop FillCode
	pop es

	call Openfiles  ;Dateien îffnen
	
	db 66h
	xor ax,ax
	db 66h
	mov w[Proceed],ax

	mov b[pass],1	;zuerst Pass 1 ausfÅhren
	call Assemble
	call CloseFiles

	mov b[TapeMode],0
	call OpenFiles
	mov b[pass],2  ;Auf Pass 2 schalten
	xor ax,ax
	mov w[Codepointer],ax
	mov w[ORGindex],ax
	mov w[FirstOrg],ax
	mov w[Error],ax
	mov w[Lines],ax
	call Assemble	;Pass ausfÅhren
		
	cmp b[MacroMode],0 ;Wurde .EM vergessen ?
	jz NoMacLeft
	inc w[error]
	mov al,21	;Ja, Fehlermeldung
	call PrintError

NoMacLeft:
	mov ax,w[Error]
	or ax,ax
	jz NoErrors
	call pline	;Fehlermeldung ausgeben
	mov ax,w[error]
	xor bp,bp
	call printword
	mov dx,TXT_errors
	mov ah,9
	int 21h
	jmp endofprog

noerrors2:
	jmp EndOfProg
NoErrors:
	mov ah,3ch	;Assemblierte Daten schreiben
	xor cx,cx
	mov dx,DestName
	int 21h
	jc Derr
	mov bx,ax
	cmp b[TapeMode],2 ;Tapefile erstellen ?
	jne NoTape	;Nein
	mov ax,w[codepointer]
	sub ax,w[orgindex]
	dec ax
	mov w[tapeend],ax ;Endadresse des Codes
	mov cx,60h	;Ja, Header schreiben
	mov dx,TapeCode
	mov ah,40h
	int 21h
	jc derr
	mov cx,3ah
TLoop:	push cx 	;freie EintrÑge schreiben
	mov cx,16
	mov dx,Nothing
	mov ah,40h
	int 21h
	jc derr
	pop cx
	loop tloop

	push bx
	mov dx,TXT_tapemode
	mov ah,9
	int 21h
	pop bx
	jmp NoTape

derr:	jmp DOSerrorRoutine

NoTape: mov ax,w[SysAdr];SYS-Kommando einfÅgen ?
	or ax,ax
	jz NoSysLine	;Nein,Weiter
	push bx
	mov bp,1
	mov di,SysChar
	mov ax,w[SysAdr]
	push cs
	pop es
	call PrintWord
	mov dx,TXT_SysMode
	mov ah,9
	int 21h
	pop bx
	mov cx,0dh	;Zeile schreiben
	mov dx,SysLine
	mov ah,40h
	int 21h
	jc derr2
NoSysLine:
	mov ax,w[Labels]
	xor bp,bp
	call PrintWord
	mov dx,TXT_labels
	mov ah,9
	int 21h
	mov ax,w[Lines] 	;Gibt Anzahl der Zeilen aus
	xor bp,bp
	call PrintWord
	mov dx,TXT_lines
	mov ah,9
	int 21h

	mov cx,w[CodePointer]
	sub cx,w[ORGindex]	;Index zurÅcknehmen
	mov dx,w[FirstOrg]
	sub cx,dx
	push cx 		;CodelÑnge merken
	push ds,w[CodeSeg]
	pop ds
	mov ah,40h
	int 21h
	jc derr2
	pop ds,bp
	mov dx,TXT_size
	mov ah,9
	int 21h
	mov ax,bp
	xor bp,bp
	call PrintWord
	mov dx,TXT_bytes
	mov ah,9
	int 21h
	call pline
	mov ah,9
	mov dx,txt_ok
        int 21h
_exit:
EndOfProg:
	mov ax,4c00h
	int 21h
SyntaxErr:
	mov al,8
	jmp ErrorEnd

derr2:	jmp DOSerrorRoutine

pline:	mov dx,txt_line
	mov ah,9
	int 21h
	ret

cret:	mov al,13
	int 29h
	mov al,10
	int 29h
	ret

; ********************************
; *** FÅhrt einzelnen Pass aus ***
; ********************************

SyntaxErr2:jmp SyntaxErr
EQUjump:jmp EQUequal	;Ansprung der EQU-Routine bei Gleichzeichen

Assemble:
	call getline	;Zeile holen
	jc EndOfPass	;bei Ende zurÅck ...
	;push si
	;mov si,line
	;call print
	;pop si
ContinueLine:
	call DelSpaces	;Leerzeilen Åberspringen
	cmp al,0dh
	je Assemble
	cmp al,'.'	;Directive berÅcksichtigen
	je Directive
	cmp al,'='
	je EQUjump
	cmp al,'A'
	jl SyntaxErr2
	cmp al,'Z'
	jg SyntaxErr2
Directive:
	call GetStringLen ;StringlÑnge holen
	cmp al,3        ;LÑnge=3 ?`
	jne GetLabel    ;Nein,als Label eintragen
	call GetMnemonic;auf Mnemonic testen.
	jc GetLabel     ;Kein Mnemonic, als Label eintragen.
	cmp al,75	;Directive ?
	jb Assemble10
	sub al,75	;Ja,aufrufen ...
	xor ah,ah
	shl ax,1
	mov bx,ax
	add bx,DirTab
	jmp w[cs:bx]

Assemble10:
	mov b[Mnemonic],al;Mnemonic vermerken.
	call DelSpaces
	lodsb
	cmp al,0dh      ;Ende der Zeile
	jne Assemble2   ;Nein,weiterprÅfen
	mov b[OpMode],Implied ;Ja,kein Operand
	mov b[OpCode],AMimplied
	mov b[OpSize],0
	jmp WriteCode

GetLabel:
	jmp GetLabel2

EndOfPass:
	ret

Assemble2:
	cmp al,'#'      ;Immediate ?
	jne NotImmediate
	call GetByte    ;Byte holen
	jc ErrorEnd
	mov b[Op],al	;Wert speichern
	mov b[OpSize],byteop
	mov b[OpMode],Immediate
	mov b[OpCode],AMimmediate
	jmp WriteCode	;und Code schreiben.
ErrorEnd:
	jmp ErrorEnd2

NotImmediate:
	cmp al,'('      ;Indirect Indexed ?
	jne NotIndirect ;Nein,muss Absolute oder ZeroPage sein
	mov b[OpSize],byteop ;Operand ist Byte
	call GetWord    ;Operand holen/Adresse nur im Zero Page
	jc ErrorEnd     ;Fehler im Operanden                   
	mov w[Op],ax    ;Wert speichern
	call DelSpaces
	lodsb
	cmp al,')'	;Klammer hinter dem Wert ?
	je TestOnIndirectY;Ja,auf Y indexed prÅfen
	cmp al,','      ;Komma vorhanden ?
	je TestOnIndirectX;Nein,prÅfen ob X-register angegeben ist
	mov al,7        ;Fehler,Operand ist unvollstÑndig
	jmp ErrorEnd
TestOnIndirectX:
	cmp b[Op+1],0
	jnz IndErr
	mov b[OpMode],indirectXY
	mov b[OpCode],AMindirectX ;<<<<<<<<<<<<<<<<<<<< kann auch Y sein !!
	call DelSpaces
	lodsb
	cmp al,'X'
	je WriteCode3
	xor al,al       ;X expected
	jmp ErrorEnd
IndErr: mov al,10
	jmp ErrorEnd

TestOnIndirectY:
	call DelSpaces
	lodsb
	cmp al,','	;Klammer vorhanden
	je Assemble3    ;Ja,weiterprÅfen
	mov b[Opmode],Indirect ;Nein,Adressierung ist "indirect"
	mov b[OpCode],AMindirect
	mov b[OpSize],byteop
	mov ax,w[Op]	;Relative Adresse berechnen
	mov bx,w[CodePointer]
	add bx,2
	sub ax,bx
;	 push ax
	cmp ax,-80h
	jl RelativeErr
	cmp ax,7fh
	jg RelativeErr

RelativeOk:
;        pop ax
	mov b[Op],al
WriteCode3:
	jmp WriteCode2
NotIndirect:
	jmp NotIndirect2
RelativeErr:
	pop ax
	mov al,13
	jmp ErrorEnd

Assemble3:
	cmp b[Op+1],0
	jnz IndErr
	call DelSpaces
	lodsb
	cmp al,'Y'	;Y register angegeben ?
	jne TestOnIndirectYErr;Nein, Fehlermeldung hervorrufen
	mov b[OpMode],IndirectXY
	mov b[OpCode],AMindirectY
	jmp WriteCode

TestOnIndirectYErr:
	mov al,1
	jmp ErrorEnd

NotIndirect2:            ;entweder ZP oder Absolute
	dec si
	call GetWord	;Wert holen
	jnc Assemble5
	mov al,2	;Fehler im Operanden
	jmp ErrorEnd

; *** ABSOLUTE
Assemble5:
	mov w[Op],ax	;Wert speichern/Testen auf Indexregister
	mov b[OpMode],absolute
	mov b[OpCode],AMabsolute
	mov b[OpSize],wordop
	lodsb
	cmp al,','	;Komma vorhanden
	jne TestOnZeroPage;Nein,testen auf zero page
	call DelSpaces
	cmp al,'X'
	jne TestOnY
	mov b[OpMode],absoluteX
	mov b[OpCode],AMabsolutex
	jmp TestOnZeroPage
WriteCode:
	jmp WriteCode2
TestOnY:cmp al,'Y'
	je Assemble4
	mov al,2
	jmp ErrorEnd
Assemble4:
	mov b[OpMode],absoluteY
	mov b[Opcode],AMabsoluteY
TestOnZeroPage:
	mov b[OpSize],wordop
	mov ax,w[Op]
	or ah,ah        ;Highbyte gleich null ?
	jne WriteCode
	mov al,b[OpMode]
	cmp al,AbsoluteY
	jne GotZero     ;zeropageY ist nicht mîglich
	mov al,b[Mnemonic];ZeropageY ist mîglich bei LDX & STX
	cmp al,30       ;LDX ?
	je ChangeToZPX  ;Ja, modifizieren
	cmp al,48       ;STX ?
	je ChangeToZPX
	jmp WriteCode   ;Zero page ist nicht mîglich ...
ChangeToZPX:            ;éndert Adressierung von AbsoluteY um in AboluteX ...
	mov b[OpMode],absoluteX ;um ZeroPageY zu erzielen
	mov b[OpCode],AMabsoluteX

GotZero:mov b[OpSize],byteop
	cmp al,Absolute
	je Justzeropage
	mov b[OpMode],zeropageX
	mov b[OpCode],AMzeropageX
	jmp WriteCode
Justzeropage:
	mov b[OpMode],zeropage
	mov b[OpCode],AMzeropage

; ********************************************************
; *** Bildet Opcode aus Mnemonic und Adressierungsmodi ***
; ********************************************************

WriteCode2:
	cmp b[MacroWrite],0     ;Zeile in Macrotabelle schreiben ?
	jz NoMacroLine
	cmp b[Pass],2           ;nicht in Pass 2
	je NoMacroLine
	push es,w[MacroSeg],w[MacroPointer]
	pop di,es
	mov si,line
MacrolineLoop:	      
	lodsb
	cmp al,0dh
	je LineWritten
	stosb
	jmp MacrolineLoop
LineWritten:
	stosb
	pop es
	push di
	pop w[MacroPointer]
NoMacroLine:
	mov al,b[OpMode]	;Korrigieren von "Immediate" bei Indexreg.
	cmp al,Immediate
	jne Write7
	mov al,b[LastChar]
	cmp al,'X'
	je Write8
	cmp al,'Y'
	jne Write7
Write8: mov b[OpCode],AMindirectX ;Immediate=0
Write7: mov al,b[Mnemonic]      ;Position des Eintrags berechnen
	xor ah,ah
	shl ax,1
	add ax,Descriptions
	mov bx,ax
Retry:  mov al,b[bx]            ;Adressierung prÅfen
	and al,b[OpMode]
	cmp al,b[OpMode]
	je WriteCodeOk          ;Adressierung ist Ok
	mov al,b[OpMode]	;ZeroPage in Absolute Ñndern ?
	cmp al,ZeroPage
	jne NoChange1
	mov b[OpMode],absolute
	mov b[OpCode],AMabsolute
	jmp Retry
NoChange1:
	cmp al,ZeroPageX
	jne NoChange2
	mov b[OpMode],absoluteX
	mov b[OpCode],AMabsoluteX
	jmp Retry
NoChange2:              ;Kein ZeroPage ...
	mov al,b[bx]    ;Ist Åberhaupt ein Operand gefragt ?
	cmp al,accu
	jne Write3
	mov al,5
	jmp ErrorEnd
Write3: cmp al,indirect;fehlen bloss Klammern fÅr indirekten Sprungbefehl ?
	jne Write3b     ;wohl kaum ...
	cmp b[OpMode],absolute
	jne Write3b     ;falsche Adressierung angeben
	mov b[OpMode],indirect
	mov b[OpCode],AMindirect
	mov b[OpSize],byteop
	mov ax,w[Op]    ;Relative Adresse berechnen
	mov bx,w[CodePointer]
	add bx,2
	sub ax,bx
	push ax
	or ax,ax
	jz RelativeOk2
	cmp al,127
	jl RelativeOk2
	neg al
	cmp al,-128
	jge RelativeOk2
	jmp RelativeErr
RelativeOk2:
	pop ax
	mov b[Op],al
	jmp WriteCode
Write3b:mov al,4                ;Falsche Adressierung...
	jmp ErrorEnd            ;fÅr diesen Befehl.
WriteCodeOk:
	or al,al                ;wurde kein Operand angegeben ?
	jnz Write4		;doch,alles korrekt
	mov al,b[bx]
	and al,11000000b        ;accu ?
	cmp al,accu
	je Write4		;Ok
	mov al,6		;Operand wird gebraucht.
	jmp errorend

Write4: mov al,b[Mnemonic]      ;wurde fÅr LDX/STX kein ZeroPageX benutzt ?
	cmp al,30
	je TestXinst
	cmp al,48
	jne Write10
TestXinst:
	mov al,b[OpMode]
	cmp al,ZeroPageX
	Jne Write10		;Nein,alles in Ordnung ...
	mov al,17
	jmp ErrorEnd

Write10:inc bx                  ;Adressierungscode + base code
	mov al,b[OpCode]
	add al,b[bx]
	push di,es,w[CodePointer],w[codeseg]
	pop es,di
	sub di,w[ORGindex]
Write10b:
	stosb
	;Korrigieren von JMP indirect
	cmp al,60h      ;RTS oder JMP Indirect ?
	jne NoCorrJMP
	cmp b[OpMode], indirect ;Indirect
	jne NoCorrJMP   ;Es ist ein RTS,weiter.
	dec di
	mov al,6ch      ;Ja,es ist ein JMP ind. , Wert korrigieren
	jmp Write10b
NoCorrJMP:
	mov al,b[OpSize]
	or al,al
	jz write6	;kein Operand
	cmp al,WordOp
	je WriteWord
	mov al,b[Op]
	stosb
	jmp Write6
WriteWord:
	mov ax,w[Op]
	stosw
Write6: add di,w[ORGindex]
	push di
	pop w[CodePointer],es,di
	call DelSpaces
	lodsb
	cmp al,0dh
	jmp Write5
	mov al,3
	jmp errorend2

Write5: jmp assemble

; **********************************************
; *** Label mit Adresse in Tabelle eintragen ***
; **********************************************
GetLabel2:                  
	push w[MacroMode]	;Makromodus speichern
	mov b[MacroMode],2
	push si
	call GetLabelData
	jc GetLabel2b           ;Label ist kein Macro
	mov w[MacInsAdr],ax	;Adresse des Makrotextes speichern
	pop ax			;SI nicht wiederholen
	pop w[MacroMode]
	call GetMnemonic
	jc NoMACline
	cmp al,61		;MAC-Befehl ?
	jne DoubleLabel2        ;Nein,Label ist doppelt
	jmp MACroutine          ;Ja,zur MACroutine
NoMACLine:
	mov di,MacroWordtab     ;Wordpuffer initialisieren
	push di
	xor ax,ax
	mov cx,8
	rep stosw
	pop di
GetNextWord:
	call Delspaces
	cmp al,0dh
	je GoAssemble
	call GetWord    ;öbergebenen Wert in Buffer eintragen
	stosw
	lodsb
	cmp al,','      ;PrÅfen,ob Komma als Trennung benutzt wurde
	je GetNextWord	;und ggf. Åberspringen.
	dec si
	jmp GetNextWord
GoAssemble:             ;Alle Werte sind Åbertragen     
	mov b[MacroMode],1 ;Flags setzen, damit Makro eingefÅgt wird
	mov b[MacroWrite],0
MacInsEnd2:
	push w[MacroSeg],w[MacInsAdr],w[MacSize]
	push w[InputSeg],w[InputPointer],W[InputSize]
	pop w[MacSize],w[MacInsAdr],w[MacroSeg]
	pop w[InputSize],w[InputPointer],w[InputSeg]
	jmp Assemble
MacInsEnd:
	pop es,ds
	mov b[MacroMode],0
	jmp MacInsEnd2
DoubleLabel2:
	jmp doublelabel
GetLabel2b:
	pop si
	pop w[MacroMode]
	
	mov bl,b[cs:pass]  ;Wenn Pass 2, keine Labels mehr eintragen
	cmp bl,1
	je g
	
	call GetStringLen
	xor ah,ah
	add si,ax
	jmp ContinueLine
g:      
	push si
	call GetLabelData       ;PrÅfen, ob Label schon existiert
	jnc DoubleLabel         ;Ja,Fehlermeldung
	inc w[Labels]
	pop si
	call GetStringLen       ;Label eintrageen
	xor ah,ah
	mov cx,ax
	push ds,es
	mov es,w[LabelSeg]
	mov di,w[LabelPointer]
	mov al,cl       ;LÑnge des Labels speichern
	stosb
	rep movsb       ;Label Åbertragen
	
	mov al,b[MacroMode] ;Endmarkierung des Labels
	stosb		;bei Makro 1 sonst 0
	mov ax,w[cs:CodePointer]
	mov cl,b[MacroWrite] ;relative Adresse berechnen ?
	cmp cl,1
	jne NoRelativeAdress
	sub ax,w[MacroAdress] ;Ja,Startadresse des Macros abziehen
NoRelativeAdress:
	stosw
	pop es,ds
	mov w[Labelpointer],di
	jmp continueLine
DoubleLabel:
	pop si
	mov al,11
	jmp ErrorEnd

; ************************************************
; *** Liest Wert eines Labels aus Labeltabelle ***
; ************************************************
GetLabelData:
	call GetStringLen
	mov dx,ax
	xor dh,dh
	push es,w[LabelSeg]
	pop es
	xor di,di       ;Ab erstem Label anfangen
	mov bp,si	;Position des gesuchten Labels
NextLoop:
	cmp di,w[LabelPointer]
	je NoLabel      ;Ende der Liste, kein Label gefunden
	xchg si,di      ;Adresse des nÑchsten Labels berechnen
	es:lodsb
	xchg si,di
	mov bl,al
	xor bh,bh
	add bx,di
	add bx,3        ;Labelheader Åberspringen
	mov si,bp       ;Position des gesuchten Labels finden
	mov cx,dx

GetLabelLoop:
	lodsb
	mov ah,al
	xchg si,di
	es:lodsb
	xchg si,di
	cmp ah,al
	jne NextLabel
	loop GetLabelLoop
	
	xchg si,di	;Auf Endmarkierung prÅfen
	es:lodsb
	cmp al,b[MacroMode];or al,al
	jnz NextLabel
	es:lodsw        ;Adresse nach AX
	
	mov bl,b[MacroMode];PrÅfen, ob richtige Adresse Åbergeben werden muss
	cmp bl,1
	jne NoMacroLabel
	add ax,w[MacroAdress];Anfangsadresse des Macros hinzuaddieren
NoMacroLabel:
	xchg si,di
	mov si,bp       ;1. Buchstabe nach neuem Label
	add si,dx
	pop es
	clc
	ret
NextLabel:
	;mov ah,b[MacroMode] ;Bei Macromodus Macrolabels auch
	;cmp al,ah
	;jne NextLabel2
	;es:lodsw
	;add ax,W[MacroAdress] ;richtige Adresse berechnen
	;jmp MacroLabelFound

NextLabel2:
	mov di,bx
	jmp NextLoop
NoLabel:pop es
	mov si,bp
	add si,dx
	stc
	ret

; *************************************************
; *** Gibt Fehlermeldung auf dem Bildschirm aus ***
; *************************************************
ErrorEnd2:
	inc w[cs:error]
	cmp b[Pass],2
	je Error5
	jmp assemble
	cmp al,11
	je Error5
	cmp al,12
	je Error5
Error6: jmp assemble
Error5: call PrintError
	jmp Assemble
PrintError:
	push si
	push ax
	mov dx,TXT_Sorry
	mov ah,9
	int 21h
	pop cx
	mov si,ErrorMessages
	xor ch,ch
error4: or cl,cl
	jz Error2
Error3: lodsb
	or al,al
	jnz Error3
	dec cx
	jmp Error4
Error2: mov bp,0
	call Print
	mov si,line
	call Print
	pop si
	ret

; ***************************************
; *** Gibt Zeichenkette aus zur Datei ***
; ***************************************
Print:  lodsb
	or al,al
	jz PrintEnd
	cmp al,0dh
	je PrintEnd
	mov ah,6
	mov dl,al
	int 21h
	jmp Print
PrintEnd:
	cmp bp,1
	je PrintEnd2
	mov dx,TXT_CR
	mov ah,9
	int 21h
PrintEnd2:
	ret

; **********************************************************
; *** VerlÑsst Programm mit Errorlevel und Fehlermeldung ***
; **********************************************************
ErrorBye:
	mov ah,9
	int 21h
	mov ax,4c01h
	int 21h

; ********************************************************
; *** Holt Dateinamen aus Kommandozeile und îffnet Sie ***
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
	mov ax,3d00h	  ;Datei îffnen
	int 21h
	jnc Open4       ;Kein Fehler,weiter
	mov dx,ERR_SourceNotOpen
	call ErrorBye
Open4:  mov bx,ax       ;Handle Åbergeben
	mov w[SourceHandle],ax
	
	xor cx,cx       ;Grîsse der Datei holen
	xor dx,dx
	mov ax,4202h
	int 21h
	push dx,ax
	xor dx,dx
	mov ax,4200h
	int 21h
	db 66h
	pop ax
	mov bl,b[Pass]
	cmp bl,2
	je NoAdd
	db 66h
	xor dx,dx
	db 66h
	mov bx,100
	dw 0
	db 66h
	div bx
	db 66h
	add w[FileSize],ax
NoAdd:  jmp ReadSourceBlock2

; *** Liest Block aus Datei in den Puffer
ReadSourceBlock:
	db 66h
	mov bx,w[Filesize]
	db 66h
	mov ax,w[Proceed]
	db 66h
	xor dx,dx
	db 66h
	or bx,bx
	jz readsourceblock2	;BX=0, don't print message
	db 66h
	div bx
	xor bp,bp
	call PrintWord
	mov dx,TXT_proceed
	mov ah,9
	int 21h
	mov al,[Pass]
	xor ah,ah
	call PrintWord
	mov dx,TXT_proceed2
	mov ah,9
	int 21h

ReadSourceBlock2:
	mov bx,w[SourceHandle]
	mov ah,3fh      ;Block einlesen
	mov cx,-1	;Soviel wie mîglich laden
	push ds,w[Inputseg];Zielsegment Åbergeben
	pop ds
	xor dx,dx       ;Zieladresse
	int 21h         ;Daten laden
	pop ds
	mov bx,ax
	mov w[InputSize],ax
	push bx,ax
	db 66h
	xor ax,ax
	pop ax
	shr ax,1
	db 66h
	add w[Proceed],ax

	pop bx
	xor ax,ax
	mov w[InputPointer],ax
	ret


; ******************************************************************
; *** Holt Dateinamen aus String und prÅft dessen Endung auf T64 ***
; ******************************************************************
; Aufruf: DS:SI=Quellstring DS:DI=Ziel
GetEnvName:
	mov ds,cs
	mov es,cs
OpenLoop0:
	lodsb
	cmp al,' '
	je OpenLoop0
	or al,al
	jz EndofName
	cmp al,0dh      ;Ende der Zeile ?
	jne Open2       ;Nein,weiter ...
Endofname:
	mov dx,ERR_NoName
	jmp ErrorBye
Open2:  dec si
	xor cx,cx
OpenLoop3:
	lodsb
	stosb
	inc cx
	cmp al,' '
	je Open3
	cmp al,'.'
	je gotExtension
	cmp al,0dh
	jne OpenLoop3
Open3:	dec cx
	dec si
	dec di
	xor al,al	;Ende des Namens kennzeichnen
	stosb
	ret
GotExtension:
	lodsw
	stosw
	cmp ax,'6' by 'T'
	je GotT6
	cmp ax,'6' by 't'
	jne NoT6
GotT6:  inc b[TapeMode]
NoT6:   mov [FileExtension],ax
	lodsb
	stosb
	cmp al,'4'
	jne No4
	inc b[TapeMode]
No4:    mov [FileExtension+2],al
	inc si
	inc di
	add cx,4
	jmp Open3

; ******************************
; *** Schliesst alle Dateien ***
; ******************************
CloseFiles:
	mov bx,[SourceHandle]
	mov ah,3eh      ;Datei schliessen
	int 21h
	mov bx,w[DestHandle]
	mov ah,3eh
	int 21h
	ret

; ****************************************************
; *** Holt eine Zeile aus Quelldatei in den Puffer ***
; ***		 und entfernt Kommentare	   ***
; ****************************************************
FileEnd2:jmp fileend
GetLine:inc w[Lines]
;        mov ax,w[lines]
;	 xor bp,bp
;	 call Printword
;        mov al,13
;        int 29h
	push ds,es		       
	mov es,w[InputSeg]
	mov di,line
	mov si,w[InputPointer]
GetLine2:
	call GetFB      ;prÅfen, ob Blockende erreicht ist
	es:lodsb
		
	or al,al        ; Ende eines Makrotextes ?
	jnz NoMacEnd
	cmp b[MacroMode],0 ;Makromode an ?
	jz NoMacEnd
	jmp MacInsEnd
NoMacEnd:
	cmp al,0ah
	je tab2
	cmp al,9        ;tab ?
	jne Notab2
tab2:   mov al," "
NoTab2: cmp al,0dh
	je GotLine
	cmp al,';'
	je GotLine2
	cmp al,'"'
	jne ConvASCII
	mov b[di],al
	inc di
Getline3:
	 call getFB
	es:lodsb
	cmp al,0ah
	je tab
	cmp al,9        ;Tab ?
	jne Notab
tab:    mov al," "      ;Ja, in Space umwandeln
NoTab:	mov b[di],al
	inc di
	cmp al,0dh
	je GotLine
	call GetFB
	cmp al,'"'
	jne Getline3
	jmp GetLine2
ConvASCII:
	cmp al,'a'
	jl GetLine4
	cmp al,'z'
	jg GetLine4
	sub al,'a'-'A'
GetLine4:
	mov b[di],al
	inc di
	jmp getline2
GotLine2:        
	call GetFB
	es:lodsb
	cmp al,0dh
	jne GotLine2
GotLine:inc si
	mov w[InputPointer],si
	mov b[di],0dh
	mov si,line
	pop es,ds
	clc
	ret

GetFB:	cmp si,w[InputSize];lÑdt nach wenn nîtig
	jne GetFBOk
	push ds,es,di
	call ReadSourceBlock
	pop di,es,ds
	cmp w[InputSize],0
	je FileEnd

GetFbOk:ret

; *************************************************
; *** Schliesst aktuellen Pass bei Dateiende ab ***
; *************************************************
FileEnd:pop ax,es,ds
	pop ax
	mov al,b[EXTdepth];Include-Datei ?
	or al,al
	je FileEnd3;Nein, Pass ordentlich beenden.
	dec b[EXTdepth];Ja, vorheriges Handle holen.
	mov bl,b[EXTdepth]
	xor bh,bh
	shl bx,3
	add bx,EXTstack
	mov ax,w[bx]
	push ax
	mov ax,w[bx+2]
	mov w[InputPointer],ax
	mov ax,w[bx+4]
	mov w[InputSize],ax
	mov ax,w[InputSeg]
	sub ax,800h
	mov w[InputSeg],ax
	pop ax
	mov bx,w[SourceHandle];Handle der aktuellen Datei holen ...
	mov w[SourceHandle],ax;(vorheriges Handle eintragen]
	mov ah,3eh;... und schliessen.
	int 21h
	jmp assemble ;weitermachen
FileEnd3:	;Alle Dateien wurden abgearbeitet, Pass beenden.
	stc
	ret

; *******************************
; *** öberspringt Leerzeichen ***
; *******************************
DelSpaces:
	lodsb
	cmp al," "
	je delspaces
	dec si
	ret

; ************************
; *** Holt StringlÑnge ***
; ************************
GetStringLen:
	push si
	xor cx,cx
GSL1:   lodsb
	inc cx
	cmp al,0dh
	je GSL2
	cmp al,','
	je GSL2
	cmp al,')'
	je GSL2
	cmp al,'='
	je GSL2
	cmp al,'+'
	je GSL2
	cmp al,'-'
	je GSL2
	cmp al,'*'
	je gsl2
	cmp al,'/'
	je gsl2
	cmp al,'&'
	je gsl2
	cmp al,'|'
	je gsl2
	cmp al,'"'
	je gsl2
	cmp al,' '
	jne GSL1
GSL2:	dec cx
	mov al,cl
	pop si
	ret

; *******************************
; *** Wandelt Text in Word um ***
; *******************************
GetOpAttr:
	mov b[OpAttr],al
	lodsb
	jmp GetWord2c
GetWord2:
	xor ax,ax
	mov b[CompMode],al
	mov w[Op],ax
GetWord2b:
	call DelSpaces
	cmp al,'-'              ;Wurde eine negative Zahl angegeben ?
	jne GetWord2d           ;Nein, weiter
	push ax
	mov al,'<'
	mov b[OpAttr],al
	pop ax
	mov bx,100h             ;negative Zahl gefunden
	jmp GotWordOka
GetWord2d:
	mov b[OpAttr],0
	lodsb
	cmp al,'<'
	je GetOpAttr
	cmp al,'>'
	je GetOpAttr
GetWord2c:
	cmp al,'$'		;Hexadezimale Konstante ?
	jne TestOnBin		;Nein, weiter
	xor bx,bx
HexLoop:lodsb
	cmp al,'A'
	jl Hex1
	cmp al,'F'+1
	jg GotHex
	sub al,'A'-10
Hex2:   or bl,al
	rol bx,4
	jmp HexLoop
Hex1:	cmp al,'0'
	jl GotHex
	cmp al,'9'
	Jg GotHex
	sub al,'0'
	jmp Hex2
GotHex: call CheckOnValidConstant
	ror bx,4
	dec si
	jmp GotWord
TestOnBin:
	cmp al,'%'
	jne TestOnOct
	xor bx,bx
Bin1:   lodsb
	cmp al,'0'
	je Bin2
	cmp al,'1'
	jne GotBin
Bin2:   sub al,'0'
	or bl,al
	rol bx,1
	jmp Bin1
GotBin: call CheckOnValidConstant
	ror bx,1
	dec si
	jmp GotWord
TestOnOct:
	cmp al,'!'
	jne TestOnDec
	xor bx,bx
Oct1:	lodsb
	cmp al,'0'
	jl GotOct
	cmp al,'7'
	jg GotOct
	sub al,'0'
	or bl,al
	rol bx,3
	jmp Oct1
GotOct: call CheckOnValidConstant
	ror bx,3
	dec si
	jmp GotWord
TestOnDec:
	cmp al,'0'
	jl TestOnChar
	cmp al,'9'
	jg TestOnChar
	dec si
	xor bx,bx
	xor ax,ax
Dec1:   lodsb
	cmp al,'0'
	Jl GotDec
	Cmp al,'9'
	jg GotDec
	sub al,'0'
	add bx,ax
	mov dx,bx
	shl dx,1
	xor bx,bx
	add bx,dx
	add bx,dx
	add bx,dx
	add bx,dx
	add bx,dx
	jmp Dec1
GotDec: call CheckOnValidConstant
	xor dx,dx
	mov ax,bx
	or ax,ax 
	jz GotDec2
	mov bx,10
	div bx
GotDec2:mov bx,ax
	dec si
	jmp GotWord

TestOnChar:
	cmp al,'"'
	jne TestOnCodePosition
	xor bx,bx
	lodsb
	mov bl,al
	lodsb
	cmp al,'"'
	je Char2
	mov bh,al
	lodsb
	cmp al,'"'
	jne CharErr
Char2:	jmp GotWord
CharErr:mov al,16
	stc
	ret

TestOnCodeposition:             ;öbergibt aktuelle Codeposition
	cmp al,'*'              ;durch das '*'-Zeichen.
	jne TestOnMacDir	;prÅfen ob Operand eine Macrovariable ist
	mov bx,w[CodePointer]
	jmp GotWord

TestOnMacDir:
	push si
	call GetStringLen
	cmp al,3
	jne TestOnLabel
	pop si
	push si
	call GetMnemonic        ;Auf Macrovariable prÅfen
	jc TestOnLabel          ;weiter ...
	pop si
	mov bx,MacroWordTab
	cmp al,58
	jl MacDirErr
	cmp al,59
	jg MacDirErr
	call GetByte
	shl ax,1
	mov bx,ax
	mov ax,w[bx]
	mov bx,ax
	xor ax,ax
	mov w[Op],ax
	jmp GotWord
MacDirErr:
	mov al,23
	jmp ErrorEnd

TestOnLabel:    ;Operand ist Label, Daten holen
	pop si
	dec si
	call GetLabelData
	jnc GotWord2   ;Label gefunden, weiter ...
	mov ax,w[CodePointer];Label existiert nicht,
	mov bl,b[Pass]  ;bei Pass 1 aktuelle Position Åbergeben
	cmp bl,1
	jne TOL2	;Pass 2, Fehler ausgeben
GotWord2:
	mov bx,ax
GotWord:                ;Auf Rechenoperation prÅfen
	mov al,b[OpAttr]
	or al,al
	jz GotNoAttr
	cmp al,'>'
	jne AttrLow
	mov bl,bh
AttrLow:xor bh,bh
GotNoAttr:
	mov al,b[CompMode];Rechenoperation ausfÅhren ?
	or al,al
	jz GotWordOka;Nein,Ende
	cmp al,'+'
	je Plus
	cmp al,'-'
	je minus
	cmp al,'*'
	je multiply
	cmp al,'/'
	je divide
	cmp al,'&'
	je ANDcomb
	cmp al,'|'
	je ORcomb
	jmp GetWordErr
GotWordOka:
	mov w[op],bx
GotWordOk:
	call delspaces
	cmp al,'+'
	je GotComp
	cmp al,'-'
	je GotComp
	cmp al,'*'
	je GotComp
	cmp al,'/'
	je gotcomp
	cmp al,'&'
	je gotcomp
	cmp al,'|'
	je GotComp
	mov ax,w[Op]
	clc
	ret

TOL2:	stc		;Label existiert nicht
	mov al,12
	ret
GetWordErr:             ;Programmierer wusste auch nicht was er wollte ...
	stc
	mov al,8
	ret
GotComp:inc si
	mov b[CompMode],al
	jmp GetWord2b

CheckOnValidConstant:
	cmp al,"A"      ;Wurde ein Steuerzeichen eingesetzt ?
	jnb cfvcError    ;Nein, String enthÑlt Fremdzeichen
	ret
cfvcError:
	pop ax          ;Stack korrigieren
	mov ax,24       ;Fehlercode in AX
	stc             ;in Carryflag setzen
	ret

; *********************************
; *** diverse Rechenoperationen ***
; *********************************
Plus:   add w[Op],bx
	jmp GotWordOk
Minus:  sub w[Op],bx
	jmp GotWordOk
multiply:
	mov ax,w[op]
	xor dx,dx
	mul bx
	mov w[op],ax
	jmp GotWordOk
divide: mov ax,w[Op]
	xor dx,dx
	div bx
	mov w[Op],ax
	jmp GotWordOk
ANDcomb:and w[op],bx
	jmp GotWordOk
ORcomb: or w[op],bx
	jmp GotWordOk

; *** Wandelt Text in Word um
GetByte:push w[lastchar]
	call GetWord2
	jc GetByteErr2
	or ah,ah
	jz GetByte2
	mov al,10
GetByteErr2:
	mov bl,b[pass];in Pass 1 jeden Wert annehmen
	cmp bl,1
	je GetByte2
	pop w[Lastchar]
	pop bx
	jmp ErrorEnd
GetByte2:
	pop w[LastChar]
	clc
	ret

; *** Wandelt Text in Word um
GetWord:push w[lastchar]
	call GetWord2
	jc GetByteErr2
	pop w[Lastchar]
	clc
	ret

; *******************************************
; *** Sucht Token zu angegebenem Mnemonic ***
; *******************************************
GetMnemonic:
	call DelSpaces
	push si
	lodsb
	mov dl,al
	lodsw
	mov b[LastChar],ah
	mov bx,ax
	mov si,Mnemonics
	xor cx,cx
GetMnemonic2:
	lodsb
	inc cx
	cmp al,-1
	je NoMnemonicFound
	cmp al,dl
	jne OverJumpWord
	lodsw
	cmp ax,bx
	jne GetMnemonic2
	dec cl
	mov al,cl
	pop si
	add si,3
	clc
	ret
OverJumpWord:
	lodsw
	jmp GetMnemonic2
NoMnemonicFound:
	pop si
	stc
	ret

; ************************************
; *** Gibt Word auf Bildschirm aus ***
; ************************************
PrintWord:
	push bx
	or ax,ax
	jnz WriteOk2
PrintNothing:
	mov al,'0'
	call outchar
	pop bx
	ret
WriteOk2:
	mov si,0
WriteOk3:
	mov bx,10000
	call WriteNibble
	mov bx,1000
	call WriteNibble
	mov bx,100
	call WriteNibble
	mov bx,10
	call WriteNibble
	add al,'0'
	call outchar
	pop bx
	ret

WriteNibble:
	cmp ax,bx
	jnl WriteNibble2
	test ah,128
	je WriteZero
WriteNibble2:
	mov si,1
	xor cx,cx
WDL:	sub ax,bx
	jc WDOk
WDL2:   inc cx
	jmp WDL
WDOK:	add ax,bx
	push ax
	mov ax,cx
	add al,'0'
	call outchar
	pop ax
	ret
WriteZero:
	cmp si,0
	je WriteOk
	push ax
	mov al,'0'
	call outchar
	pop ax
WriteOk:ret

OutChar:cmp bp,1
	je OutChar2
	mov dl,al
	mov ah,6
	int 21h
	ret
OutChar2:
	stosb
	ret

; **********************************
; *** Routinen der Pseudobefehle ***
; **********************************
ORGroutine:
	mov ax,w[ORGindex]      ; Adressindex zurÅcknehmen
	sub w[CodePointer],ax
	call Delspaces
	cmp al,","
	je getORGindex
	call GetWord
	mov w[CodePointer],ax
	cmp w[FirstORG],0
	jne ORG2
	mov w[FirstORG],ax
	mov w[ORGindex],0
ORG2:	call Delspaces
	cmp al,","
	je GetORGindex
	jmp assemble

getORGindex:
	inc si
	call Getword
	push ax
	sub ax,w[CodePointer]
	mov w[ORGindex],ax
	pop w[CodePointer]
	jmp assemble

SYSroutine:
	call GetWord
	mov w[SysAdr],ax
	mov ax,80eh
	mov w[FirstOrg],ax
	mov w[codePointer],ax
	jmp assemble

EQUequal:inc si
EQUroutine:
	call GetWord
	mov bl,b[Pass]
	cmp bl,2
	je NoEQU
	mov bx,w[Labelpointer]
	mov es,w[LabelSeg]
	sub bx,2
	mov w[es:bx],ax
NoEQU:  jmp assemble

BYTroutine:
	call DelSpaces
	cmp al,'"'
	je ASCroutine2  
BYTroutine2:
	call GetByte
	mov di,w[CodePointer]
	sub di,w[ORGindex]
	mov es,w[CodeSeg]
	stosb
	inc w[codePointer]
	call DelSpaces
	lodsb
	cmp al,','
	je BYTroutine
	jmp assemble

WORroutine:
	call GetWord
	mov di,w[CodePointer]
	sub di,w[ORGindex]
	mov es,w[CodeSeg]
	stosw
	add di,w[ORGindex]
	mov w[codePointer],di
	call DelSpaces
	lodsb
	cmp al,','
	je WORroutine
	jmp assemble

ASCroutine:
	call DelSpaces
	cmp al,'"'
	jne BYTroutine2
ASCroutine2:
	inc si
	mov di,w[CodePointer]
	sub di,w[ORGindex]
	mov es,w[CodeSeg]
ASCLoop:lodsb
	cmp al,'"'
	je ASCEnd
	cmp al,0dh
	je ASCError
	stosb
	jmp ASCLoop

ASCEnd: add di,w[ORGindex]
	mov w[CodePointer],di
	call DelSpaces
	cmp al,","
	jne ASCroutine3
	inc si
	jmp BYTroutine
ASCroutine3:
	jmp assemble
ASCError:
	mov al,15
	jmp ErrorEnd

LSTroutine:
	mov bp,0
	mov bl,b[Pass]
	cmp bl,2
	je LI2
	jmp Assemble
LI2:    call GetWord
	jc LI3
	push si
	push ax
	mov si,line
	mov bp,1
	call Print
	mov ah,6
	mov dl,'='
	int 21h
	pop ax
	mov bp,0
	call PrintWord
	mov dx,TXT_CR
	mov ah,9
	int 21h
	pop si
Li3:    jmp Assemble

EXBroutine:
	call DelSpaces
	cmp al,0dh
	je EXTerrorb
	mov di,EXBname
	push di
	call GetEnvName
	pop dx
	mov ax,3d00h
	int 21h
	jc EXTerror2b
	mov bx,ax
	mov cx,-1
	mov dx,w[CodePointer]
	push ds
	mov ds,w[CodeSeg]
	mov ah,3fh
	int 21h
	pop ds
	add w[CodePointer],ax
	mov ah,3eh
	int 21h
	jmp assemble
EXTerrorB:
	jmp EXTerror
EXTerror2B:
	jmp EXTerror2
EXTroutine:
	call DelSpaces
	cmp al,0dh
	je EXTerror
	mov di,Sourcename
	push di
	call GetEnvName
	pop dx
	mov ax,3d00h
	int 21h
	jc EXTerror2
	push ax         ;Handle retten
	mov bl,b[Pass]
	cmp bl,2
	je NoAdd1
	
	mov bx,ax
	xor cx,cx       ;Grîsse der Datei holen
	xor dx,dx
	mov ax,4202h
	int 21h
	push dx,ax
	xor dx,dx
	mov ax,4200h
	int 21h
	pop ax,dx
	xor dx,dx
	mov bx,100
	div bx
	db 66h
	add w[FileSize],ax
NoAdd1:
	mov ax,w[SourceHandle]
	mov bl,b[EXTdepth];altes Handle in Liste eintragen
	xor bh,bh
	shl bx,3
	add bx,EXTstack
	mov w[bx],ax
	mov ax,w[InputPointer]
	mov w[bx+2],ax
	mov ax,w[InputSize]
	mov w[bx+4],ax
	inc b[EXTdepth]
	mov ax,w[InputSeg]
	add ax,800h
	mov w[InputSeg],ax
	pop ax          ;neues Handle eintragen
	mov w[SourceHandle],ax
	call ReadSourceBlock
	jmp assemble    ;die nÑchste Zeile wird nun 
			;aus der neuen Datei gelesen
EXTerror:       ;es wurde kein Dateiname angegeben
	mov al,18
	jmp ErrorEnd
EXTerror2:      ;Datei konnte nicht geîffnet werden
	mov al,19
	jmp ErrorEnd

MACroutine:
jmp IFroutine
	cmp b[MacroMode],0;keine Makros in einem Makro !!!
	jne MacroErr2
	cmp b[pass],2
	je DontStore ;bei Pass 2 Macrodefinition Åberspringen lassen

	; Endung des Labels in 2 umÑndern
	push w[LabelSeg],w[LabelPointer]
	pop bx,es
	sub bx,3
	mov b[es:bx],2
	; Position des Makros in Label speichern
	push w[MacroSeg],w[macropointer]
	pop di
	inc bx
	mov w[es:bx],di
	pop es
	
	; öbergabevariablen in Tabelle speichern
	; Mnemonics: BYT=58 WOR=59
NextVar:call DelSpaces  ;Variablen vorhanden ?
	cmp al,0dh
	je NoVars       ;Nein, Kennzeichnen
	call GetMnemonic ;Variableart feststellen
	cmp al,58
	jl MacroErr	;falsche Angabe,Ende der Variablen & Fehler
	cmp al,59
	jg MacroErr
	sub al,57
	mov bl,b[Pass]
	cmp bl,2
	je NextVar
	stosb           ;Variablenart eintragen
	jmp NextVar     ;und nÑchste Variable feststellen
NoVars: xor al,al       ;Ende der Variablen eintragen
	stosb

	; Pointer speichern, Makromodus einschalten und weitermachen
	mov w[MacroPointer],di
	mov es,cs
	mov ax,w[CodePointer] ;Codezeiger zur Berechnung relativer
	mov w[MacroAdress],ax ;Labeladressen mi·brauchen
	mov b[MacroWrite],1
DontStore:        
	mov b[MacroMode],1
	jmp assemble
MacroErr:
	mov al,20
MacroErr2b:
	mov ah,b[Pass]  ;In Pass 1 keine Fehlermeldung
	cmp ah,1
	je NoVars
	jmp ErrorEnd
MacroErr2:
	mov al,22
	jmp MacroErr2b

; *** Kennzeichnet das Ende des Eintrags
; *** und schaltet Makromodus aus
ENMroutine:
jmp IFroutine
	push w[MacroSeg],w[MacroPointer]
	pop di,es
	xor al,al
	stosb
	mov b[MacroMode],al
	mov b[MacroWrite],al
	push cs
	pop es
	mov ax,w[MacroAdress] ;Codezeiger wiederherstellen
	mov w[CodePointer],ax
	jmp assemble

IFroutine:
ENDroutine:
ELSroutine:
ORroutine:
NOTRoutine:
OLDroutine:
NEWroutine:
	mov al,14
	jmp ErrorEnd

; *************
; *** Texte ***
; *************
TXT_Standard:
	db 218,78 dup 196,191
	#if english
	db 179,'  Genesis 65XX V2.2 - This is free software and a part of the TMA project     ',179
	#else
	db 179," Genesis 65XX V2.2 - Dies ist freie Software und ein Teil des TMA projectes.  ",179
	#endif
	db 192,78 dup 196,217,'$'
TXT_Ok: #if english
	db'                           Assembly was successful.',0ah,0dh
	#else
	db"                        Assemblierung war erfolgreich.",10,13
	#endif

TXT_line:
	db 80 dup 196,"$"

TXT_errors:
	db" errors occured. No file created.",10,13,"$"
	#else
	db" Fehler traten auf. Assemblierung wurde abgebrochen.",10,13,"$"
	#endif

#if english
TXT_proceed:
	db '% done, $'
TXT_proceed2:
	db'. pass   ',0dh,'$'
TXT_tapemode:
	db'Destination is a tape file. ',0ah,0dh,'$'
txt_SysMode:
	db'SYS line has been inserted.',0ah,0dh,'$'
TXT_labels:
	db" labels / $"
TXT_lines:
	db" lines assembled.$"
TXT_size:
	db" Code size : $"
TXT_bytes:
	db" bytes.",10,13,"$"
TXT_Sorry:
	db'Sorry, $'
#endif
#if !english
TXT_proceed:
	db"% fertig, $"
TXT_proceed2:
	db". Pass   ",13,"$"
TXT_tapemode:
	db"Das Ziel ist eine Banddatei.",10,13,"$"
txt_SysMode:
	db"SYS-Kommando wurde eingefÅgt.",10,13,"$"
TXT_labels:
	db" Labels / $"
TXT_lines:
	db" Zeilen assembliert.$"
TXT_size:
	db" CodelÑnge : $"
TXT_bytes:
	db" Bytes.",10,13,"$"
TXT_Sorry:
	db"Fehler: $"
#endif

TXT_CR: db 0ah,0dh,'$'
ERR_NoName:
	db 80 dup 196
	#if english
	db'See documentation file ENGLISH.DOC for permissions or restrictions.',0ah,0dh
	db 80 dup 196
	db'Sorry, you',27h,'ll have to type in source and destination. For example',0ah,0dh
	#else
	db"Weitere Informationen finden sie in der Datei DEUTSCH.DOC.",10,13
	db 80 dup 196
	db"Fehler: Sie mÅssen den Quell- und Zielnamen angeben. Zum Beispiel:",10,13
	#endif
	db'GENESIS PROG.ASM PROG.BIN',0ah,0dh
	db 80 dup 196,"$"

ERR_SourceNotOpen:
	#if english
	db'Sorry, I wasn',27h,'t able to open your sourcefile.',0ah,0dh,'$'
	#else
	db"Fehler: Ich konnte Ihre Quelldatei nicht finden.",10,13,"$"
	#endif

; ******************************************
; *** Liste der Mnemonics und Direktiven ***
; ******************************************
Mnemonics:
	db'ADCANDASL'
	db'BCCBCSBEQBITBMIBNEBPLBRKBVCBVS'
	db'CLCCLDCLICLVCMPCPXCPY'
	db'DECDEXDEY'
	db'EOR'
	db'INCINXINY'
	db'JMPJSR'
	db'LDALDXLDYLSR'
	db'NOP'
	db'ORA'
	db'PHAPHPPLAPLP'
	db'ROLRORRTIRTS'
	db'SBCSECSEDSEISTASTXSTY'
	db'TAXTAYTSXTXATXSTYA'
	; Undokumentierte Befehle
	db"AAXARRASRATSAXSAX2AY2"
	db"DCP"
	db"HLT"
	db"ISB"
	db"LAX"
	db"MAX"
	db"RLARRA"
	db"SLOSPMSRESX0"
	db"XAM"
	; Direktiven
	db'ORGEQUBYTWORASCMACENM#IFELSENDNOT#OREXTLST'
	db'SYSEXB',-1

; *********************************************
; *** interne Beschreibung der 65XX-Befehle ***
; *********************************************
; Aufbau: Legale Adressierungen,Base code
Descriptions:
	db -1,61h;ADC
	db -1,21h;AND
	db accu+zeropage+zeropageX+Absolute+AbsoluteX,02h;ASL
	db indirect,70h;BCC
	db indirect,090h;BCS
	db indirect,0d0h;BEQ
	db zeropage+Absolute,20h;BIT
	db indirect,10h;BMI
	db indirect,0b0h;BNE
	db indirect,0f0h;BPL
	db accu,-8;BRK
	db indirect,30h;BVC
	db indirect,50h;BVS
	db accu,10h;CLC
	db accu,0d0h;CLD
	db accu,50h;CLI
	db accu,0b0h;CLV
	db -1,0c1h;CMP
	db immediate+zeropage+absolute,0e0h;CPX
	db immediate+zeropage+absolute,0c0h;CPY
	db zeropage+zeropagex+absolute+absolutex,0c2h;DEC
	db accu,0c2h;DEX
	db accu,80h;DEY
	db -1,41h;EOR
	db zeropage+zeropagex+absolute+absolutex,0e2h;INC
	db accu,0e0h;INX
	db accu,0c0h;INY
	db indirect+absolute,40h;JMP
	db absolute,14h;JSR
	db -1,0a1h;LDA
	db immediate+zeropage+zeropagex+absolute+absolutey,0a2h;LDX
	db immediate+zeropage+zeropagex+absolute+absolutex,0a0h;LDY
	db accu+zeropage+zeropagex+absolute+absolutex,42h;LSR
	db accu,0e2h;NOP
	db -1,01;ORA
	db accu,40h;PHA
	db accu,00h;PHP
	db accu,60h;PLA
	db accu,20h;PLP
	db accu+zeropage+zeropagex+absolute+absolutex,22h;ROL
	db accu+zeropage+zeropagex+absolute+absolutex,62h;ROR
	db accu,40h-8;RTI
	db accu,60h-8;RTS
	db -1,0e1h;SBC
	db accu,30h;SEC
	db accu,0f0h;SED
	db accu,70h;SEI
	db -2,81h;STA
	db zeropage+zeropagex+absolute,82h;STX
	db zeropage+zeropagex+absolute,80h;STY
	db accu,0a2h;TAX
	db accu,0a0h;TAY
	db accu,0b2h;TSX
	db accu,082h;TXA
	db accu,092h;TXS
	db accu,090h;TYA

	; Undokumentierte Befehle
	db zeropage+absolute+zeropagex,87h-AMzeropage;AAX
	db accu,6bh;ARR
	db accu,4bh;ASR
	db accu,0bh;ATS
	db absolutey,9bh-AMabsolutey;AXS
	db indirectxy+absolutey,93h-AMindirecty;AX2
	db 0,0;AY2
	db uall,0c3h-ucor;DCP
	db accu,02h;HLT
	db uall,0e8h-ucor;ISB
	db uall,0a3h-ucor;LAX
	db accu,8bh;MAX
	db uall,23h-ucor;RLA
	db uall,63h-ucor;RRA
	db uall,03h-ucor;SLO
	db absolutey,0bbh-AMabsolutey;SPM
	db uall,43h-ucor;SRE
	db indirectXY,83h-AMindirectX;SX0
	db accu,0cbh;XAM

; *******************************************
; *** Adressen fÅr Routinen der Direktive ***
; *******************************************
DirTab: dw ORGroutine,EQUroutine,BYTroutine,WORroutine,ASCroutine
	dw MACroutine,ENMroutine,IFroutine,ELSroutine,ENDroutine
	dw NOTroutine,ORroutine,EXTroutine,LSTroutine,SYSroutine
	dw EXBroutine

; ****************************
; *** Daten fÅr Zieldatei ****
; ****************************
; (SYS-Befehl)
SysAdr:dw 0
SysLine:db 13,8,0,0,9eh
SysChar:db'     ',0,0,0

; (Header fÅr T64-Datei)
TapeCode:
	db"C64S tape file by Genesis v2.3s",1ah
	dw 0100h        ;Bandversion
	dw 1eh          ;Anzahl EintrÑge
	dw 0            ;Anzahl benutzter EintrÑge
	dw 0            ;Frei
	db'GENESIS 2.3 CREATED TAPE'
tapeentrytype:
	db 1            ;normal
	db 44h          ;Typ der Datei
tapestart:
	dw 0801h        ;Start des Programmes
tapeend:
	dw 0c3c6h	;Ende des Programmes
	db 0,0		;frei
	dd 400h         ;Offset des Dateiinhalts in der T64-Datei
	db 4 dup 0      ;frei
TapeName:
	db'GENESIS 2.3 FILE'

NME_tmp:db"deleteme.now",0

; *******************
; *** Fehlertexte ***
; *******************
ErrorMessages:
#if english
	db'indexing indirect only works with the X register',0;0
	db'indirect indexing only works with the Y register',0;1
	db'bad adressing syntax',0;2
	db'I think you forgot the remark sign ";"',0;3
	db'this mnemonic requires other adressing modes',0;4
	db'this mnemonic requires no operand',0;5
	db'this mnemonic requires an operand',0;6
	db'you didn',27h,'t finish this operand',0;7
	db'what does THAT mean ? :',0;8
	db'this constant is too large',0;9
	db'this adressing mode requires a byte not word',0;10
	db'this label is double',0;11
	db'this label is not defined',0;12
	db'jump got out of range (>128)',0;13
	db'you can only use this directive in the registered version',0;14
	db'you forgot to end the string',0;15
	db'you just forgot to use one or two chars or to end the string',0;16
	db'you can only use the Y register for indexing on a LDX/STX instruction',0;17
	db'EXT/EXB needs a filename',0;18
	db'I wasn',27h,'t able to open the include file',0;19
	db'the macro definition only allows BYT or WOR mnemonics f.e. (MAC BYT WOR)',0;20
	db'I',27h,'m missing the .EM-instruction',0;21
	db'you can',27h,'t use macros in a macro',0;22
	db'you can',27h,'t use mnemonics in an operand',0;23
	db"this constant is not valid",0;24
errorOccurred:
	db"Source file has been backuped, error messages have been inserted.",10,13,"$"
errortext:
	db"Error :",0
leavein:db"Error messages will be removed if you leave this line in !",0
removing:
	db"Removing error messages...$"
#else
	db"Indiziert indirekte Adressierung klappt nur mit dem X-Register",0
	db"Indirekt indizierte Adressierung klappt nur mit dem Y-Register",0
	db"Unbekannte Adressierung",0
	db'Sie verga·en das Kommentar-Zeichen ";"',0
	db"Dieses Mnemonic benîtigt eine andere Adressierung",0
	db"Dieses Mnemonic benîtigt keinen Operanden",0
	db"Dieses Mnemonic benîtigt einen Operanden",0
	db"Dieser Operand ist nicht vollstÑndig",0
	db"Was soll das heissen ?!?",0
	db"Diese Konstante ist zu gross",0
	db"Diese Adressierung benîtigt ein Byte, kein Wort",0
	db"Dieses Label kommt doppelt vor",0
	db"Dieses Label ist nicht definiert worden",0
	db"Sprung kann sein Ziel nicht erreichen (>128)",0
	db"Sie kînnen dieses Direktiv nur in der registrierten Version von",10,13
	db"Genesis benutzen",0
	db"Dieser String hat keine Endung",0
	db"Sie verga·en ein oder zwei Buchstaben oder den String zu beenden",0
	db"Sie kînnen nur das Y-Register fÅr ein Indizierung mit LDX/LDY-Befehlen benutzen",0
	db"Das EXT/EXB-Direktiv benîtigt einen Dateinamen",0
	db"Ich konnte die Include-Datei nicht finden",0
	db"Die Makro-Definition erlaubt nur BYT oder WOR mnemonics z.B. (MAC BYT WOR)",0
	db"Ich vermisse das #EM-Direktiv",0
	db"Es kînnen keine Makros in einem Makro benutzt werden",0
	db"Es dÅrfen keine Mnemonics in einem Operanden benutzt werden",0
	db"Diese Konstante ist fehlerhaft",0
errorOccurred:
	db"Die Quelldatei wurde gesichert und Fehlermeldungen eingesetzt.",10,13,"$"
errortext:
	db"Fehler :",0
LeaveIn:db"Fehlermeldungen werden entfernt, wenn Sie die Zeile beibehalten !",0
removing:
	db"Entferne Fehlermeldungen...$"
#endif

TXT_fill:
#if english
	db'What about letting Genesis 6510 V2.3 register ? ',0
#else
	db"Wie wÑr's mit einer Registrierung von Genesis ? Sie erhalten dafÅr die "
	db"Version 3.0 mit Makros und Fehlermeldungen im Quellcode !"
#endif
	db 16 dup 20h,0

	include "..\..\..\lib\io\doserrmn.inc"
#if english
	include "..\..\..\lib\io\doserruk.inc"
#else
	include "..\..\..\lib\io\doserrge.inc"
#endif

MacSize:dw -1

; ********************
; *** Datenbereich ***
; ********************
	.data
even
DataStart:
;errorpointer:dw ?
;errorlist:dd ? dup 1000
nothing:db 16 dup 0
EXBname:db 64 dup 0
Pass:dw 0
Lines:dw 0,0
Labels:dw 0
Labelseg:dw 0
LabelPointer:dw 0
FirstORG:dw 0
CodeSeg:dw 0
CodePointer:dw 0
ORGindex:dw 0
InputSeg:dw 0
InputPointer:dw 0
InputSize:dw 0
MacroSeg:dw 0
MacroPointer:dw 0
MacroMode:dw 0
MacroWrite:dw 0
MacroAdress:dw 0
MacroWordTab:dw 16 dup 0
MacInsAdr:dw 0
Mnemonic:dw 0
Op:dw 0,0
OpMode:dw 0
OpSize:dw 0
OpCode:dw 0
OpText:dw 0
OpAttr:dw 0
Error:dw 0
LastChar:dw 0;der letzte Buchstabe eines Mnemonics
CompMode:dw 0
TapeMode:dw 0
filesize:dw 0,0
Proceed:dw 0,0
SourceLength:dw 0
DestLength:dw 0
SourceHandle:dw 0
DestHandle:dw 0
SourceName:db 64 dup 0
DestName:db 64 dup 0
Line:db 256 dup 0
FileExtension:db 0,0,0,0
ExtDepth:dw 0
ExtStack:dw 0;Stack fÅr Handles
DataEnd:
; Makrospeicherung :
;
; Makronamen enden in der Labeltabelle mit einer 2
; Hinter dem Namen steht anstatt einer normalen Adresse
; ein Zeiger auf den Quelltext im Makrospeicher.
; Labels eines Makros enden in der Tabelle mit einer 1
; und werden nur Åbergeben wenn MacroMode=1 ist.
; Makrolabels Åbertragen nur die relative Adresse zum Anfang des
; Makros

