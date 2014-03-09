; ********************************************************
; *** Routinen fÅr das Abspielen von Amiga MOD-Dateien ***
; ********************************************************
; SoundBlaster Pro

MODmixLen=(MODresolution/60) and -2 ;Framegrîsse immer gerade
MODhelp=32000/MODmixLen
MODbufferLen=MODhelp*MODmixLen	   ;PufferlÑnge
MODframes=MODbufferLen/MODmixLen   ;Anzahl der Frames in einem Teilpuffer

; *****************************
; *** Zeiger und Strulturen ***
; *****************************
MODpatList=3b8h
MODpatBase=43ch                    ;Anfang der Patterns
MODsampleList=2ah                  ;Anfang der MODsampleListe
SampleInfoSize=30
ChaninfoSize=32

struc
        samplelength:   dw 0
        dontknow1:      db 0
        samplevolume:   db 0
        loopstart:      dw 0
        loopend:        dw 0
ends

struc
        sampleadr:      dd 0      ;aktuelle
        samplerest:     dd 0
        sampleLoopAdr:  dd 0
        sampleLoopLen:  dd 0
        samplestep:     dw 0
        samplemod:      dw 0    ;Rest von Zooming
        instr:          dw 0
        fxcommand:      db 0
        fxcontrol:      db 0
        volume:         db 0,0,0 ;1./2.Divisor,Addition
ends

db"DWMOD (c)1994-1995 Sven Michael Klose/Deepware"

; ******************************
; *** Interruptroutine IRQ 7 ***
; ******************************
SBirq:
        push ax,bx,dx,ds
        mov ds,cs
        mov dx,w[SBport]	;Datum von DSP einlesen
        or dl,0eh
        in al,dx
        ;
        mov b[IRQflag],MODframes ;Flag fÅr das Mischen eines Blocks
        mov ax,w[MODoldplayadr]
        add ax,MODbufferLen     ;NÑchste Abspieladresse
        mov w[PlayAdr],ax
        mov w[MODoldPlayAdr],ax
        mov al,20h              ;EOI an Interruptcontroller
        out 20h,al
        cmp b[SBIRQNR],10	;IRQ 10 ?
        jne NoIRQ7
        out 0a0h,al
NoIRQ7: pop ds,dx,bx,ax
        iret

  
; ***********************************
; *** Frameroutine fÅr MOD-Player ***
; ***********************************
MODframe:
        push ds,es
        pusha
        push cs
        pop ds
        mov al,b[Irqflag]     ;Fehlen noch Blîcke ?
        or al,al
        jnz MOD2              ;Ja,nÑchsten Block mischen
        popa
        pop es,ds
        ret

MOD2:   dec b[MODspeedC]      ;Neue Zeile holen ?
        jz MODline
        mov bp,ChannelInfos   ;Nein,normal mischen...
        mov cx,4
MODm:   push cx,ds
        call MixChannels
        pop ds,cx
        add bp,ChanInfoSize
        loop MODm
        jmp MODend

ModLine:
        mov ds,cs
        mov ax,w[MODseg]      ;Basisadresse des Moduls
        mov es,ax
        dec b[MODpatcnt]      ;Ende des Patterns ?
        mov al,b[MODpatcnt]
        or al,al
        jnz MODnonewpat       ;Nein,weiter
        mov al,64             ;Ja,ZeilenzÑhler zurÅcksetzen
        mov b[MODpatcnt],al
        db 66h
        xor bx,bx
        mov bx,w[MODpatpos]   ;neue Patternadresse holen
        inc w[MODpatpos]
        mov bl,b[es:bx]       ;Patternnummer holen
        xor bh,bh
        db 66h
        shl bx,10             ;* 1024
        db 66h
        add bx,MODpatBase
        dw 0
        db 66h
        mov w[MODpatadr],bx   ;Patternadresse speichern

MODnonewpat:
        db 66h
        mov si,w[MODpatadr]   ;Aktuelle Zeilenadresse holen
        mov bp,ChannelInfos   ;Index fÅr Channelinfos
        mov ds,es
        mov cx,4              ;4 KanÑle mischen
MODloop1:
        push cx
        db 66h,67h
        lodsw                 ;Ton holen
        db 66h
        push si
        push ds
        mov ds,cs
        call MODGetVoice
        call MODgetFX
        call MixChannels
        pop ds
        db 66h
        pop si
        pop cx
        add bp,Chaninfosize
        loop MODloop1
        
        db 66h
        mov w[cs:MODpatadr],si        ;Neue Zeilenadresse speichern
        mov al,b[cs:MODspeed]
        mov b[cs:MODspeedC],al
MODend:
        add w[cs:Playadr],MODmixLen
        dec b[cs:IRQflag]             ;Flag fÅr neue Zeile lîschen
        popa
        pop es,ds
        ret

; *********************************************
; *** Note und Instrument aus Pattern holen ***
; ***             und umrechnen             ***
; *********************************************
MODgevo2b:
        jmp MODgevo2
MODgetvoice:
        db 66h
        push ax               ;Eintrag retten
        db 66h
        mov bx,ax             ;wert retten fÅr zooming
        mov cl,al
        db 66h
        shr ax,8              ;Samplenummmer holen
        xchg al,ah
        shr ax,4
        and al,0001111b
        and cl,10000b
        or al,cl
        jz MODgevo2b          ;Kein Ton da, Åberspringen
        push bx
        dec al
        xor ah,ah
        mov w[bp+instr],ax
        push ax
        shl ax,2              ;adresse des Samples holen
        mov bx,ax
        add bx,sampletab
        db 66h
        mov ax,w[bx]
        db 66h
        mov w[bp+sampleadr],ax
        pop ax
        shl ax,5              ;samplelÑnge holen
        mov bx,ax
        add bx,SampleInfos
        db 66h
        xor ax,ax
        db 66h
        xor cx,cx
        mov ax,w[bx+loopend]
        mov cx,w[bx+loopstart]
        db 66h
        add ax,cx
        db 66h
        mov cx,ax
        db 66h
        shr cx,2
        db 66h
        or cx,cx
        jnz LengthIsOk
        mov ax,w[bx]
LengthIsOk:
        db 66h
        shl ax,1                      ;samplelÑnge korrigieren
        db 66h
        mov w[bp+samplerest],ax
        mov ax,w[bx+loopstart]
        mov w[sampleloopS],ax         ;loopdaten Åbertragen
        push ax
        mov ax,w[bx+loopend]
        mov w[sampleloopE],ax
        mov w[bp+volume],8            ;LautstÑrke =voll
        mov b[bp+volume+2],0
        mov w[bp+SampleMOD],0
        db 66h
        xor ax,ax                     ;Loopadresse berechnen
        pop ax
        db 66h
        shl ax,1
        db 66h
        add ax,w[bp+sampleadr]
        db 66h
        mov w[bp+sampleloopadr],ax

        db 66h                        ;LooplÑnge berechnen
        xor ax,ax
        db 66h
        xor bx,bx
        mov ax,w[sampleloopE]
        mov bx,w[sampleloopS]
        db 66h
        shl ax,1

        db 66h
        mov w[bp+samplelooplen],ax
  
        db 66h
        xor bx,bx
        pop bx                ;wert fÅr kommando wiederholen
        xchg bl,bh
        shr bx,1              ;Notenwert holen
        and bx,07ffh
        push bx
        db 66h
        xor bx,bx
        pop bx
  
        mov ax,001bh          ;Referenzwert fÅr Frequenz
        db 66h
        shl ax,16
        mov ax,5678h
        db 66h
        xor dx,dx
        db 66h
        div bx

        db 66h
        xor bx,bx
        mov bx,MODresolution
        db 66h                ;Genauigkeit erhîhen
        shl ax,16
        db 66h                ;zooming berechnen
        xor dx,dx
        db 66h
        div bx

        mov w[bp+SampleStep],ax
  
MODgevo2:
        db 66h
        pop ax
        ret

; **************************
; *** Bearbeitet Effekte ***
; **************************
MODgetfx:
        db 66h
        shr ax,16
        and al,0fh            ;kommando ausmaskieren
        mov b[bp+fxcommand],al
        mov b[bp+fxcontrol],ah
        cmp al,06h            ;Sample Cut ?
        jne NoLenCut
        mov w[bp+samplerest],MODbufferLen
        mov w[bp+samplelooplen],0
        ret
NoLenCut:
        cmp al,0ch            ;LautstÑrke
        jne NoVol
        cmp ah,40h
        jne nofullvol
        mov w[bp+volume],8
        mov b[bp+volume+2],0
        jmp novol
NoFullVol:
        mov w[bp+volume],808h ;Schon mal auf 0 stellen
        mov al,20h
        mov bh,ah             ;Wert zum Aufrunden
        push bp
        xor dl,dl             ;ZÑhler der Verschiebungen
        mov dh,1              ;Anzahl der Verschiebungen
        mov cx,5
vol:    test ah,al            ;Bit testen
        jz novolbit           
        sub bh,al             ;Verschiebung berechnen
        mov b[bp+volume],dh
        inc bp                ;nÑchste Position
        inc dl
        cmp dl,2
        je volend
novolbit:
        shr al,1
        inc dh
        loop vol
volend:
        or bh,bh              
        jz NoSHinc            ;nicht aufrunden
        dec b[bp+volume-1]
NoShinc:
        pop bp
        xor bl,bl
        mov cl,b[bp+volume]
        mov al,40h
        shr al,cl
        or bl,al
        mov cl,b[bp+volume+1]
        mov al,40h
        shr al,cl
        or bl,al
        mov al,40h
        sub al,bl
        shr al,1
        mov b[bp+volume+2],al
        ret

NoVol:  cmp al,0dh                    ;Pattern-Cut ?
        jne NoPatCut
        mov b[MODpatcnt],1
        ret
NoPatCut:
        cmp al,0ah                    ;Sample-Cut ?
        jnz NoSamCut
        mov w[bp+samplelooplen],0
        mov bl,ah
        xor bh,bh
        shl bx,1 ;shr bx,2
        xor dx,dx
        mov ax,MODbufferLen
        or bx,bx
        jz NoFx
        div bx
SamCut2:mov w[bp+samplerest],ax
        ret

NoSamCut:
        cmp al,0fh                    ;Speed ?
        jne NoSpeed
        and ah,0fh
        mov al,ah
        shr al,2
        add al,ah
        mov b[MODspeed],al
        ret
NoSpeed:cmp al,0bh                    ;Order jump ?
        jne NoOrder
        mov al,ah
        xor ah,ah
        push ax
        db 66h
        xor ax,ax
        pop ax
        db 66h
        add ax,MODpatList
        dw 0
        db 66h
        mov w[MODpatpos],ax
        mov b[MODpatcnt],1            ;(aktuelles Pattern abbrechen)
        ret
NoOrder:
NoFX:
        ret

; **********************************************************************
; ***********************************
; *** Mischt KanÑle in den Puffer ***
; ***********************************
MixChannels:
        mov ds,cs
        mov es,w[bufferseg]
        mov di,w[PlayAdr]
        db 66h
        mov cx,MODmixLen              ;Grî·e des zu mischenden Abschnittes
        dw 0
        mov dx,cx     
Mich5:  db 66h
        mov ax,w[bp+samplerest]       ;Testen ob Åberhaupt ein Sample anliegt
        db 66h
        or ax,ax
        je LoopCheck                  ;Nein, auf Loop testen
        
        push dx                       ;AX=CX*steps (richtige SamplelÑnge)
        db 66h
        mov ax,cx
        db 66h
        xor bx,bx
        mov bx,w[bp+samplestep]
        db 66h
        xor dx,dx
        db 66h
        mul bx
        pop dx
        db 66h
        shr ax,16
        db 66h
        sub ax,w[bp+samplerest]       ;genug sample da ?
        jc MiCh3                      ;Ja,kopieren
        
        db 66h
        mov ax,w[bp+samplerest]       ;Nein,Rest vom Sample spielen
        db 66h
        shl ax,16                     ;Rest des Samples/steps=LÑnge im Puffer (cx)
        push dx
        db 66h
        xor dx,dx
        db 66h
        xor bx,bx
        mov bx,w[bp+samplestep]
        db 66h
        div bx
        mov cx,ax
        pop dx

Mich3:
        push cx,dx
        db 66h
        mov si,w[bp+Sampleadr]        ;sampleadresse
        call Mixit
        db 66h                        ;Gespielte LÑnge des SamplestÅcks berechnen
        mov ax,si
        db 66h
        sub ax,w[bp+SampleAdr]
        db 66h                        ;RestlÑnge berechnen
        sub w[bp+samplerest],ax
        db 66h
        mov w[bp+SampleAdr],si
        pop dx,cx
        sub dx,cx                     ;RestlÑnge berechnen
        mov cx,dx

ClearBuffer:
        or cx,cx                      ;Ist der Puffer voll ?
        jz Mixed                      ;Ja,fertig
LoopCheck:
        db 66h                        ;Ist eine Schleife vorhanden ?
        mov ax,w[bp+samplelooplen]
        db 66h
        shr ax,2
        db 66h
        or ax,ax
        jz ClBu2                      ;Nein, Rest auffÅllen
        db 66h
        mov ax,w[bp+sampleloopadr]    ;Adresse und LÑnge des Loops Åbertragen
        db 66h
        mov w[bp+sampleadr],ax        ;ins Sampleinfo
        db 66h
        mov ax,w[bp+samplelooplen]
        db 66h
        mov w[bp+Samplerest],ax
        jmp Mich5                     ;und weitermischen

; *********************************************
; *** Lîscht Rest des Puffers bei 1. Kanal, ***
; *** oder addiert Mindestwerte zum Puffer  ***
; *********************************************
ClBu2:  mov ds,w[bufferseg]
        cmp bp,Channelinfos
        jne soothe
        mov al,20h
        rep stosb
Mixed:  ret

Soothe: add b[di],20h
        db 66h
        inc di
        loop Soothe
        ret

; ****************************************************
; *** Kopiert (1.Kanal) oder addiert (2.-4.Kanal)  ***
; *** zum DMA-Puffer (inklusive LautstÑrkeregelung ***
; ****************************************************
MixIt:  cmp bp,ChannelInfos
        jne MiCh2
        call Copy2buffer
        ret
MiCh2:  call Mix2buffer
        ret

Copy2buffer:
        mov bx,w[bp+SampleMOD]
        mov dx,w[bp+SampleStep]
        mov ax,w[bp+volume]
        mov b[v2+2],al
        mov b[v2b+2],ah
        mov al,b[bp+Volume+2]
        mov b[v2a+1],al
        push ds
        mov ds,w[MODseg]

Co2BuQ2:db 67h,8ah,06h          ;mov al,b[esi]
        mov ah,al               ;LautstÑrke regeln
v2:     shr al,0
v2b:    shr ah,0
v2a:    add al,0        
        add al,ah
        stosb                   ;Wert in den Puffer schreiben
        add bx,dx               ;BX erhîhen bei öbertrag
        db 66h
        db 83h,0d6h,0           ;adc si,0
        loop Co2buQ2            ;Weiter bis Puffer gefÅllt ist
        
        pop ds
        mov w[bp+sampleMOD],bx
        ret

Mix2Buffer:
        mov bx,w[bp+SampleMOD]
        mov dx,w[bp+SampleStep]
  
        mov ax,w[bp+volume]
        mov b[v4+2],al
        mov b[v4b+2],ah
        mov al,b[bp+Volume+2]
        mov b[v4a+1],al
        push ds
        mov ds,w[MODseg]

Mi2BuQ2:db 67h,8ah,06h          ;mov al,b[esi]
        mov ah,al
v4:     shr al,0
v4b:    shr ah,0
v4a:    add al,0
        add al,ah
        add b[es:di],al
        inc di
        add bx,dx
        db 66h
        db 83h,0d6h,0           ;adc si,0
        loop Mi2buQ2
  
        pop ds
        mov w[bp+SampleMOD],bx
        ret

; ****************************************************
; *** MOD-Datei laden und Variablen initialisieren ***
; ****************************************************
initMOD:push ds,es
        mov ds,cs
        mov es,cs
        mov di,MODl1            ;Tabelle lîschen
        mov cx,MODl2-MODl1
        xor al,al
        rep stosb

        mov es,w[Newseg]
        db 66h
        xor ax,ax
        mov ax,es
        mov w[MODseg],ax
        db 66h
        shl ax,4
        db 66h
        mov w[FileBase],ax
        mov dx,si
        call MODloadFile
        mov si,MODpatList
        mov ds,w[cs:MODseg]
        mov cx,128
        db 66h
        xor ax,ax
CountPatterns:
        db 67h
        lodsb
        cmp al,ah
        jle NoNewerPattern
        mov ah,al
NoNewerPattern:
        loop CountPatterns
        mov al,ah
        xor ah,ah
        inc ax
        db 66h
        shl ax,10             ;1 Pattern=1024 bytes
        db 66h
        add ax,MODpatBase
        dw 0
        db 66h
        mov w[cs:samplebase],ax ;adressen der sampledaten
  
        db 66h
        push ax

        db 66h
        mov dx,ax
        mov bp,SampleTab      ;Adressen der Samples
        mov si,MODsampleList
        mov di,SampleInfos
        mov es,cs
        mov cx,31             ;31 Sampledaten Åbertragen
CopySampleInfo:
        push cx
        db 66h
        mov w[cs:bp],dx       ;lineare Sampleadresse speichern
        add bp,4
  
        db 66h
        xor ax,ax
        lodsw                 ;samplelÑnge auf intel-notation
        xchg al,ah
        stosw
  
        db 66h
        shl ax,1              ;SamplelÑnge*2
        db 66h
        add dx,ax             ;und neue Position ist berechnet
        movsw                 ;byte X und lautstÑrke
  
        db 66h
        xor ax,ax
        lodsw                 ;Loopanfang auf intel-notation
        xchg al,ah
        stosw
  
        lodsw                 ;loopende auf intel-notation
        xchg al,ah
        stosw
  
        mov cx,6              ;rest kopieren
        db 66h
        rep movsw
        pop cx
        dec si,2
        loop CopySampleInfo
  
        mov es,ds
  
        db 66h
        pop si                ;1. Sampleadresse holen
        db 66h
        mov di,si
        mov bp,SampleInfos
        mov cx,31
Vol1:   push cx
        db 66h
        xor ax,ax
        mov ax,w[cs:bp+Samplelength]
        or ax,ax
        jz VolSkip
        db 66h
        shl ax,1
        db 66h
        mov cx,ax
        mov bl,b[cs:bp+samplevolume]
        xor bh,bh
Vol2:   db 67h
        lodsb
        add al,128            ;Vorzeichen korrigieren
        xor ah,ah
        xor dx,dx
        mul bx                ;Wert mal LautstÑrke
        mov al,ah             ;/40h(max.Vol)/4(auf 6 Bits runterrechnen)
        db 67h
        stosb
        db 66h
        dec cx
        jnz Vol2
VolSkip:pop cx
        add bp,32
        loop Vol1

        mov ds,cs
        mov es,cs
        mov ax,cs
        mov w[mod_irqseg],ax
        mov w[mod_irqadr],sbirq
        mov w[bufferseg],9000h
        pop es,ds
        ret

; ********************************
; *** Musikausgabe einschalten ***
; ********************************
StartMOD:
        push ds,es
        mov ds,cs
        mov ax,MODbufferLen-1           ;LÑnge des DMA-Puffer vermerken
        mov w[MODinputsize],ax
        mov es,w[BufferSeg]             ;Puffer mit Mittelwert fÅllen
        xor di,di
        mov ax,8080h
        mov cx,8000h
        rep stosw

        mov al,1                        ;Variablen initialisieren
        mov b[MODspeedc],al             ;damit sofort die erste Pattern-
        mov b[MODpatcnt],al             ;zeile ausgelesen wird
        mov b[MODspeed],7
        db 66h        
        mov w[MODpatpos],MODpatList
        dw 0

        mov al,40h                      ;Samplerate einstellen
        call WriteSB
        mov al,high(65536-(256000000/MODresolution))
        call WriteSB
	mov bx,w[MODinputsize]
	mov al,48h			;Blockgrîsse
	call WriteSB
	mov al,bl
	call WriteSB
	mov al,bh
	call WriteSB

	mov al,90h			;Sample spielen (Autoinit)
	call WriteSB

        call SwitchIRQ                  ;Interrupt verbiegen
        call SetDMA                     ;DMA-Kanal anschalten
        call SetIRQ                     ;Interrupt erlauben
        call SpeakerOn
        mov ax,w[Playadr]               ;Startadresse im Puffer festlegen
        mov w[MODoldPlayAdr],ax
SBinitINT:
        int 20h                         ;und 1. Pufferabschnitt spielen
        pop es,ds
        ret

; *******************************
; *** Musikausgabe abschalten ***
; *******************************
StopMOD:push ds,es
        mov ds,cs
        call SpeakerOff
        call SwitchIRQ          ;Interrupt wieder herstellen
        call LockIRQ
        mov al,14h
	out 8,al               ;DMA 1 deaktivieren
	pop es,ds
        ret

; *************************
; *** IRQ Vektor setzen ***
; *************************
SwitchIRQ:
        pusha
        push ds,es
        mov al,b[cs:SBirqNr]    ;Alten IRQ lesen
        add al,8
        mov b[cs:SBinitINT+1],al
        mov ah,35h
        push ax
        int 21h
        mov ds,es
        mov dx,bx
        push ds,w[cs:mod_irqseg]
        pop ds,w[cs:mod_irqseg]
        push dx,w[cs:mod_irqadr]
        pop dx,w[cs:mod_irqadr]
        pop ax
        mov ah,25h
        int 21h
        pop es,ds
        popa
        ret

; ***********************
; *** DMA einschalten ***
; ***********************
SetDMA: mov ds,cs
        mov al,14h
        out 8,al                ;DMA deaktivieren
        mov al,01011000b        ;demand,adress inc.,write,ch.x
        or al,b[SBdmaNr]
        out 0bh,al              ;DMA-Modus fÅr Kanal einstellen
        out 0ch,al              ;reset Flip-Flop
	mov dl,b[SBdmaNr]
	xor dh,dh
	shl dx,1
	inc dx
        mov al,-1
        out dx,al                ;LÑnge des Blocks-1
        out dx,al
        mov al,b[Bufferseg+1]   ;Page
        shr al,4
        out 0ch,al              ;reset Flip-Flop
        out 83h,al
        mov al,10h
        out 8,al                ;DMA aktivieren
        out 0ch,al              ;reset Flip-Flop
	dec dx
        mov ax,w[PlayAdr]
        out dx,al                ;Adresse
        mov al,ah
        out dx,al
        mov al,b[SBdmaNr]
        out 0ah,al              ;Kanal freigeben
        mov al,1101b
        out 0fh,al
        ret
        
DMApages:
	db 87h,83h,81h,82h,8fh,8bh,89h,8ah

; ********************************
; *** Lautsprecher einschalten ***
; ********************************
SpeakerOn:
        mov dx,w[sbport]        ;Filter abschalten
        or dl,4
        mov al,0eh
        out dx,al
        inc dl
        in al,dx
        or al,32
        out dx,al
        mov al,0d1h             ;Lautsprecher einschalten
        jmp WriteSB

; *******************************
; *** Lautsprecher abschalten ***
; *******************************
SpeakerOff:
        mov al,0d0h
        jmp WriteSB

; **********************
; *** PIC einstellen ***
; **********************
SetIRQ: mov ah,0feh
        mov cl,b[cs:SBirqNr]
        cmp cl,7
        jbe SetIRQ2
        sub cl,8
        shl ah,cl
        in al,0a1h
        and al,ah
        out 0a1h,al
        ret

SetIRQ2:in al,21h
        shl ah,cl
        and al,ah
        out 21h,al
        ret

; **********************
; *** IRQ abschalten ***
; **********************
LockIRQ:mov ah,1
        mov cl,b[cs:SBirqNr]
        cmp cl,7
        jbe LockIRQ2
        sub cl,8
        shl ah,cl
        in al,0a1h
        or al,ah
        out 0a1h,al
        ret
LockIRQ2:
        in al,21h               ;IRQ abschalten
        shl ah,cl
        or al,ah
        out 21h,al
        ret

; *******************
; *** Datei laden ***
; *******************
MODloadFile:
        push w[cs:NewSeg]       ;Segment retten zur öbergabe
        mov ax,w[cs:NewSeg]
        push ax
        mov w[cs:MODseg],ax
        mov ax,3d00h            ;Datei îffnen
        int 21h
        jc MODErrorBye
        mov bx,ax               ;Handle Åbergeben
        mov w[SourceHandle],ax
MODloadNew:mov ah,3fh           ;Block einlesen
        mov cx,32768            ;Soviel wie mîglich laden
        push ds                 ;Zielsegment Åbergeben
        mov ds,es
        xor dx,dx               ;Zieladresse
        int 21h                 ;Daten laden
        pop ds
        or ax,ax
        jz MODloaded
        push ax
        db 66h
        xor ax,ax
        pop ax
        db 66h
        add w[filesize],ax
        push ax
        shr ax,4      
        add w[NewSeg],ax
        pop ax
        mov ax,es
        add ax,800h
        mov es,ax
        jmp MODloadNew
MODloaded: mov ah,3eh           ;Datei schliessen
        int 21h
        pop ax                  ;Segmentadresse steht in AX
        pop w[cs:NewSeg]
        ret

MODErrorBye:
        mov ax,3
        int 10h
        mov dx,ERR_MODerr
        mov ah,9
        mov ds,cs
        int 21h
        mov ax,4c01h
        int 21h

Maniac_StartFX:
        ret

ERR_MODerr:
        db"MODPLAY: Can't find file.",10,13,"$"
even
MODl1:
sampleTab:      dd 32 dup 0   ;Adressen der Samples
ChannelInfos:   db ChanInfoSize*32 dup 0
SampleInfos:    db 32*Sampleinfosize dup 0
MODl2:

BufferAdr:      dw 0
BufferSeg:      dw 0
mod_irqadr:     dw 0
mod_irqseg:     dw 0
PlayAdr:        dw 0
MODoldPlayAdr:  dw 0

FileSize:       dd 0
FileBase:       dd 0
samplebase:     dd 0         ;Adresse der Patterns
sampleloopS:    dw 0
sampleloopE:    dw 0
MODpatadr:      dd 0          ;Adresse im Pattern
MODpatpos:      dd 0          ;Adresse in Patternliste
MODseg:         dw 0
MODspeedC:      db 0          ;WartezÑhler
MODpatcnt:      db 0          ;ZeilenzÑhler
IRQflag:        db 0
MODspeed:       db 0
MODinputSize:   dd 0
