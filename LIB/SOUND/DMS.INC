; ===============================
; >>>  Digital mixing system  <<<
; >>>  (c)1994-1996 S. Klose  <<<
; ===============================
; 8 bit/16 bit mono
	.code
surroundshift=1
; =========================================
; >>> Initialisierung der Mischroutinen <<<
; =========================================
DMS_init:
	push ds,es
	mov ds,cs
	mov es,ax
	mov [DMAbufferseg],ax	;64k zum mixen
	xor di,di		;Puffer lîschen
	mov ax,8080h
	mov cx,8000h
	rep stosw
	mov ax,[DMSresolution] ;Anzahl der zu mischenden Werte pro Tick
	xor dx,dx
	mov bx,50
	div bx
	and ax,0fffch		;(muss wegen stereo durch 4 teilbar sein
	mov [DMSmixlen],ax
	test b[sdev_bits],128	;Stereo ?
	if nz add ax,ax 	;Ja, doppelte Ausgabegrî·e
	mov [DMSoutlen],ax
	test b[sdev_bits],16	;16-Bit-Ausgabe ?
	jne DMS_init16		;ja,...

	mov b[DMSmode],0	;8-Bit-Ausgabe ...
	mov w[DMSvoltabsize],256
	mov b[DMSvoltabpot],8
	mov ax,cs		;LautstÑrken berechnen
	mov es,ax
	mov di,Voltab
	mov cx,64		;LautstÑrken
l2:	push cx
	mov bp,64
	sub bp,cx
	mov cx,256		;Werte
l3:	mov ax,128		;w=w*vol
	sub ax,cx
	mov bx,bp
	xor dx,dx
	imul bx
	mov bx,[DMSchannels]	;/chnls
	test b[sdev_bits],128
	jz >l6
	shr bx,1		;stereo: halbe Kanaldichte pro Seite
l6:	shl bx,6		;/64
	xor dx,dx
	or ax,ax
	if nz idiv bx
	;mov bl,al		;Bereich fÅr Surround offenhalten
	;sar bl,surroundshift
	;sub al,bl
	stosb
	loop l3
	pop cx
	loop l2
	jmp dmi1

DMS_init16:
	mov b[DMSmode],1
	mov w[DMSvoltabsize],512
	mov b[DMSvoltabpot],9
	shl w[DMSoutlen],1	;Doppelte PufferlÑnge bei 16bit
	mov ax,cs		;LautstÑrken berechnen
	mov es,ax
	mov di,Voltab
	mov cx,64		;LautstÑrken
l2:	push cx
	mov bp,64
	sub bp,cx

	xor ecx,ecx
	mov cx,256		;Werte
l3:
	xor eax,eax
	mov ax,128		 ;w=w*vol
	sub eax,ecx
	shl eax,8
	xor ebx,ebx
	mov bx,bp
	xor edx,edx
	imul ebx
	xor ebx,ebx
	mov bx,[DMSchannels]	;/chnls
	test b[sdev_bits],128
	jz >l6
	shr bx,1
l6:	shl bx,6		;/64
	xor edx,edx
	or eax,eax
	if nz idiv ebx
	cmp b[dms_sur],1
	jne >n1
	mov bx,ax
	sar bx,surroundshift	;Freiraum fÅr Surround
	sub ax,bx
n1:	stosw
	loop l3
	pop cx
	loop l2

dmi1:	mov ax,[DMSoutlen]     ;Startposition fÅr die Mixroutine unten
	mov [DMSnewoutlen],ax
	mov [DMSmixpos],ax
	mov ax,[DMSmixlen]
	mov [DMSnewmixlen],ax
	pop es,ds
	ret

; --------------------------------------------------------------------------
; ===================================
; >>> Mischt KanÑle in den Puffer <<<
; ===================================
; BP=Zeiger auf Channelinfo
; b[actualchannel] muss vor dem mischen des 1. Kanals auf 0 gesetzt werden
MixChannels:
	mov ds,cs
	mov es,[dmabufferseg]
	mov di,[DMSmixpos]     ;Startposition des nÑchsten Ticks
	test b[sdev_bits],128	;stereo ?
	jz >l1			;Nein, weiter
	test b[actualchannel],1 ;offset fÅr rechten kanal angeben ?
	je >l1			;Nein, weiter
	inc di			;8-bit +1
	test b[sdev_bits],16	;16-bit ?
	je >l1			;Nein, weiter
	inc di			;Ja, +2 insgesamt
l1:	xor ecx,ecx
	mov cx,[DMSmixlen]     ;Grî·e des zu mischenden Abschnittes
	mov dx,cx     
Mich5:	mov eax,[bp+samplerest];Testen ob Åberhaupt ein Sample anliegt
	or eax,eax
	je LoopCheck		;Nein, auf Loop testen
	
	push dx 		;AX=CX*steps (richtige SamplelÑnge)
	mov eax,ecx
	xor ebx,ebx
	mov bx,[bp+samplestep]
	xor edx,edx
	mul ebx
	shr eax,16
	sub eax,[bp+samplerest];genug sample da ?
	jc >l1			;Ja,kopieren
	
	mov eax,[bp+samplerest];Nein,Rest vom Sample spielen

	shl eax,16		;Rest des Samples/steps=LÑnge im Puffer (cx)
	xor edx,edx
	xor ebx,ebx
	mov bx,[bp+samplestep]
	div ebx
	mov cx,ax
l1:	pop dx

	push cx,dx
	mov esi,[bp+Sampleadr]	;sampleadresse
	call Mixit
	mov eax,esi		;Gespielte LÑnge des SamplestÅcks berechnen
	sub eax,[bp+SampleAdr]
	sub [bp+samplerest],eax ;RestlÑnge berechnen
	mov [bp+SampleAdr],esi
	pop dx,cx
	sub dx,cx		;RestlÑnge berechnen
	jz >m1			;Tickabschnitt ist voll, fertig
	mov cx,dx
LoopCheck:
	mov eax,[bp+samplelooplen];Ist eine Schleife vorhanden ?
	shr eax,2
	or eax,eax
	jz DMS_ClearBuffer	;Nein, Rest auffÅllen
	mov eax,[bp+sampleloopadr];Adresse und LÑnge des Loops Åbertragen
	mov [bp+sampleadr],eax	;ins Sampleinfo
	mov eax,[bp+samplelooplen]
	mov [bp+Samplerest],eax
	jmp Mich5		;und weitermischen
DMS_clearbuffer:
	call clbu2
m1:	inc b[cs:actualchannel]
	ret

; ---------------------------------------------------------------------------
; =============================================
; >>> Lîscht Rest des Puffers bei 1. Kanal, <<<
; >>> oder addiert Mindestwerte zum Puffer  <<<
; =============================================
ClBu2:	mov ax,[dmabufferseg]
	mov ds,ax
	mov es,ax
	xor ax,ax
	cmp b[cs:DMSmode],1
	je >n1			;zu den 16-bit Routinen

	test b[cs:sdev_bits],128
	jne >l1

	; 8 bit mono
	cmp bp,Channelinfos
	jne >s1
	mov al,80h
	rep stosb
s1:	ret

	; 8 bit stereo
l1:	cmp b[cs:actualchannel],1
	ja >l2
	mov al,80h
	sub di,2
l3:	add di,2
	mov [di],al
	dec cx
	jnz l3
l2:	ret
;l4:
;	 mov al,[cs:DMSchannels]
;	 dec al
;	 cmp [cs:actualchannel],al
;	 jne l2
;l6:	 mov ax,[di-1]
;	 xor ax,8080h
;	 mov bx,ax
;	 shr al,1
;	 shr ah,1
;	 add ah,bl
;	 add al,bh
;	 mov [di-1],ah
;	 mov [di],al
;	 inc di,2
;	 dec cx
;	 jnz l6
;	 ret

	;16 bit Routinen
n1:	test b[cs:sdev_bits],128
	jne >s2 	;stereo
	;16 bit mono
	cmp bp,Channelinfos
	jne >s1
	rep stosw
s1:	ret

	; 16 bit stereo
s2:	cmp b[cs:actualchannel],1;stereo lîschen
	ja >l4
	xor ax,ax
l3:	mov [di],ax
	add di,4
	dec cx
	jnz l3
l2:	ret

l4:	mov al,[cs:DMSchannels]
	dec al
	cmp [cs:actualchannel],al
	jne l2
l6:	mov ax,[di-2]	       ;16 bit stereo surround clear
	mov bx,[di]
	sar ax,surroundshift
	sar bx,surroundshift
	add [di-2],bx
	add [di],ax
	add di,4
	dec cx
	jnz l6
	ret

; =====================================================
; >>> Kopiert (1.Kanal) oder addiert (2.-x.Kanal)   <<<
; >>> zum DMA-Puffer (inklusive LautstÑrkeregelung) <<<
; =====================================================
MixIt:	cmp b[cs:DMSmode],1
	je >n16 		;16-Bit-Routinen
	cmp bp,channelinfos
	jne >l1
	jmp Copy2buffer
l3:	jmp copy2bufferS
l1:	test b[cs:sdev_bits],128;Bei stereo...
	jz >l2
	cmp b[cs:actualchannel],1;2. Kanal auch kopieren
	je l3
l2:	jmp Mix2buffer

; >>> 16 bit
n16:	cmp bp,ChannelInfos	;1. Kanal ?
	jne >l1
l3:	jmp Copy2buffer16	;Ja, Sample draufkopieren
l1:	test b[cs:sdev_bits],128;Nein, 2. Kanal bei stereo ?
	jz >l2			;  Nein, 16-bit mono
	cmp b[cs:actualchannel],1
	je l3			;Ja, kopieren
l2:	jmp Mix2buffer16	;addieren

Copy2buffer:
	mov bx,[bp+SampleMOD]
	mov dx,[bp+SampleStep]
	mov ax,[bp+volume]
	mov [tVolAdr],ax
	push ds,bp
	xor ax,ax
	mov ds,ax
	test b[cs:sdev_bits],128
	jne copy2bufferS       ;8-Bit stereo

l1:	mov al,80h
	movzx bp,b[esi]
	add bp,[cs:tvoladr]
	add al,[cs:bp]
	stosb			;Wert in den Puffer schreiben
	add bx,dx		;BX erhîhen bei öbertrag
	adc esi,0
	dec cx
	jnz l1			;Weiter bis Puffer gefÅllt ist
	pop bp,ds
	mov [bp+sampleMOD],bx
	ret

copy2bufferS:
l1:	mov al,80h
	movzx bp,b[esi]
	add bp,[cs:tvoladr]
	add al,[cs:bp]
	mov [es:di],al		  ;Wert in den Puffer schreiben
	add di,2
	add bx,dx		;BX erhîhen bei öbertrag
	;adc esi,0
	dec cx
	jnz l1			;Weiter bis Puffer gefÅllt ist
	pop bp,ds
	mov [bp+sampleMOD],bx
	ret

copy2buffers_sur:
l1:	movzx bp,b[esi]
	add bp,[cs:tvoladr]
	mov al,[cs:bp]
	mov ah,[es:di-1]
	sub ah,80h
	push ax
	sar ah,surroundshift
	add al,ah
	add al,80h
	mov [es:di],al		  ;Wert in den Puffer schreiben
	pop ax
	sar al,surroundshift
	add ah,al
	add ah,80h
	mov [es:di-1],ah
	add di,2
	add bx,dx		;BX erhîhen bei öbertrag
	adc esi,0
	dec cx
	jnz l1			;Weiter bis Puffer gefÅllt ist
	pop bp,ds
	mov [bp+sampleMOD],bx
	ret

Copy2buffer16:
	mov bx,[bp+SampleMOD]
	mov dx,[bp+SampleStep]
	mov ax,[bp+volume]
	mov [tVolAdr],ax
	push ds,bp
	xor ax,ax
	mov ds,ax
	test b[cs:sdev_bits],128   ;stereo ?
	jne copy2buffer16s
	
l1:	movzx bp,b[esi]
	add bp,bp
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
	stosw			;Wert in den Puffer schreiben
	add bx,dx		;BX erhîhen bei öbertrag
	adc esi,0
	dec cx
	jnz l1			;Weiter bis Puffer gefÅllt ist
	pop bp,ds
	mov [bp+sampleMOD],bx
	ret

Copy2buffer16s: 		;16-Bit-Ausgabe stereo
	cmp dx,8000h		;Ist das Sample skalierbar ?
	je copy2buffer16si	;Yepp, Oversampling benutzen
copy2buffer16sb:
l1:	movzx bp,b[esi]
	add bp,bp
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
	mov [es:di],ax	       ;Wert in den Puffer schreiben
	add bx,dx		;BX erhîhen bei öbertrag
	adc esi,0
	add di,4
	dec cx
	jnz l1			;Weiter bis Puffer gefÅllt ist
	pop bp,ds
	mov [bp+sampleMOD],bx
	ret

; Mix 16bit stereo oversampled
copy2buffer16si:		;16-Bit-Ausgabe stereo
	cmp b[cs:DMS_over],2
	jne copy2buffer16sb
	add dx,dx		;Zooming verklerinern
l3:	movzx bp,b[esi]
	add bp,bp		;1. Wert holen
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
l1:	mov [es:di],ax		;Wert auf den Puffer addieren
	add di,4
	add bx,dx
	jc >l2			;Neuen Wert holen...
	dec cx			;weiter, stop bei Frameende
	jnz l1
l5:	pop bp,ds
	mov [bp+SampleMOD],bx  ;Rest des ScalierungszÑhlers retten.
	ret

	align 10h
l2:	dec cx			;CX korrigieren (letzer Loop verpasst)
	jz l5			;Ende des Frames
	movzx bp,b[esi+1]
	add bp,bp		;nÑchsten Wert holen
	add bp,[cs:tVoladr]
	mov bp,[cs:bp]		;und mit Hilfe des alten Wertes einen
	sub bp,ax		;Zwischenwert errechnen
	sar bp,1		;zw=nw+(nw-aw)/2
	add ax,bp

l1:	mov [es:di],ax
	add di,4
	add bx,dx
	jc >l4
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

l4:	inc esi
	dec cx			;CX korrigieren
	jz l5
	jmp l3

; 8 bit mono
Mix2Buffer:
	mov bx,[bp+SampleMOD]
	mov dx,[bp+SampleStep]
	mov ax,[bp+volume]
	mov [tVoladr],ax
	push ds,bp
	xor ax,ax
	mov ds,ax
	test b[cs:sdev_bits],128
	jne mix2bufferS 	;8-bit stereo

l1:	movzx bp,b[esi]
	add bp,[cs:tVoladr]
	mov al,[cs:bp]
	add [es:di],al
	add bx,dx
	adc esi,0
	inc di
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; 8 bit stereo
mix2bufferS:
;	 mov al,[cs:DMSchannels]
;	 dec al
;	 cmp [cs:actualchannel],al
;	 jz mix2bufferS_sur
l1:	movzx bp,b[esi]
	add bp,[cs:tVoladr]
	mov al,[cs:bp]
	add [es:di],al
	add bx,dx
	adc esi,0
	add di,2
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; 8 bit stereo surround
mix2bufferS_sur:
l1:	mov ax,8080h		;bytes vorzeichenlos
	movzx bp,b[esi]
	add bp,[cs:tVoladr]
	xor al,[cs:bp]
	xor ah,[es:di-1]
	add al,[es:di]
	push ax
	sar al,surroundshift
	add al,ah
	add al,80h
	mov [es:di-1],al
	pop ax
	sar ah,surroundshift
	add ah,al
	add ah,80h
	mov [es:di],ah
	add bx,dx
	adc esi,0
	add di,2
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; Mix 16bit mono
Mix2Buffer16:
	mov bx,[bp+SampleMOD]
	mov dx,[bp+SampleStep]
	mov ax,[bp+volume]
	mov [tVoladr],ax
	push ds,bp
	xor ax,ax
	mov ds,ax
	test b[cs:sdev_bits],128   ;stereo ?
	jnz mix2buffer16s

l1:	movzx bp,b[esi]
	add bp,bp
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
	add [es:di],ax
	add bx,dx
	adc esi,0
	add di,2
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; Mix 16bit stereo
l10:	jmp mix2buffer16s_sur
mix2buffer16s:			;16-Bit-Ausgabe stereo
	cmp dx,8000h		;Ist das Zooming skalierbar ?
	jb mix2buffer16si	;Ja, interpolieren...
mix2buffer16sb:
	cmp b[cs:DMS_sur],1
	jne >l1
	mov al,[cs:DMSchannels]
	dec al
	cmp [cs:actualchannel],al
	je l10
l1:	movzx bp,b[esi]
	add bp,bp
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
	add [es:di],ax
	add bx,dx
	adc esi,0
	add di,4
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; Mix 16bit stereo oversampled
l10:	jmp mix2buffer16s_suri	;Ja, zur Surround-Routine
mix2buffer16si: 		;16-Bit-Ausgabe stereo
	cmp b[cs:DMS_over],2
	jne mix2buffer16sb	;kein Oversampling
	cmp b[cs:DMS_sur],1
	jne mix2buffer16sib
	mov al,[cs:DMSchannels] ;Letzter Kanal ?
	dec al
	cmp [cs:actualchannel],al
	je l10
mix2buffer16sib:
	add dx,dx		;Zooming verkleinern
l3:	movzx bp,b[esi]
	add bp,bp		;1. Wert holen
	add bp,[cs:tVoladr]
	mov ax,[cs:bp]
l1:	add [es:di],ax	       ;Wert auf den Puffer addieren
	add di,4
	add bx,dx
	jc >l2			;Neuen Wert holen...
	dec cx			;weiter, stop bei Frameende
	jnz l1
l5:	pop bp,ds
	mov [bp+SampleMOD],bx  ;Rest des ScalierungszÑhlers retten.
	ret

	align 10h
l2:	dec cx			;CX korrigieren (letzer Loop verpasst)
	jz l5			;Ende des Frames
	movzx bp,b[esi+1]
	add bp,bp		;nÑchsten Wert holen
	add bp,[cs:tVoladr]
	mov bp,[cs:bp]	       ;und mit Hilfe des alten Wertes einen
	sub bp,ax		;Zwischenwert errechnen
	sar bp,1		;zw=nw+(nw-aw)/2
	add ax,bp

l1:	add [es:di],ax
	add di,4
	add bx,dx
	jc >l4
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

l4:	inc esi
	dec cx			;CX korrigieren
	jz l5
	jmp l3

; Mix 16bit stereo surround
mix2buffer16s_sur:
	cmp dx,8000h		;Ist das Zooming skalierbar ?
	jb mix2buffer16s_suri	;Ja, interpolieren...
mix2buffer16s_surb:
l1:	movzx bp,b[esi]
	add bp,bp
	add bp,[cs:tVoladr]
	push [es:di-2]	       ;Wert vom linken Kanal retten
	mov ax,[cs:bp]	       ;Neuen Wert holen (rechts)
	add ax,[es:di]	       ;vorherigen Wert dazuaddieren
	mov [es:di],ax	       ;und zurÅckschreiben.
	sar ax,surroundshift	;Diesen Wert verkleinert auf den linken
	add [es:di-2],ax       ;Kanal addieren
	pop ax			;Wert vom linken Kanal holen,
	sar ax,surroundshift	;verkleinern
	add [es:di],ax	       ;und auf den rechten Kanal addieren.
	add bx,dx
	adc esi,0
	add di,4
	dec cx
	jnz l1
	pop bp,ds
	mov [bp+SampleMOD],bx
	ret

; Mix 16bit stereo surround oversampled
mix2buffer16s_suri:
	cmp b[cs:DMS_over],2
	jne mix2buffer16s_surb	;Kein Oversampling
	push di,cx
	push mxb1,ds,bp
	jmp mix2buffer16sib	;erstmal normale Mischroutine aufrufen
mxb1:	pop cx,di
	; Jetzt sind die KanÑle komplett. Jetzt nur noch den
	; Surround-Effekt darÅberlegen.
	; (Muss sein, da das nicht in 1 Rutsch zu machen ist)
l1:	push [es:di-2]	       ;Wert vom linken Kanal retten
	mov ax,[es:di]	       ;vorherigen Wert dazuaddieren
	mov [es:di],ax	       ;und zurÅckschreiben.
	sar ax,surroundshift	;Diesen Wert verkleinert auf den linken
	add [es:di-2],ax       ;Kanal addieren
	pop ax			;Wert vom linken Kanal holen,
	sar ax,surroundshift	;verkleinern
	add [es:di],ax	       ;und auf den rechten Kanal addieren.
	add di,4
	dec cx
	jnz l1
	pop bp,ds
	ret

; ================================
; >>> Bestimmt neue FramelÑnge <<<
; ================================
; AX=LÑnge
DMS_defnewoutlen:
	test b[cs:sdev_bits],128
	jz >l1
	add ax,ax
l1:	test b[cs:sdev_bits],16
	jz >l1
	add ax,ax
l1:	mov [cs:DMSnewoutlen],ax
	ret

	.data
DMS_over:	db ?	;Oversampling-Tiefe
DMS_sur:	db ?	;Flag fÅr Surround

sdev_start:	dw ?	;Vector zum starten der Soundkarte.
sdev_stop:	dw ?	;Vector zum stoppen der Soundkarte.
sdev_bits:	dw ?	;Anzahl der benutzten Bits
sdev_maxrate:	dw ?	;Maximal mîgliche Rate.
sdev_rate:	dw ?	;Aktuelle Rate
tVoladr:	dw ?
DMSmode:	db ?	; Bit 0= 8/16 bit
DMSchannels:	dw ?
DMSvoltabsize:	dw ?	;size for 1 volume
DMSvoltabpot:	db ?	;potency of size
DMSmixpos:	dw ?
DMSmixlen:	dw ?
DMSoutlen:	dw ?
DMSnewoutlen:	dw ?
DMSnewmixlen:	dw ?
DMShlp: 	dw ?
DMSresolution:	dw ?	;Aktuelle Samplingrate
actualchannel:	db ?
Voltab: 	db 512*64 dup ?  ;Umrechnungstabelle fÅr LautstÑrken

