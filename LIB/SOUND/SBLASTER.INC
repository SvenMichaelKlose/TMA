; *** Soundblaster routines
; *** (c)1994-1996 Sven Klose
	.code
bufseg	       = 9000h
sbnormal       = 0
sb20	       = 1
sbpro	       = 2
sb16	       = 3
sbcmd16s       = 30h	;, len 16bit stereo
sbcmd16m       = 10h	;,len 16bit mono
sbcmdset16     = 0b4h ; set 16bit mode
sbcmdset16rate = 41h ; set 16 bit sampling rate
; --------------------------------------------------------------------------
; Auf Soundblaster-Karte prfen
; --------------------------------------------------------------------------
GetSB:  mov dx,210h             ;1. Basisport
GetSB1: mov cx,8                ;8 Ports sind m”glich
GetSB2: add dx,6                ;Reset ber Port 2x6h ausl”sen
	mov al,1                ;1 schreiben
	out dx,al
	call sbwt               ;warten
	xor al,al
	out dx,al               ;0 schreiben
	add dx,0ah-6
	call sbwt               ;warten
	in al,dx                ;Wert aus Port 2xeh lesen
	cmp al,0aah             ;Wahrscheinlich Soundblaster ?
	je GetSB3               ;Ja, weiter
	add dx,10h-0ah          ;nein, dx auf n„chsten Basisport
	loop GetSB2
	stc
	ret

cloneerr:
	mov dx,ERR_sbtimeout
	mov ah,9
	int 21h
	stc
	ret

GetSB3: sub dx,0ah
	mov w[SBPort],dx      ;ganze Portnummer fr Soundroutine speichern
	shr dl,4              ;Portnummer in Text schreiben
	add dl,"0"
	mov b[TXT_sbport],dl
	mov dx,TXT_sb         ;"SoundBlaster " ausgeben
	mov ah,9
	int 21h

; *** Sounblaster-Version feststellen
	mov al,0e1h           ;Version abfragen
	call WriteSB
	call ReadSB
	mov ah,al
	call ReadSB
	cmp ax,0aaaah
	je cloneErr
	cmp ax,105h             ;SoundBlaster/MCV
	jae NoMCV
	mov dx,TXT_orMCV
	mov cx,sbnormal by 8
	mov bx,21379
	jmp VersionFound
NoMCV:  cmp ah,2                ;Soundblaster 2.0
	ja NoSB20
	mov dx,TXT_sb20
	mov cx,sb20 by 8
	mov bx,21379
	jmp VersionFound
NoSB20: cmp ah,3
	jne NoSBpro		;Soundblaster Pro
	mov dx,TXT_sbpro
	mov cx,sbpro by 8+128
	mov bx,48000
	jmp VersionFound
NoSBpro:cmp ah,4		;Soundblaster 16
	jne NoSB16
	mov dx,TXT_sb16
	mov cx,sb16 by 8
	mov bx,48000
	cmp al,0dh
	jne noawe32
	mov dx,TXT_awe32
noawe32:jmp VersionFound
NoSB16: mov dx,TXT_SBbetter
VersionFound:		;Name ausgeben
	mov b[sdev_bits],cl
	mov b[sbcardtype],ch
	mov w[sdev_maxrate],bx
	mov ah,9
	int 21h

; ***********************
; *** DMA & IRQ holen ***  
; ***********************
;	 call SpeakerOff	;...damit User nicht die Ohren wegfliegen...
;	 mov di,SBoldirqtab
;	 mov al,3+8
;	 call SBreadoldvec
;	 mov al,5+8
;	 call SBreadoldvec
;	 mov al,7+8
;	 call SBreadoldvec
;	 mov ax,1
;	 mov w[DMAbufferlen],ax
;	 mov w[MODinputsize],ax
;	 in al,21h		 ;enable irqs
;	 and al,1010111b	;irq 3/5/7
;	 out 21h,al
;	 in al,0a1h		 ;irq 10
;	 and al,11111011b
;	 out 0a1h,al
;	 sub al,al
;	 call WaitforSBDMA
;	 jc sbaction
;	 mov al,1
;	 call WaitforSBdma
;	 jc sbaction
;	 mov al,3
;	 call WaitforSBdma
;	 jne getblastervar
; ----- DMA and IRQ are found...restore vectors now
;sbaction:
;	 jmp getblastervar
;	 mov si,SBoldirqtab
;	 mov al,3+8
;	 call sbWriteoldback
;	 mov al,5+8
;	 call sbWriteoldback
;	 mov al,7+8
;	 call sbwriteoldback
;	 in al,21h
;	 and al,10101000b
;	 out 21h,al
;	 in al,0a1h
;	 and al,100b
;	 out 0a1h,al
;	 jmp gotsb
; ----- Hardware detection failed, now try to get the BLASTER variable
getblastervar:
	in al,21h
	and al,10101000b
	out 21h,al
	in al,0a1h
	and al,100b
	out 0a1h,al
	mov cx,8000h	      ;BLASTER-Variable holen
	mov es,w[cs:2ch]      ;Segment des Environmentblocks
	push cs
	pop ds
	xor di,di
GetSBID:push cx,di
	mov si,SBvar
	mov cx,9
	repe cmpsb
	or cx,cx
	jz GotSBvar
	pop di,cx
	inc di
	loop GetSBID
nosbvar:mov dx,TXT_noSBvar
	mov ah,9
	int 21h
	stc
	ret

GotSBvar:
	pop ax,ax	      ;Stack wiederherstellen
	mov si,di
	push es
	pop ds
	xor dl,dl
GetBlas:cmp dl,3	      ;IRQ&DMA gefunden ?
	je GotSB	      ;Ja,Ende
	lodsb
	cmp al,"i"
	je GetSBirq
	cmp al,"I"
	jne NoSBirq
GetSBirq:
	lodsb
	sub al,"0"
	mov b[cs:SBirqnr],al
	mov b[cs:irqnr],al
	or dl,1
	jmp GetBlas
NoSBirq:cmp al,"d"
	je getsbdma
	cmp al,"D"
	jne GetBlas
GetSBdma:
	lodsb
	sub al,"0"
	mov b[cs:SBDMAnr],al
	or dl,2
	jmp GetBlas
GotSB:	push cs
	pop ds
	mov dx,TXT_sbfound    ;Meldung ausgeben, daá Karte gefunden wurde
gotmozartsbmode:
	mov al,b[sbirqnr]
	add al,"0"
	mov b[TXT_sbirq],al
	mov al,b[sbdmanr]
	add al,"0"
	mov b[TXT_sbdma],al
	mov ah,9
	int 21h
	mov ax,sb_start 	;Sprungvectoren fr Hauptroutinen
	mov w[sdev_start],ax
	mov ax,sb_stop
	mov w[sdev_stop],ax
	clc
	ret
sb_start:
	call startsb
	call speakeron
	ret
sb_stop:
	call stopsb
	call speakeroff
	ret

WaitforSBdma:
	mov b[sbdmanr],al
	mov b[sbirqnr],0
	push ax
	call sbreset
	pop ax
	call StartSB2
	call WaitforSBIRQ
	pushf			;save carry flag !
	call StopSB2
	popf
	ret

WaitforSBirq:
	mov cx,2
WaitforSBirq2:
	call retrace
	cmp b[sbirqnr],0
	jne sbwaited	;irq number has been stored by the int. routine
	loop waitforsbirq2
	clc		;time out, no sb action detected
	ret
sbwaited:
	stc
	ret

SBreadoldvec:
	push ax
	mov ah,35h	;alten Vector lesen
	int 21h
	mov ax,bx
	cs:stosw
	mov ax,es
	cs:stosw
	mov dx,SBrecint
	push cs
	pop es,ax
	mov ah,25h
	int 21h
	ret

SBwriteoldback:
	mov bl,al	;alten Vector zurckschreiben
	lodsw
	mov dx,ax
	lodsw
	mov es,ax
	mov al,bl
	mov ah,25h
	int 21h
	ret

; **************************
; *** Erkennt SB-IRQ-Nr. ***
; **************************
SBrecint:
	cli
	pusha
	xor bl,bl
	mov al,10	;ISR lesen
	out 20h,al
	in al,20h
	and al,10101000b
	jnz IRQfound
	mov bl,8
	mov al,10
	out 0a0h,al
	in al,0a0h
	and al,100b
	jz noirqx
IRQfound:
	mov cx,8
sbrec2: inc bl
	shr al,1
	jc sbrec3
	loop sbrec2
sbrec3: dec bl
	mov b[cs:SBirqNr],bl
noirqx: mov al,20h
	out 20h,al
	cmp bl,8
	jb noirq7b
	out 0a0h,al
noirq7b:popa
	iret

; ********************************
; *** Gibt Kommando an den DSP ***
; ********************************
WriteSB:push ax,cx
	mov cx,-1
	mov dx,w[cs:SBport]
	or dl,0ch
	mov ah,al
WriteSB2:
	in al,dx
	or al,al
	jns WriteSB3
	loop WriteSB2
	pusha
	push ds,cs
	pop ds
	mov dx,ERR_sbtimeout
	mov ah,9
	int 21h
	pop ds
	popa
	ret
WriteSB3:
	mov al,ah
	out dx,al
	pop cx,ax
	ret

ReadSB: push dx
	mov dx,w[cs:SBport]
	or dl,0eh
readsb2:in al,dx
	or al,al
	jns readsb2
	sub dl,0eh-0ah
	in al,dx
	pop dx
	ret

; waits...
sbwt:	push ax,cx
	mov cx,80h
sbwt2:	in al,64h
	loop sbwt2
	pop cx,ax
	ret

; ********************************
; *** Lautsprecher einschalten ***
; ********************************
SpeakerOn:
	mov dx,w[sbport]	;Filter abschalten
	or dl,4
	mov al,0eh
	out dx,al
	inc dl
	in al,dx
	or al,32
	out dx,al
	dec dl			;Lautst„rke voll aufdrehen
	mov al,2
	out dx,al
	inc dl
	mov al,11101110b	;DSP-Lautst„rke max.
	out dx,al
	dec dl
	mov al,22h		;allg. Lautst„rke
	out dx,al
	inc dl
	mov al,11101110b
	out dx,al
	dec dl
	mov al,2eh		;Line aufdrehen
	out dx,al
	inc dl
	mov al,11101110b
	out dx,al
	mov al,0d1h		;Lautsprecher einschalten
	jmp WriteSB

; *******************************
; *** Lautsprecher abschalten ***
; *******************************
SpeakerOff:
	mov al,0d0h
	jmp WriteSB

; ******************
; *** Resets DSP ***
; ******************
sbReset:mov dx,w[cs:sbport]	;DSP resetten
	add dl,6
	mov al,1
	out dx,al
	mov cx,8
l1:	in al,dx
	loop l1
	xor al,al
	out dx,al
	add dl,0Eh-6		      ; SB data available port 2xEh
	mov cx,1000

l1:	in al,dx
	or al,al
	js >l2			 ; wait until bit 7 (data available)
	loop l1 		; is 1 or 1000 times
;	 jmp @@err		     ; no data - no SB

l2:	sub	dl,0Eh-0ah		; read data port (2xAh)
	mov	cx,1000
l1:	in	al,dx
	cmp	al,0AAh 		; wait until data is 0AAh or 1000
	je	>l2			; times
	loop	l1
;	 jmp	 @@err			 ; no 0AAh - no SB
l2:	ret

; ************************
; *** Interrupthandler ***
; ************************
SBirq:	push ax,bx,dx,ds,cs
	pop ds
	mov dx,w[SBport]	;DSP auf naechsten Block hinweisen...
	or dl,0eh		;(8 bit)
	cmp b[DMSmode],1
	jne sbirq8
	inc dl			;(16 bit)
sbirq8: in al,dx
	mov al,20h		;EOI an Interruptcontroller
	out 20h,al
	cmp b[SBIRQNR],7	;IRQ 10 ?
	jna NoIRQ7
	out 0a0h,al
NoIRQ7: pop ds,dx,bx,ax
	iret

; ***************************************************
; *** Startet Soundausgabe der Soundblaster-Karte ***
; ***************************************************
StartSB:call sbreset
	mov ax,sbtimingcheck
	mov w[cs:modtimingcheck],ax
	mov ax,cs
	mov w[cs:irqseg],ax		;IRQ-Handler
	mov w[cs:irqadr],sbirq
	mov al,b[cs:irqnr]
	mov al,b[cs:sbirqnr]
	call SwitchIRQ			;Interrupt verbiegen
	mov b[cs:sbirqnr],al
	call SetIRQ			;Interrupt erlauben
	mov ds,cs
	xor ax,ax
	mov w[DMAbufferAdr],ax
	mov w[DMAbufferlen],ax
	mov ax,bufseg
	mov w[DMAbufferseg],ax
	mov al,b[sbdmanr]
StartSB2:
	call StartDMA
	mov al,b[cs:sbcardtype]
	cmp al,sb20
	jb >l1
	cmp al,sb16
	je >l2			;SB 16

	; SoundBlaster Pro
	mov al,40h			;Samplerate einstellen
	call WriteSB
	mov eax,1000000 	;4240h
	xor ebx,ebx
	mov bx,w[cs:DMSresolution]
	xor edx,edx
	div ebx
	neg al
	call WriteSB
l4:	mov bx,-1
	mov al,48h			;Blockgr”sse des DSP einstellen
	call WriteSB
	mov al,bl
	call WriteSB
	mov al,bh
	call WriteSB
	mov al,90h			;Sample spielen (Autoinit) ab SB pro
	call WriteSB
	ret

	;SoundBlaster 16
l2:	pop ax
	call StartDMA
	mov al,sbcmdset16rate
	call writesb
	mov bx,w[cs:DMSresolution]
	mov al,bh
	call writesb
	mov al,bl
	call writesb
	test b[cs:sdev_bits],16
	jz l4			;kein 16bit mode...zur pro-routine
	mov al,sbcmdset16
	call writesb
	mov al,sbcmd16s 	;stereo
	test b[cs:sdev_bits],128
	jne >l5
	mov al,sbcmd16m 	;mono
l5:	call writesb
	mov al,-1
	call writesb
	mov al,-1
	call writesb
	ret

	;SoundBlaster <= 2.0
l1:	pop ax
	call startDMA
	mov al,14h		;Sample spielen (bis SB 2.0)
	call writesb
l3:	mov al,-1		;Blockgroesse
	call writesb
	call writesb
	ret

; ********************************
; *** Schaltet Soundblaster ab ***
; ********************************
StopSB: push ds,es
	mov al,b[cs:sbirq]
	call SwitchIRQ		;Interruptvektor wieder herstellen
	call LockIRQ		;und Interrupt deaktivieren
	call sbReset		;Karte resetten.
	call sbReset
	call sbReset
	call sbReset
	call StopSB2
	pop es,ds
	ret
StopSB2:push cs
	pop ds
	call SpeakerOff 	;Lautsprecher ausschalten
	mov al,b[sbdmanr]
	call StopDMA
	ret

; =======================================
; >>> framecheck for playing routines <<<
; =======================================
sbtimingcheck:
	mov al,b[sbdmanr]
	call GetDMApos
	mov bx,ax
	mov ax,w[modoldmixpos]
	sub ax,bx
	mov bx,w[DMSoutlen]
	cmp ax,bx
	ja >l1
	clc	;no frame to be played
	ret
l1:	stc	;frame needed
	ret

sbmode: db ? ;sb8

#if !_mdltiny
_text   segment
#endif

SBvar:db"BLASTER=",0

TXT_noSBvar:	db"Can't find BLASTER environment variable.",10,13,"$"
TXT_SB: 	db"Soundblaster $"
TXT_orMCV:	db"or Soundblaster MCV$"
TXT_sb20:	db"2.0$"
TXT_SBpro:	db"Pro$"
TXT_SB16:	db"16$"
TXT_AWE32:	db"16 AWE 32$"
TXT_SBbetter:	db"16 or higher version$"
TXT_SBfound:	db" or compatible found at port "
TXT_sbfound2:	db"2"
TXT_sbport:	db"?0 using IRQ "
TXT_sbirq:	db"? and DMA "
TXT_sbdma:	DB"?.",10,13,"$"
ERR_sbtimeout:	db"Soundblaster clone timeout error !",10,13,"$"

	.data
SBPort:dw ?
SBIRQNR:db ?
SBDMANR:db ?
sbcardtype:db ?

