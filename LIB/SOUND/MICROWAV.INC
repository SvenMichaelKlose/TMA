; *********************
; * >>> Microwave <<< *
; *********************
; 4-channel PC-speaker
        .code

db"Microwave V1.0 - (c) 1995 Deepware, Bielefeld/Written by Sven Michael Klose"

initMicro:
	ret
	pusha           ;Interrupt initialisieren
	push ds,es
	mov ds,cs
	cli
	xor ax,ax     ;*o*
	mov es,ax
	mov ax,MicroSine
	mov dx,cs
	xchg ax,w[es:8*4]
	xchg dx,w[es:8*4+2]
	mov w[MicroOld],ax
	mov w[MicroOld+2],dx
	mov al,0b6h      ;Samplingrate 
	out 43h,al
	mov al,144     
	out 40h,al      ;(INT 08H)
	xor al,al
	out 40h,al
	mov al,10010000b ;Timer auf Empfang
	out 43h,al
	in al,61h       ;Lautsprecher anschalten
	or al,3
	out 61h,al
	pop es,ds
  
	mov es,cs
	mov di,Square	;S„gezahnschwingung schreiben
	mov al,128
	mov cx,512
	push cx
	rep stosb
	pop cx
	mov al,1
	rep stosb
	popa
	sti
	ret

MicroOff:
	ret
	pusha           ;Interrupt wieder herstellen
	push es
	cli
	xor ax,ax     ;*O*
	mov es,ax
	mov ax,w[MicroOld]
	mov dx,w[MicroOld+2]
	mov w[es:8*4],ax
	mov w[es:8*4+2],dx
	mov al,36h	;Samplingrate 18.2Hz 
	out 43h,al
	xor al,al   
	out 40h,al      ;(INT 08H)
	out 40h,al
	in al,61h       ;Lautsprecher aus
	and al,-3
	out 61h,al
	pop es
	popa
	sti
	ret

MicroSine:
	push ax,bx,dx,ds
	mov ds,cs
	mov ah,dh

	mov bx,w[MicroSpos]     ;Track 1
	mov al,b[bx+square]
	add bx,w[MicroFreq]
	and bh,3
	mov w[MicroSpos],bx
  
	mov bx,w[MicroSpos+10h] ;Track 2
	add al,b[bx+square]
	add bx,w[MicroFreq+10h]
	and bh,3
	mov w[MicroSpos+10h],bx
  
	;mov bx,w[MicroSpos+20h];Track 3
	;add al,b[bx+triangle]
	;add bx,w[MicroFreq+20h]
	;and bx,1023
	;mov w[MicroSpos+20h],bx
  
	;mov bx,w[MicroSpos+30h];Track 4
	;add al,b[bx+triangle]
	;add bx,w[MicroFreq+30h]
	;and bx,1023
	;mov w[MicroSpos+30h],bx
  

	shr al,1
	out 42h,al
;       push ax
;	mov al,10h
;       call WriteSB
;       pop ax
;       call WriteSB
	
	pop ds,dx,bx
	mov al,20h
	out 20h,al
	pop ax
	iret

MicroOld:dw 0,0

MicroOK:
	pop ds
	popa
	ret

MicroNewTrack2:
	jmp MicroNewTrack

MicroMusic:
	pusha
	push ds
	mov ds,cs
	inc b[MicroWait]
	mov al,b[MicroWait]
	cmp al,4
	je MicroGotIt
;        cmp w[MicroFreq],0
;        je MicroOk3
;        cmp al,1
;        jne Micro13
;        dec w[MicroFreq]
;        jmp MicroOk3
;Micro13:cmp al,2
;        jne Micro14
;        add w[MicroFreq],2
;	 jmp MicroOk3
;Micro14:cmp al,3
;        jne MicroOk3
;        dec w[MicroFreq]
;MicroOk3:       
	jmp MicroOk

MicroGotIt:
	mov b[MicroWait],0
  
MicroAgain:
	mov bp,w[TrackCounter]
	cmp bp,10h      ;Staccato fr Track 2
	jne NoStac
	push w[MicroFreq+bp],w[Staccato]
	pop w[MicroFreq+bp],w[Staccato]
NoStac: dec w[bp+MicroCount]    ;Tonende ?
	mov ax,w[bp+MicroCount]
	or ax,ax
	jnz MicroNewTrack2      ;Nein, Ende
	mov si,w[bp+MicroNext]

Micro0: lodsb
	mov w[bp+MicroNext],si
	cmp al,"o"      ;Oktave „ndern ?
	jne Micro2
	call HexConv
	mov b[bp+MicroOctave],al
	jmp Micro0

Micro2: cmp al,"l"      ;Notenl„nge „ndern ?
	jne Micro3
	call HexConv
	call microGetLength
	mov w[bp+MicroLength],ax
	jmp Micro0

Micro3: cmp al,"."      ;Pause (einfach)
	jne Micro4
	mov al,2
	jmp Micro3b
	cmp al,"p"      ;Pause ?
	jne Micro4
	call HexConv
	call microGetLength
Micro3b:mov b[bp+Microlength],al
	mov b[bp+MicroCount],al
	xor ax,ax
	mov w[bp+MicroFreq],ax
	jmp MicroNewTrack

Micro4: cmp al,"v"      ;Lautst„rke ?
	jne Micro10
	call Hexconv
	mov b[bp+MicroVolume],al
	jmp Micro0

Micro10:or al,al        ;Ende des Tracks ?
	jz MicroOff2    ;Ja,Routine ausschalten
	cmp al,"c"
	jge Micro11
	sub al,"a"-5    ;Nein,Note holen
	jmp Micro12
Micro11:sub al,"c"
Micro12:xor ah,ah
	shl ax,1
	cmp b[si],"#"
	jne Micro6
	inc w[bp+MicroNext]
	add ax,MicroTabH;Halbnote holen
	jmp Micro5
Micro6: add ax,MicroTabN;Wert aus Tabelle
Micro5: mov si,ax       ;Adresse der Note in der Tabelle
	mov al,b[bp+MicroOctave];Oktave berechnen
	inc al
	xor ah,ah
	or al,al
	jz NoOctave
	mov bx,ax       ;*14
	shl ax,1
	shl bx,2
	add ax,bx
	shl bx,1
	add ax,bx
	shl ax,1
nooctave:        
	add si,ax
	mov ax,w[si]
	mov w[bp+MicroFreq],ax;und Frequenz speichern
	mov ax,w[bp+MicroLength]
	mov w[bp+MicroCount],ax
  
MicroNewTrack:
	mov ax,w[TrackCounter]  ;N„chsten Track anw„hlen 
	add ax,16
	and ax,30h
	mov w[TrackCounter],ax
	;or ax,ax
	cmp al,20h
	jz MicroOk2
	jmp MicroAgain

MicroOff2:
	push si,di,cx,cs,cs
	pop es,ds
	mov cx,16
	mov si,offset MicroStab
	mov di,offset MicroVolume
	db 66h
	rep movsw
	pop cx,di,si
MicroOk2:
	xor ax,ax
	mov w[Trackcounter],ax
	jmp MicroOk

MicroGetLength:         ;Rechnung Notenl„nge um
	mov bx,ax
	xor bh,bh
	or bl,bl
	jnz MicroGetl2
	mov bl,16
MicroGetL2:
	xor dx,dx
	mov ax,32
	div bx
	ret

HexConv:lodsb
	cmp al,'0'
	jl MicroError     ;illegale Angabe
	cmp al,"9"
	jg Hexconv2
	sub al,"0"
	ret
HexConv2:
	sub al,"a"-10
	ret

MicroError:    
	mov dx,MicroErr1
	mov ah,9
	int 21h
	ret

; *******************************
; *** Daten fuer Playroutinen ***
; *******************************
MicroErr1:
db"Microwave: Illegal constant at track data !$"

; tempor„re Daten fr Tracks 1-4
MicroFX1:db 21h
Staccato:dw 0
MicroWait:db 0
MicroOffFlag:db 0
TrackCounter:dw 0

MicroVolume:dw 0
MicroVcount:dw 15
MicroLength:dw 0
MicroCount:dw 1
MicroFreq:dw 0
MicroOctave:dw 0
MicroNext:dw Track1
MicroSpos:dw 0
  dw 0,15,0,1,0,0,Track2,0
  dw 0,15,0,1,0,0,Track3,0
  dw 0,15,0,1,0,0,Track4,0
MicroStab:
  dw 0,15,0,1,0,0,Track1,0
  dw 0,15,0,1,0,0,Track2,0
  dw 0,15,0,1,0,0,Track3,0
  dw 0,15,0,1,0,0,Track4,0

; **************
; * Frequenzen *
; **************
MicroTabN:
dw 4,4,5,5,6,7,7
MicroTabH:
dw 4,4,0,5,6,7,0
dw 8,9,10,11,12,14,15,8,9,0,11,13,14,0
dw 16,18,21,22,25,28,31,17,19,0,23,26,29,0
dw 33,37,42,44,50,56,63,35,39,0,47,53,59,0
dw 66,75,84,89,100,112,126,70,79,0,94,106,119,0
dw 133,150,168,178,200,225,252,141,159,0,189,212,238,0
dw 267,300,337,357,401,450,505,283,318,0,378,425,477,0
dw 535,601,675,715,802,901,1011,567,637,0,757,850,954,0




