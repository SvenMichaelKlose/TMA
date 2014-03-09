; ========================================================
; >>> Routinen fÅr das Abspielen von Amiga MOD-Dateien <<<
; >>> (c)1995-1997 S.Klose			       <<<
; ========================================================

	.code
; =============================
; >>> Zeiger und Strukturen <<<
; =============================
MODsonglength=950
MODpatList=3b8h
MODpatBase=43ch 		   ;Anfang der Patterns
MODsampleList=2ah		   ;Anfang der MODsampleListe
SampleInfoSize=30
MODheadersign=1080

; ================================
; >>> Struktur der Instrumente <<<
; ================================
struc
	samplelength:	dw ?
	samplefinetune: db ?
	samplevolumeA:	db ?
	loopstart:	dw ?
	looplen:	dw ?
ends

; =====================
; >>> Kanalstruktur <<<
; =====================
struc
chaninfostart:
	sampleadr:	dd ?	  ;aktuelle
	samplerest:	dd ?
	sampleLoopAdr:	dd ?
	sampleLoopLen:	dd ?
	sampleoldpriod: dw ?
	samplestep:	dw ?	;Intern zum mischen
	samplemod:	dw ?	; "
	sampleperiod:	dw ?	;periode im pattern
	samplenote:	db ?	;real note
	samplevolume:	db ?
	volume: 	dw ?	;Position in VolumeTab
	volume2:	db ?	;Zwischenspeicher f. Berechnung
	oldinstr:	db ?
	fxcommand:	db ?
	fxparam:	db ?
	finetune:	db ?
	slidedest:	dw ?
	slidespeed:	dw ?
	retrigcnt:	db ?
	glissfunc:	db ?
	VibWaveCtrl:	db ?
	vibratopos:	db ?
	vibratocmd:	db ?
	vibratobase:	dw ?
	TrmWaveCtrl:	db ?
	tremolopos:	db ?
	tremolocmd:	db ?
	tremolobase:	dw ?
	ntdlynote:	dw ?
	ntdlytime:	db ?
	ntdlyinstr:	db ?
	notecut:	db ?
chaninfoend:
ends
chaninfosize=chaninfoend-chaninfostart

even
; =======================================
; >>> Interruptroutine fÅr MOD-Player <<<
; =======================================
MODframe:
	push ds,es
	pushad
	mov ds,cs
	call w[MODtimingcheck]
	jc mod2 	;neuen Abschnitt berechnen
	jmp skipline

MOD2:	mov ax,[DMSoutlen]
	add [DMSmixpos],ax
	add [modoldmixpos],ax
	mov ax,[DMSnewoutlen]
	mov [DMSoutlen],ax
	mov ax,[DMSnewmixlen]
	mov [DMSmixlen],ax
	inc w[playcnt]
	dec b[MODspeedC]      ;Neue Zeile holen ?
	jz MODline
	mov bp,ChannelInfos   ;Nein,normal mischen...
	mov cx,w[MODchannels]
	mov b[actualchannel],0
MODm:	push cx,ds
	call notedelay
	call MODsteadyFX
	call MixChannels
	pop ds,cx
	add bp,ChanInfoSize
	loop MODm
	call MixFXchannels
	jmp MODend

ModLine:mov ax,cs
	mov ds,ax
	cmp b[patterndelay],0
	je nopdly2
	dec b[patterndelay]
	jmp skipline
nopdly2:dec b[MODpatcnt]      ;Ende des Patterns ?
	jnz MODnonewpat       ;Nein,weiter
	mov b[MODpatcnt],64   ;Ja,ZeilenzÑhler zurÅcksetzen
	call GetNextPattern
	mov [MODpatadr],ebx   ;Patternadresse speichern

MODnonewpat:
	cmp b[patloopcnt],0	;pattern loop in process ?
	je nopatinproc
	dec b[patloopcnt]
	jnz nopatinproc
	mov eax,[patstrtadr]
	mov [MODpatadr],eax
	mov eax,[patstrtcnt]
	mov [MODpatcnt],eax
nopatinproc:
	mov esi,[MODpatadr]   ;Aktuelle Zeilenadresse holen
	mov bp,ChannelInfos   ;Index fÅr Channelinfos
	xor ax,ax
	mov ds,ax
	mov cx,w[cs:MODchannels]
	mov b[cs:actualchannel],0
MODloop1:
	push cx
	addr:
	lodsd		      ;Ton holen
	push ds
	mov ds,cs
	push esi		;Zeiger auf nÑchste Spalte retten
	call MODGetVoice	;Instrumente
	call notedelay
	call MODgetFX		;Effekte
sk2:	call MixChannels	;abmischen
	pop esi,ds,cx
	add bp,Chaninfosize
	loop MODloop1
	call MixFXchannels
	
	xor eax,eax
	mov ax,w[cs:MODchannels] ;ZeilenlÑnge
	shl ax,2
	add [cs:MODpatadr],eax	;Neue Zeilenadresse speichern
	mov al,b[cs:MODspeed]
	mov b[cs:MODspeedC],al
MODend:
skipline:
	cs:add w[MODoldinttimer],1000h*182/modintcalls
	jnc >l1
	popad
	pop es,ds
	jmpf [cs:oldtimerV]
l1:	mov al,20h
	out 20h,al
	popad
	pop es,ds
k1:	iret

MixFXchannels:
	mov cx,w[cs:FXchannels]
	jcxz NoFXchans
Mixchannels2:
	push cx
	call Mixchannels
	add bp,Chaninfosize
	pop cx
	loop Mixchannels2
NoFXchans:
	ret

; ==========================================
; >>> Holt Adresse des nÑchsten Patterns <<<
; ==========================================
; Ergebnis in BX
GetNextPattern:
	xor ax,ax
	mov es,ax
	xor ebx,ebx
	xor eax,eax
	mov bx,w[MODpatpos]   ;neue Patternadresse holen
	inc w[MODpatpos]
	add ebx,[MODbase]
	mov al,[es:ebx]       ;Patternnummer holen
	xor ebx,ebx
	mov bx,w[MODchannels]
	shl bx,8		;*64*4*channels
	xor edx,edx
	mul ebx
	mov ebx,eax
	add ebx,MODpatBase
	add ebx,[MODbase]
	ret

; =============================================
; >>> Note und Instrument aus Pattern holen <<<
; >>>		  und umrechnen 	    <<<
; =============================================
notedlyinit:
	xchg bl,bh
	and bx,0fffh
	mov w[bp+ntdlynote],bx
	mov dl,ah
	and dl,0fh
	mov b[bp+ntdlytime],dl
	mov al,b[bp+oldinstr]
	inc al
	mov b[bp+ntdlyinstr],al
	pop eax
	ret

; ===================================
; >>> Initialize slide (effect 3) <<<
; ===================================
MODslideinit:
	xchg bl,bh
	and bx,0fffh		;Slide initialisieren
	jz modslideinit2	;use old dest value..
l1:	push w[bp+sampleperiod]
	mov w[bp+sampleperiod],bx
	push bx
	call findnote
	pop bx
	pop w[bp+sampleperiod]
	call getfinetune
	mov w[bp+slidedest],bx	;(Zielnote)
modslideinit2:
l1:	mov al,b[bp+oldinstr]
	inc al
	mov al,b[bp+samplevolume] ;3fh
	call GetNewVol
	pop eax
	ret

MODgetvoice:
	push eax,cs		;Eintrag retten
	pop ds
	mov ebx,eax
	
	mov cl,al
	shr eax,16
	mov ch,al
	and ch,0fh
	cmp ch,3
	je MODslideinit 	;Slide initialisieren
	mov dx,ax
	and dx,0f00fh
	cmp dx,0d00eh
	je notedlyinit
	mov w[bp+fxcommand],0
dontclearfx:
	shr eax,4		;Samplenummmer holen
	and al,0fh
	and cl,0f0h
	or al,cl
	jnz newsam		;neues Sample initialisieren
	cmp al,3
	je modgevo2		;slide initialisieren (note)
	mov ax,bx		;wurde eine Note uebergeben ?
	and ax,0ff0fh		;restliche bits ausmaskieren
	jz modgevo2		;nein, keine Aenderung
	mov al,b[bp+oldinstr]	;altes Instrument benutzen
	inc al
newsam: test bx,0ff0fh
	jz >x1		;nur eine Samplenummer wurde Åbergeben, Vol=max

	push bx
	call getnewsample	 
	xor ebx,ebx
	pop bx			;wert fÅr kommando wiederholen
	xchg bl,bh		;intel notation
	and bh,0fh
	mov w[bp+sampleperiod],bx
	mov w[bp+vibratobase],bx
	push bx
	call findnote
	mov b[bp+samplenote],al
	pop bx
	call GetFinetune
	call newsamplestep
	jmp >l1
x1:	dec al
	mov b[bp+oldinstr],al	;Samplenummer merken
l1:	mov al,b[bp+samplevolume] ;3fh
	call GetNewVol
MODgevo2:
	pop eax
	ret

; ================================
; >>> Berechnet das Finetuning <<<
; ================================
; al=note  out:bx=new period
GetFinetune:
	mov cl,b[bp+finetune]
	and cl,0fh
	jz nofinetune2		;no fine tuning
	mov bl,cl	;get offset in ft table
	xor bh,bh
	shl bl,3
	mov cx,bx
	add cx,cx
	add bx,cx
	mov cl,al
	shr cl,4	;octave
	and ax,0fh	;use note as offset to finetunetable
	add ax,ax
	add bx,ax
	mov bx,w[finetunetable+bx]
	shr bx,cl
nofinetune2:
	ret

; =========================================
; >>> Find closest note in period table <<<
; =========================================
; AX=Period Out: AL=Octave/Note
findnote:
	mov cx,6*12		    ; there are 6*12 note numbers
	mov si,PeriodTable
	xor bx,bx		    ; current note number = 0
findnote2:
	lodsw
	cmp w[bp+sampleperiod],ax   ; equal to period in pattern?
	je found
	ja closest		  ; passed the exact position, find
				    ; nearest note
	inc bx			    ; no, try next note
	loop findnote2
closest:sub ax,w[periodtable-2+si]  ; Upper delta
	mov dx,w[bp+sampleperiod]
	sub dx,w[Periodtable-4+si]  ; Lower delta
	cmp ax,dx
	jb found		    ; Upper is closer
	dec bx			    ; Lower is closer

found:	mov ax,bx
	xor dx,dx
	mov bx,12
	div bx			    ; AX = Octave, DX = Note
	shl ax,4
	or ax,dx
	; AL Upper nibble = Octave
	; AL Lower nibble = Note
	ret

; =========================================
; >>> Initializes intrument for playing <<<
; =========================================
GetNewSample:
	dec al		;Samplenummer korrigieren
	xor ah,ah
	mov oldinstr[bp],al
	push ax
	shl ax,2	;adresse des Samples holen
	mov bx,ax
	mov eax,sampletab[bx]
	mov sampleadr[bp],eax
	pop ax
	shl ax,5	;Adresse des Sampledescriptors nach bx
	mov bx,ax
	add bx,SampleInfos
	xor eax,eax
	mov ax,[bx+SampleLength]
	add eax,eax	;samplelÑnge korrigieren
	mov [bp+samplerest],eax
	
	xor eax,eax	;Loopstartadresse berechnen
	mov ax,[bx+loopstart]
	add eax,eax
	add eax,[bp+sampleadr]
	mov [bp+sampleloopadr],eax

	xor eax,eax	;LooplÑnge berechnen
	mov ax,[bx+looplen]
	add eax,eax
	mov [bp+samplelooplen],eax
	mov al,[bx+samplefinetune]
	mov [bp+finetune],al
	mov al,[bx+samplevolumeA]
	mov [bp+samplevolume],al
	ret

; ============================================
; >>> Berechnet 100% Protracker-Frequenzen <<<
; ============================================
; BX=periode
newsamplestep:
	mov ax,bx
	xor ebx,ebx
	mov bx,ax
	and bx,0fffh
	jz oldfreq		;neues Sample mit alter Frequenz spielen
  
	mov eax,365fd0h ;0acf0h ;Abzuspielende Frequenz berechnen
	xor edx,edx
	div ebx 		;AX=Samplefrequenz

	xor ebx,ebx
	mov bx,w[DMSresolution] ;Abspielfrequenz
	shl eax,16		;Genauigkeit erhîhen
	xor edx,edx		;zooming berechnen
	div ebx
	mov w[bp+SampleStep],ax
;	 mov w[bp+samplemod],0
oldfreq:ret

; ========================
; >>> Setzt LautstÑrke <<<
; ========================
; BP=Channelinfoblock AL=new vol.
GetNewvol:
	cmp al,40h
	jb getnewvol2
	mov al,3fh
getnewvol2:
	mov b[bp+volume2],al	;volume value
	mov b[bp+tremolobase],al ;save value for tremolo action
	xor ah,ah
	push cx
	mov cl,b[DMSvoltabpot] ;mov ah,al	;vol*256
	shl ax,cl
	pop cx
	add ax,offset voltab
	mov w[bp+volume],ax	;=volume table
	ret

; =====================================
; >>> Korrigiert Periode wenn nîtig <<<
; =====================================
checknotelimit:
	ret
	cmp bx,w[lowlimit]
	jbe checknote2
	mov bx,w[lowlimit]
	jmp checknote3
checknote2:	   
	cmp bx,w[highlimit]
	jae checknote3
	mov bx,w[highlimit]
checknote3:
	ret

notedelay:
	pusha
	cmp b[bp+ntdlytime],0
	je nonotedly
	dec b[bp+ntdlytime]
	jnz nonotedly
	mov al,b[bp+ntdlyinstr]
	call getnewsample
	mov bx,w[bp+ntdlynote]
	call newsamplestep
	popa
	ret
nonotedly:
	cmp b[bp+notecut],0
	je nocut2
	dec b[bp+notecut]
	jnz nocut2
	xor eax,eax
	mov [bp+samplerest],eax
	mov [bp+samplelooplen],eax
nocut2: popa
	ret

; ==========================
; >>> Bearbeitet Effekte <<<
; ==========================
MODsteadyFXb:
	jmp MODsteadyFX
MODgetfx:
	shr eax,16
	and al,0fh		;kommando ausmaskieren
	or ax,ax
	jz MODsteadyFXb
	mov w[bp+FXcommand],ax	;Kommando speichern
; ---------------------------------------------------------------------------
; > Effect 0ch - Volume
; ---------------------------------------------------------------------------
	cmp al,0ch	      ;LautstÑrke
	jne NoVol
	mov al,ah
	call getnewvol
	jmp nofx

NoVol:	
; ---------------------------------------------------------------------------
; > Effect 0dh - Pattern break
; ---------------------------------------------------------------------------
	cmp al,0dh
	jne NoPatCut
	mov al,65
	sub al,ah
	mov b[MODpatcnt],al	;öbrige Zeilen nach Sprung
	xor al,al
	shr ax,4
	xor ecx,ecx
	mov cx,ax
	call getnextpattern
	xor eax,eax
	mov ax,w[MODchannels]
	shl ax,2
	add ebx,ecx
	sub ebx,eax
	mov [MODpatadr],ebx	;Patternadresse speichern
	jmp nofx

; ---------------------------------------------------------------------------
; > Effect 0fh - Speed
; ---------------------------------------------------------------------------
NoPatCut:
	cmp al,0fh		      ;Speed ?
	jne NoSpeed
	cmp ah,1fh
	jbe normalSpeed
	xor ebx,ebx
	mov bl,ah	;BPM*4
	xor bh,bh
	xor eax,eax
	xor edx,edx
	mov ax,w[cs:DMSresolution] ;outlen=samplerate/(BPM*4)
	mov dx,ax
	shl eax,3
	shl edx,1
	add eax,edx
	xor edx,edx
	div ebx
	shr eax,2
	and al,0fch
	mov w[cs:DMSnewmixlen],ax
	call DMS_defnewoutlen
	jmp nofx

normalspeed:
	mov b[MODspeed],ah
	jmp nofx

; ---------------------------------------------------------------------------
; > Effect 0bh - Order jump
; ---------------------------------------------------------------------------
NoSpeed:cmp al,0bh		      ;Order jump ?
	jne NoOrder
	mov al,ah
	xor ah,ah
	push ax
	xor eax,eax
	pop ax
	add eax,MODpatList
	mov [MODpatpos],eax
	mov b[MODpatcnt],1	      ;(aktuelles Pattern abbrechen)
	jmp nofx

; ---------------------------------------------------------------------------
; > Effect 9 - Sample offset
; ---------------------------------------------------------------------------
NoOrder:
	cmp al,9		;Sampleoffset
	jne noSampleoffset
	xor al,al
	push ax
	mov al,b[bp+oldinstr]	;Sample neu starten
	inc al
	call getnewsample
	xor eax,eax
	pop ax
	sub [bp+samplerest],eax
	add [bp+sampleadr],eax
	jmp nofx

; ---------------------------------------------------------------------------
; >>> Extended effects
; ---------------------------------------------------------------------------
nosampleoffset:
	cmp al,0eh		;extended fx ?
	jne noextendedfx2
	mov al,ah
	shr ah,4
	and al,0fh
	
; ---------------------------------------------------------------------------
; > Effect e1h - fine slide up
; ---------------------------------------------------------------------------
	cmp ah,1		;fine slide up
	jne nofineslideup
	mov bx,w[bp+sampleperiod]
	xor ah,ah
	add ax,ax
	sub bx,ax
	jmp storeperiod
noextendedfx2:
	jmp noextendedfx
nofineslideup:
; ---------------------------------------------------------------------------
; > Effect e2h - fine slide down
; ---------------------------------------------------------------------------
	cmp ah,2		;fine slide down
	jne nofineslidedown
	mov bx,w[bp+sampleperiod]
	xor ah,ah
	add ax,ax
	add bx,ax
	jmp storeperiod
nofineslidedown:

; ---------------------------------------------------------------------------
; > Effect eah - fine volume up
; ---------------------------------------------------------------------------
	cmp ah,0ah		;fine volume up
	jne nofinevolup
finevol:add al,b[bp+volume2]
	call getnewvol
	jmp nofx
finevolup:

nofinevolup:			;fine volume down
; ---------------------------------------------------------------------------
; > Effect ebh - fine volume down
; ---------------------------------------------------------------------------
	cmp ah,0bh
	jne nofinevoldown
	neg al
	jmp finevol

; ---------------------------------------------------------------------------
; > Effect e6h - Pattern loop
; ---------------------------------------------------------------------------
nofinevoldown:
	cmp ah,6		;Pattern loop ?
	jne nopatloop
	or al,al		;starting position
	jnz nopatlstrt
	mov eax,[MODpatadr]
	mov [patstrtadr],eax
	mov eax,[MODpatcnt]
	mov [patstrtcnt],eax
alreadylooped:
	jmp nofx
nopatlstrt:
	cmp b[patloopcnt],0	;already in loop ?
	jne alreadylooped	;yes, ignore actual command
	mov b[patloopcnt],al
	jmp nofx
nopatloop:
; ---------------------------------------------------------------------------
; > Effect e9h - note retrig
; ---------------------------------------------------------------------------
	cmp ah,9		;note retrig ?
	jne noretrig
	shl ah,1
	mov b[bp+retrigcnt],al
	jmp nofx
noretrig:
; ---------------------------------------------------------------------------
; > Effect e4h - set vibrato waveworm
; ---------------------------------------------------------------------------
	cmp ah,4
	jne novibww
	push bp
setwaveform:
	test al,4
	jnz swdontretrig
	mov b[bp+VibratoPos],0	      ;retrig waveform
swdontretrig:
	and al,3
	mov b[bp+VibWaveCtrl],al      ;set waveform nr.
	pop bp
	jmp nofx
novibww:
; ---------------------------------------------------------------------------
; > Effect e7h - set tremolo waveform
; ---------------------------------------------------------------------------
	cmp ah,7
	jne notrmww
	push bp
	add bp,5
	jmp setwaveform
notrmww:
; ---------------------------------------------------------------------------
; > Effect e5h - set fine tune
; ---------------------------------------------------------------------------
	cmp ah,5
	jne nofinetune
	mov b[bp+finetune],al
	jmp nofx
nofinetune:
; ---------------------------------------------------------------------------
; > Effect ech - cut note
; ---------------------------------------------------------------------------
	cmp ah,0ch
	jne nocut
	mov b[bp+notecut],al
	jmp nofx
nocut:
; ---------------------------------------------------------------------------
; > Effect eeh - pattern delay
; ---------------------------------------------------------------------------
	cmp ah,0eh
	jne nopdly
	mov b[patterndelay],al
	jmp nofx
nopdly:
; ---------------------------------------------------------------------------
; > Effect e3h - glissando control
; ---------------------------------------------------------------------------
	cmp ah,3
	jne nogliss
	and al,1
	mov b[bp+glissfunc],al
	jmp nofx

nogliss:jmp nofx	     ; *** neu !
noextendedfx:
; ---------------------------------------------------------------------------
; >>> Bestehende Effekte
MODsteadyFX:
	cmp b[bp+retrigcnt],0	;Note retrig control
	je nogvretrig
	dec b[bp+retrigcnt]
	jnz nogvretrig
	mov al,b[bp+oldinstr]
	inc al
	call getnewsample
nogvretrig:
	mov ax,w[bp+FXcommand]
; ---------------------------------------------------------------------------
; >>> Effect 0 - Arpeggio
; ---------------------------------------------------------------------------
	or al,al
	jmp NoArpeggio
	or ah,ah
	je noarpeggio
	mov dl,ah
	push dx
	xor dx,dx
	mov ax,w[Playcnt]
	mov bx,3
	div bx
	mov ah,dl		;this is the actual note number
	pop dx			;notes are stored in here
	or ah,ah
	je nofx3		;play base note
	dec ah
	jz arphighnote		;play high note
	and dl,0fh
	jmp arplownote		;play the fucking low note
arphighnote:
	shr dl,4
arplownote:
	xor dh,dh
	xor bh,bh
	mov bl,b[bp+samplenote]
	push bx 		;add arpnote to base note
	and bx,0fh
	add dx,bx
	pop bx
	shr bx,4		;put the octave to bx
arpgetoct:
	cmp dx,12		;octave boundary check
	jb  arpfinetune
	sub dx,12
	inc bx
	jmp arpgetoct

arpfinetune: ;dx=note bx=octave
	xor ah,ah
	mov al,[bp+samplefinetune]   ; al = instrument finetune
;	 cmp [extendedOctaves],0 ; are extended octaves in use?
;	 jne extoct
	cmp bx,3	       ; is octave number <= 3?
	jbe extoct	       ; if is, skip

	sub bx,3
	inc al		;finetune increased by 1 for 100% Protracker comp.
extoct:
	;now the octave is found in BX while the note number is stored in DX

	mov cl,24
	mul cl
	mov cx,bx	;store octave in cx
	add dx,dx	;note offset in finetune table
	add dx,ax	;tuning offset in finetune table
	mov bx,dx
	mov bx,[bx+Finetunetable]      ; bx = period number for this note
	shr bx,cl		       ; Shift by octave number
	add bx,bx
	call checknotelimit
	call newsamplestep
	ret

nofx3:	mov bx,w[bp+sampleperiod]
	call newsamplestep
	ret

NoArpeggio:
; ---------------------------------------------------------------------------
; > Effect 1 - Portamento up
; ---------------------------------------------------------------------------
	cmp al,1		;Portamento up
	jne noportup
	mov bx,w[bp+sampleperiod]
	mov al,ah
	xor ah,ah
	sub bx,ax
storeperiod:
	call checknotelimit
	mov w[bp+sampleperiod],bx
	call newsamplestep
	ret
noportup:
; ---------------------------------------------------------------------------
; > Effect 2 - Portamento down
; ---------------------------------------------------------------------------
	cmp al,2		;Portamento down
	jne noportdown
	mov bx,w[bp+sampleperiod]
	mov al,ah
	xor ah,ah
	add bx,ax
	jmp storeperiod

noportdown:
; ---------------------------------------------------------------------------
; > Effect 3 - Note slide
; ---------------------------------------------------------------------------
	cmp al,3		;note slide
	jne noslide
	or ah,ah		;neue slide-geschwindigkeit benutzen ?
	jnz newslidespd
	mov ah,b[bp+slidespeed] ;nein, alte
noteslide:
newslidespd:
	mov bx,w[bp+sampleperiod]
	mov cx,w[bp+slidedest]
	cmp bx,cx		;source and destination the same ?
	je slidedone
;	 mov al,b[bp+slidespeed]
	mov al,ah		;slide speed
	mov b[bp+slidespeed],al
	xor ah,ah
	cmp bx,cx
	ja slidesub		;subtract from period
	add bx,ax		;add to period
	cmp bx,cx
	jb slidepartdone	;sliding was ok
	jmp slidecorrect
slidesub:
	sub bx,ax
	cmp bx,cx
	jb slidecorrect
slidepartdone:
	mov w[bp+sampleperiod],bx
	mov w[bp+vibratobase],bx
	call newsamplestep
	ret

slidecorrect:			;set period to destination
	mov bx,cx
	mov w[bp+vibratobase],bx
	mov w[bp+sampleperiod],bx
	call newsamplestep
slidedone:
	xor ax,ax		;sliding beendet
	mov w[bp+fxcommand],ax
	ret

noslide:
; ---------------------------------------------------------------------------
; > Effect 0ah - Volume slide
; ---------------------------------------------------------------------------
	cmp al,0ah		;volumeslide
	jne novolumeslide
volumeslide:
	test ah,0fh
	jnz volslidedown
volumeslideup:
	shr ah,4
	add ah,b[bp+volume2]
	cmp ah,3fh
	jbe nvsc1
	mov ah,3fh
nvsc1:	mov al,ah
	call getnewvol
	ret

volslidedown:
	mov al,b[bp+volume2]
	sub al,ah
	jnc nvsc2
	xor al,al
nvsc2:	call getnewvol
	ret
notremolo2:
	jmp notremolo
novolumeslide:
; ---------------------------------------------------------------------------
; > Effect 7 - set tremolo
; ---------------------------------------------------------------------------
	cmp al,7	    
	jne notremolo2
	test ah,0fh		;depth=0 ?
	jz tremolo		;yep,nowt to do..
	test ah,0F0h		;rate=0 ?
	jz tremolo		;yep, forget it...
tremolo2:
	mov b[bp+tremolocmd],ah
	xor al,al
	mov b[bp+tremolopos],al
	mov b[bp+fxcommand+1],al
tremolo:mov al,[bp+tremoloPos]
	and ax,001Fh
	mov bl,[bp+TrmWaveCtrl]
	and bl,03h		;get waveform
	jz trm_sine		;call sine algorithm
	cmp bl,1
	je trm_rampdown 	;rampdown
	mov bl,255		;square wave
	cmp b[bp+tremoloPos],0
	jnb trm_set
	xor bl,bl
	jmp trm_set		;nowt
trm_rampdown:
	cmp b[bp+tremoloPos],0
	jnb trm_rampdown2	;rampdown
	mov bl,255
	sub bl,al
	jmp trm_set
trm_rampdown2:
	mov bl,al
	jmp trm_set
trm_sine:
	xor bx,bx
	mov bl,al
	mov bl,[bx+VibratoTable]
trm_set:mov al,[bp+tremoloCmd]	;depth
	and al,0Fh
	mul bl
	shr ax,7
	mov bl,b[bp+tremolobase]
	test b[bp+tremoloPos],20h
	jnz tremoloNeg
	add bl,al
	jmp tremolo3
tremoloNeg:	
	sub bl,al
tremolo3:
	mov al,bl
	push w[bp+tremolobase]
	call getnewvol
	pop w[bp+tremolobase]
	mov al,[bp+tremoloCmd]	;rate
	shr al,4
	add b[bp+tremoloPos],al
	ret
notremolo:
; ---------------------------------------------------------------------------
; > Effect 4 - Vibrato
; ---------------------------------------------------------------------------
	cmp al,4
	jne novibrato
	test ah,0fh		;depth=0 ?
	jz vibrato		;yep,nowt to do..
	test ah,0F0h		;rate=0 ?
	jz vibrato		;yep, forget it...
vibrato2:
	mov b[bp+vibratocmd],ah
	xor al,al
	mov b[bp+vibratopos],al
	mov b[bp+fxcommand+1],al
vibrato:mov al,[bp+VibratoPos]
	and ax,001Fh
	mov bl,[bp+VibWaveCtrl]
	and bl,03h		;get waveform
	jz vib_sine		;call sine algorithm
	cmp bl,1
	je vib_rampdown 	;rampdown
	mov bl,255		;square wave
	cmp b[bp+VibratoPos],0
	jnb vib_set
	xor bl,bl
	jmp vib_set		;nowt
vib_rampdown:
	cmp b[bp+VibratoPos],0
	jnb vib_rampdown2	;rampdown
	mov bl,255
	sub bl,al
	jmp vib_set
vib_rampdown2:
	mov bl,al
	jmp vib_set
vib_sine:
	xor bx,bx
	mov bl,al
	mov bl,[bx+VibratoTable]
vib_set:mov al,[bp+VibratoCmd]	;depth
	and al,0Fh
	mul bl
	shr ax,7
	mov bx,[bp+sampleperiod] ;[bp+vibratobase]
	test b[bp+VibratoPos],20h
	jnz VibratoNeg		 ;jb BMI
	add bx,ax
	jmp Vibrato3
VibratoNeg:	
	sub bx,ax
Vibrato3:
	call newsamplestep
	mov al,[bp+VibratoCmd]	;rate
	shr al,4
	add b[bp+VibratoPos],al
	ret
novibrato:
; ---------------------------------------------------------------------------
; > Effect 5 - Keeps tone portamento and does volume slide
; ---------------------------------------------------------------------------
	cmp al,5
	jne noprtvs
	push ax
	call NoteSlide
	pop ax
	jmp volumeslide
noprtvs:
; ---------------------------------------------------------------------------
; > Effect 6 - Keeps vibrato and does volume slide
; ---------------------------------------------------------------------------
	cmp al,06h
	jne Novibvs
	push ax
	call vibrato		;continue vibrato
	pop ax
	jmp volumeslide 	;go to volume slide
Novibvs:
	ret
nofx:	xor ax,ax
	mov w[bp+FXcommand],ax
	ret

; ----------------------------------------------------------------------------

; ====================================================
; >>> MOD-Datei laden und Variablen initialisieren <<<
; ====================================================
initMOD:cli
	push ds,es
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov di,MODl1		;Datenbereich lîschen
	mov cx,MODl2-MODl1
	xor al,al
	rep stosb

	mov dx,si		;Name des Moduls
	mov edi,[XMSbase]	;freier Platz im XMS
	mov [MODbase],edi
	call MODloadFile
; ----- 4 channel MODs
	xor ax,ax
	mov ds,ax
	mov cx,4
	xor esi,esi
	mov si,MODheadersign
	add esi,[cs:MODbase]
	mov eax,[esi]
	cmp eax,".K.M"
	je MODsignfound2
	cmp eax,"!K!M"
	je MODsignfound2
	cmp eax,"4TLF"
	je MODsignfound2
	cmp eax,"ATCO"
	je MODsignfound2

; ----- X channel MODs
	mov ebx,eax
	cmp bh,"C"
	jne MODnochnx
	mov ecx,eax
	shr ecx,16
	cmp ecx,"NH"	   ; 8CHN-sign
	jne MODnochnX
	mov cl,al
	sub cl,"0"
	xor ch,ch
MODsignfound2:
	jmp MODsignfound
MODnochnX:
	mov ebx,eax
	shr ebx,16
	cmp bx,"HC"	   ; xxCH-sign
	jne MODnoXXch
	mov cl,al
	sub cl,"0"
	shl cl,1
	mov ch,cl
	shl ch,2
	add cl,ch
	xor ch,ch
	sub ah,"0"
	add cl,ah
	jmp MODsignfound
MODnoXXch:
	cmp ax,"DT"		  ; TDZx-sign
	jne MODnotdzX
	shr eax,16
	cmp al,"Z"
	jne ModnotdzX
	mov cl,ah
	sub cl,"0"
	xor ch,ch
	jmp MODsignfound
MODnotdzX:
	mov ax,cs
	mov ds,ax
	mov dx,TXT_notamod	; not a Protracker module
	mov ah,9
	int 21h
	jmp bye2

MODsignfound:
	mov w[cs:MODchannels],cx   ;save channel count
	add cx,w[cs:FXchannels]
	mov w[cs:DMSchannels],cx
; ----- count patterns
	xor esi,esi
	mov si,MODpatList	;Zeiger auf Patternliste
	add esi,[cs:MODbase]
	mov cx,128		;128 EintrÑge untersuchen
	xor eax,eax
CountPatterns:			;Anzahl der Patterns untersuchen
	addr:
	lodsb
	cmp al,ah
	jbe NoNewerPattern
	mov ah,al
NoNewerPattern:
	loop CountPatterns
	mov al,ah		;Adresse des 1. Samples berechnen
	xor ah,ah
	inc ax
	xor ebx,ebx
	mov bx,[cs:MODchannels]
	shl ebx,8
	xor edx,edx
	mul ebx
	add eax,MODpatBase
	add eax,[cs:MODbase]
	mov [cs:samplebase],eax ;Adressen der Sampledaten

	push eax		;retten

	mov edx,eax		;Adresse des 1.Samples
	mov bp,SampleTab	;Adressen der Samples
	xor esi,esi
	mov si,MODsampleList	;Samplebeschreibungen
	add esi,[cs:MODbase]
	mov di,SampleInfos	;interne Beschreibungen
	mov ax,cs
	mov es,ax
	mov cx,31		;31 Sampledaten Åbertragen
CopySampleInfo:
	push cx
	mov [cs:bp],edx 	;lineare Sampleadresse speichern

	add bp,4		;Zeiger auf nÑchste

	xor eax,eax
	addr:
	lodsw			;samplelÑnge auf intel-notation bringen
	xchg al,ah
	stosw			;und im internen Descriptor speichern
  
	add eax,eax		;SamplelÑnge*2
	add edx,eax		;und neue Position ist berechnet
	addr:
	lodsw			;finetuning und lautstÑrke
	stosw
  
	xor eax,eax
	addr:
	lodsw			;Loopanfang auf intel-notation
	xchg al,ah
	stosw
  
	addr:
	lodsw			;loopende auf intel-notation
	xchg al,ah
	stosw
  
	mov cx,6		;rest kopieren
	addr:
	rep movsd
	pop cx
	sub esi,2
	loop CopySampleInfo
  
	mov ax,ds
	mov es,ax
  
	pop esi 		;1. Sampleadresse holen
	mov edi,esi
	mov bp,SampleInfos
	mov cx,31
Vol1:	push cx
	xor eax,eax
	mov ax,[cs:bp+Samplelength]
	or ax,ax
	jz VolSkip
	add eax,eax
	mov ecx,eax
Vol2:	addr:
	lodsb
	add al,128
	addr:
	stosb
	dec ecx
	jnz vol2
VolSkip:pop cx
	add bp,32
	loop Vol1
	pop es,ds
	sti
	ret

; ================================
; >>> Musikausgabe einschalten <<<
; ================================
StartMOD:
	push ds,es
	mov ds,cs
	mov al,1			;Variablen initialisieren
	mov b[MODspeedc],al		;damit sofort die erste Pattern-
	mov b[MODpatcnt],al		;zeile ausgelesen wird
	mov b[MODspeed],6
	mov w[SMPtabptr],SMPtab
	mov d[MODpatpos],MODpatList
	mov w[MODoldmixpos],0
	mov ax,bufseg			;Puffer initialisieren
	call DMS_init
	cli
	xor ax,ax
	mov ds,ax
	mov bx,8*4
	mov ax,cs
	shl eax,16
	mov ax,modframe
	xchg eax,[8*4]
	mov [cs:oldtimerV],eax
	mov al,34h
	out 43h,al
	mov al,low(1193180/MODintcalls)
	out 40h,al
	mov al,high(1193180/MODintcalls)
	out 40h,al
	call w[cs:SDEV_start]
	sti
	pop es,ds
	ret

; ===============================
; >>> Musikausgabe abschalten <<<
; ===============================
StopMOD:push ds,es
	cli
	call w[cs:SDEV_stop]
	mov ds,cs
	mov al,34h		;Timer auf alten Wert
	out 43h,al
	xor al,al
	out 40h,al
	out 40h,al
	xor ax,ax
	mov ds,ax
	mov eax,[cs:oldtimerV]
	mov [8*4],eax
	sti
	pop es,ds
	ret

; ===================
; >>> Datei laden <<<
; ===================
; CS:DX=name EDI=dest
MODloadFile:
	push ds,es
	xor eax,eax
	mov [cs:filesize],eax
	mov es,ax
	mov ds,cs
	mov ax,3d00h		;Datei îffnen
	int 21h
	jc MODErrorBye
	mov bx,ax		;Handle Åbergeben
	mov w[cs:SourceHandle],ax
MODloadNew:
	xor eax,eax
	mov ah,3fh		;Block einlesen
	mov cx,32k		;Soviel wie mîglich laden
	mov ds,w[cs:newseg]
	xor dx,dx		;Zieladresse
	int 21h 		;Daten laden
	or ax,ax
	jz MODloaded
	xor si,si		;geladenen Block ins XMS kopieren
	mov cx,ax
	add [cs:filesize],eax
fuckme: lodsb
	addr: stosb
	loop fuckme
	jmp MODloadNew
MODloaded:
	mov ah,3eh		;Datei schliessen
	int 21h
	mov [cs:XMSbase],edi
	pop es,ds
	ret

MODErrorBye:
	mov ax,3
	int 10h
	mov dx,ERR_MODerr
	mov ah,9
	mov ds,cs
	int 21h
	jmp bye2

; ==============================
; >>> Plays single FX sample <<<
; ==============================
; AX=Sample no.
StartSMP:
	cmp w[cs:FXchannels],0
	je >l1
	cli
	push ds,es,cs
	pop ds
	shl ax,3
	push ax
	mov bp,Channelinfos	;1. adr of FX channel info
	mov ax,chaninfosize
	mov bx,w[MODchannels]
	xor dx,dx
	mul bx
	add bp,ax
; ----- search for (neary) free channel
	mov esi,-1
	mov cx,w[FXchannels]
searchsmpchan:
	cmp [bp+samplerest],esi
	jae searchsmp2
	mov esi,[bp+samplerest]
	mov di,bp
searchsmp2:
	add bp,chaninfosize
	loop searchsmpchan
	mov bp,di
	pop bx
	mov eax,[bx+SMPtab]		;sample adr
	mov [bp+sampleadr],eax
	mov eax,[bx+SMPtab+4]		;sample len
	mov [bp+samplerest],eax
	xor eax,eax			;no loop
	mov [bp+Samplelooplen],eax
	mov ax,-1			;hehehehe....
	mov w[bp+samplestep],ax
	mov ax,w[DMSvoltabsize] ;63*voltabsize+voltab
	push bx
	mov bx,ax
	shl ax,6
	sub ax,bx
	pop bx
	add ax,offset voltab
	mov w[bp+volume],ax
	pop es,ds
	sti
l1:	ret

; =============================
; >>> Loads SMP file to XMS <<<
; =============================
; DX=Name
LoadSMP:push ds,es
	mov ax,w[cs:SMPtabptr]
	or ax,ax
	jnz loadsmp2
	mov w[cs:SMPtabptr],SMPtab
loadsmp2:
	xor ax,ax
	mov ds,ax
	mov edi,[cs:XMSbase]	;freie Adresse
	push edi
	mov bp,w[cs:SMPtabptr]
	mov [cs:bp],edi 	;Sampleadresse
	call MODloadfile
	pop esi
	mov eax,[cs:filesize]
	mov [cs:bp+4],eax	;sample len.
	add bp,8
	mov ecx,eax
	mov w[cs:SMPtabptr],bp
	xor ax,ax
	mov es,ax
	mov ds,ax
	mov edi,esi
convertSMP:
	addr: lodsb
	add al,128
	addr: stosb
	addr: loop convertSMP
	pop es,ds
	ret

; ----------------------------------------------------------------------------

#if !_mdltiny
_text	segment
#endif

; ====================
; >>> Datenbereich <<<
; ====================
ERR_MODerr:
	db"MODPLAY: Can't find file.",10,13,"$"
TXT_notamod:
	db"This is not a module !",10,13,"$"

VibratoTable:
	db 0,24,49,74,97,120,141,161
	db 180,197,212,224,235,244,250,253
	db 255,253,250,244,235,224,212,197
	db 180,161,141,120,97,74,49,24
;  C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
;  C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
;  C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113
Periodtable:
	dw  3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920,1814
	dw  1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906 
	dw  856,808,762,720,678,640,604,570,538,508,480,453
	dw  428,404,381,360,339,320,302,285,269,254,240,226 
	dw  214,202,190,180,170,160,151,143,135,127,120,113
	dw  107,101,95,90,85,80,75,71,66,63,60,56

;	 dw  3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920,1814
;	 dw  1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906 
;	 dw  856,808,762,720,678,640,604,570,538,508,480,452 
;	 dw  428,404,380,360,340,320,302,286,270,254,240,226 
;	 dw  214,202,190,180,170,160,150,142,134,126,120,112 
;	 dw  106,100,94,90,84,80,74,70,66,62,60,56 
finetunetable:
	dw  3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920,1814 
	dw  3400,3208,3028,2860,2696,2548,2404,2268,2140,2020,1908,1800 
	dw  3376,3184,3008,2836,2680,2528,2388,2252,2128,2008,1896,1788 
	dw  3352,3164,2984,2816,2660,2512,2368,2236,2112,1992,1880,1776 
	dw  3328,3140,2964,2796,2640,2492,2352,2220,2096,1980,1868,1764 
	dw  3304,3116,2944,2776,2620,2476,2336,2204,2080,1964,1852,1748 
	dw  3280,3096,2920,2756,2604,2456,2320,2188,2064,1948,1840,1736 
	dw  3256,3072,2900,2736,2584,2440,2300,2172,2052,1936,1828,1724 
	dw  3628,3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920 
	dw  3600,3400,3208,3028,2860,2700,2544,2404,2268,2140,2020,1908 
	dw  3576,3376,3184,3008,2836,2680,2528,2388,2252,2128,2008,1896 
	dw  3548,3352,3164,2984,2816,2660,2512,2368,2236,2112,1992,1880 
	dw  3524,3328,3140,2964,2796,2640,2492,2352,2220,2096,1976,1868 
	dw  3500,3304,3116,2944,2776,2620,2476,2336,2204,2080,1964,1852 
	dw  3472,3280,3096,2920,2756,2604,2456,2320,2188,2064,1948,1840 
	dw  3448,3256,3072,2900,2736,2584,2440,2300,2172,2052,1936,1828 
FXchannels:	dw 0
	.data
even
MODl1:
sampleTab:	dd 32 dup ?   ;Adressen der Samples
ChannelInfos:	db ChanInfoSize*16 dup ?
SampleInfos:	db 32*Sampleinfosize dup ?
modmixing:	db ?	      ;Flag for internal speaker
flag_frame	db ?
MODoldMixPos:	dw ?
playcnt:	dw ?

filesize:	dd ?
samplebase:	dd ?	     ;Adresse der Patterns
MODpatadr:	dd ?	      ;Adresse im Pattern
MODpatpos:	dd ?	      ;Adresse in Patternliste
MODseg: 	dw ?
MODspeedC:	db ?	     ;WartezÑhler
MODpatcnt:	db ?	     ;ZeilenzÑhler
IRQflag:	db ?
MODspeed:	db ?
MODoldIrq:	db ?
modtimingcheck: dw ?
modoldinttimer: dw ?
patstrtadr:	dd ?
patstrtcnt:	dd ?
patloopcnt:	db ?
patterndelay:	db ?
lowlimit:	dw ?
highlimit:	dw ?
oldtimerV:	dd ?
MODbase:	dd ?
MODchannels:	dw ?
SMPtabptr:	dw ?
SMPtab: 	dd 256 dup ?
MODl2:

