; ******************************
; *** Player f�r S3M-Dateien ***
; *** (nur mit AdLib-Kan�len ***
; ***   (c)1995 Sven Klose   ***
; ******************************


Ordnum  = 20h
InsNum  = 22h
Orders  = 60h
adlib=388h
S3Mspd=6
StartS3M:
	push ds,es
	mov ds,cs
	mov es,cs
	mov bx,w[S3Mbase+OrdNum] ; 1.Patternsegment holen
	add bx,S3Mbase+Orders-2
	mov w[InsPtrs],bx        ; Instrumententabelle
	mov ax,w[s3mbase+InsNum]
	shl ax,1
	add bx,ax
	add bx,2
	mov w[PatPtrs],bx
	mov w[OrderAdr],s3mbase+orders
	mov b[PatLine],1
	mov ax,00bdh
	call out1
	mov ax,4008h
	call out1

	cli
	xor ax,ax
	mov es,ax
	mov ax,cs
	db 66h
	shl ax,16
	mov ax,PlayS3M
	db 66h
	xchg ax,w[es:8*4]
	db 66h
	mov w[S3Moldint],ax

	mov al,110110b
	out 43h,al
	mov al,low(1193180/100)
	out 40h,al
	mov al,high(1193180/100)
	out 40h,al
	mov b[S3McntOld],100/18
	mov b[S3McntTmr],100/S3Mspd
	sti

	mov ah,0ah              ;Routine in Reset einbinden
	call S3MaWr
	mov ax,40h
	mov es,ax
	mov w[es:67h],S3Mreset
	mov w[es:69h],cs
	pop es,ds
	ret

S3Mreset:
	mov ds,cs
	mov cx,9
	mov b[ChannelNr],0
S3Mr2:  call SetVol
	inc b[ChannelNr]
	loop s3mr2

	xor ah,ah
	call S3Mawr
	call S3MclearVoices
	jmp 0f000:0fff0h

S3Mawr: mov al,8fh
	out 70h,al
	jmp $+2
	jmp $+2
	jmp $+2
	mov al,ah
	out 71h,al
	ret

StopS3M:push ds,es
	mov ds,cs
	cli
	xor ax,ax
	mov es,ax
	db 66h
	mov ax,w[S3Moldint]
	db 66h
	mov w[es:8*4],ax
	mov al,110110b
	out 43h,al
	xor al,al
	out 40h,al
	out 40h,al
	sti
	call S3MclearVoices
	xor ah,ah               ;Resetroutine ausschalten
	call s3mawr
	pop es,ds
	ret

; *******************************
; *** Schaltet alle Kan�le ab ***
; *******************************
s3mClearVoices:
	mov bx,4063h
	call s3mcv3
	mov bx,0b000h
	call s3mcv3
	mov bx,80ffh
	call s3mcv3
	mov bx,9000h
	call s3mcv3
	ret

S3mcv3: mov cx,18
	mov bp,chanorder
	mov dx,adlib

s3mcv2: mov al,b[bp]
	inc bp
	add al,bh
	out dx,al
	call fmwait
	push ax
	mov al,bl
	inc dx
	out dx,al
	call fmwait
	dec dx
	pop ax
	inc al
	loop s3mcv2
	ret

; **********************
; *** Abspielroutine ***
; **********************
PlayS3M:cli
	pusha
	push ds,es
	mov ds,cs
	mov es,cs
	dec b[S3McntTmr]
	jz Play2
	jmp playeol1

play2:  mov b[s3mcnttmr],100/s3mspd

	dec b[PatLine]          ;Neues Pattern ?
	jnz play3               ;Nein,weiter ...
	
NewPat: mov b[Patline],64
	mov bx,w[OrderAdr]      ;Nummer des neuen Patterns
	inc w[OrderAdr]
	mov al,b[bx]
	cmp al,-1               ;Von vorne anfangen ?
	jne Nonewtune           ;Nein,weiter
	mov bx,s3mbase+orders
	mov w[OrderAdr],bx
	jmp NewPat
NoNewtune:
	xor ah,ah
	shl ax,1
	mov bx,w[PatPtrs]
	add bx,ax
	mov ax,w[bx]            ;Segment des neuen Pattern
	shl ax,4
	add ax,s3mbase+2
	mov w[PatAdr],ax 

; komplette Zeile spielen
Play3:  mov si,w[PatAdr]        ;Adresse der Zeile holen

getnocom:
	lodsb
	or al,al                ;Ende der Zeile ?
	jz playEoL              ;Ja ...
	mov ah,al
	and ah,15               ;Kanalnummer holen
	mov b[ChannelNr],ah
	test al,32
	jz GetNoNote
	call GetNote
getnonote:
	test al,64
	jz getnovolume
	lodsb
	dec al
	mov ah,al
	and ah,63
	xor ah,63
	call SetVol
getnovolume:
	test al,128
	jz getnocom
	add si,2
	jmp getnocom
	
playeol:mov w[PatAdr],si
playeol1:
	pop es,ds
	popa
	dec b[cs:S3McntOld]
	jnz playeol2
	mov b[cs:S3McntOld],100/18
	jmp far [cs:S3MoldInt]
playeol2:
	push ax
	mov al,20h
	out 20h,al
	pop ax
	sti
	iret

; *************************************************
; *** Setzt die Lautst�rke des aktuellen Kanals ***
; *************************************************
; AL=Lautst�rke
SetVol: pusha
	push ax
	mov al,b[ChannelNr]
	xor ah,ah
	shl al,1
	add ax,Chanorder
	mov bp,ax
	pop ax
	call out4
	call out4
	popa
	ret

out4:   mov dx,adlib
	mov al,40h
	add al,b[bp]
	inc bp
	out dx,al
	call fmwait
	inc dx
	mov al,ah
	out dx,al
	call fmwait
	ret

; **********************************
; *** �bergibt Note & Instrument ***
; **********************************
GetNote:push ax
	cmp b[si],254           ;Noteoff ?
	jne gn2
	mov ah,63
	call SetVol
	add si,2
	pop ax
	ret

gn2:    mov ax,0b0h             ;Ton abschalten
	call out1
	
	mov al,b[si+1]          ;Instrument holen
	push si
	xor ah,ah
	shl ax,1
	mov bx,w[InsPtrs]       ;Adress des Instrument-Infoblocks
	add bx,ax
	mov si,w[bx]
	shl si,4
	add si,s3mbase+10h
	
	call InsOut             ;Instrument �bergeben
	
	mov dx,adlib
	mov al,0c0h             ;FB/FM
	mov bl,b[Channelnr]
	shl bl,1
	xor bh,bh
	add al,b[bx+chanorder]
	out dx,al
	call fmwait
	lodsb
	inc dx
	out dx,al
	call fmwait
	pop si
	
	mov al,b[si]
	mov cl,al
	shr cl,2                ;Oktavennummer
	and cl,11100b
	and al,0fh              ;Notenfrequenz aus Tabelle holen
	mov bl,al
	xor bh,bh
	shl bx,1
	mov ax,w[Frequencies+bx]
	push ax
	mov ah,al
	mov al,0a0h             ;Bits 0-7 der Frequenz schreiben
	call out1
	pop ax
	or ah,cl                ;Bits 8-9 merken
	or ah,32                ;Key bit
	mov al,0b0h
	call out1
	
	add si,2                ;Zeiger auf n�chsten Eintrag im Pattern
	pop ax
	ret

; *****************************************************
; *** �bergibt Instrumentendaten an aktuellen Kanal ***
; *****************************************************
InsOut: mov bp,ChanOrder
	mov al,b[channelnr]
	shl al,1
	xor ah,ah
	add bp,ax
	mov al,20h              ;AM/VIB/EG-Typ/KSR/MULTI
	call out2
	mov al,40h              ;KSL/TL
	call out2
	mov al,60h              ;AR/DR
	call out2
	mov al,80h              ;SL/RR
	call out2
	mov al,0e0h             ;Wave-Select
	call out2
	ret

; Aufteilung in Modulator und Carrier
out2:   push bp
	push ax
	call out3       ;mod
	pop ax
	call out3       ;car
	pop bp
	ret

Out3:   mov dx,adlib            ;Portadresse der Adlib-Karte
	add al,b[bp]            ;Register
	inc bp                  ;Zeiger auf n�chstes Register
	out dx,al               ;Register anw�hlen
	call fmwait
	lodsb                   ;Wert f�r Register holen und rausschmeissen
	inc dx
	out dx,al
	call fmwait
	ret

; ***********************************************
; *** �bergibt Wert an lineare Registergruppe ***
; ***********************************************
; AL=Registergruppe, AH=Wert
out1:   add al,b[channelnr]
	mov dx,adlib
	out dx,al
	call fmwait
	mov al,ah
	inc dx
	out dx,al
	call fmwait
	ret

fmwait: push ax,cx,dx
	mov cx,0ah
fmwaitb:in al,dx
	loop fmwaitb
	pop dx,cx,ax
	ret

; ***********************************
; *** Frequenzen der Adlib-Kan�le ***
; ***********************************
Frequencies:
	dw 343,363,385,408,432,458,485,514,544,577,611,647

Chanorder:
	db 00h,03h,01h,04h,02h,05h,08h,0bh,09h
	db 0ch,0ah,0dh,10h,13h,11h,14h,12h,15h

; *************
; *** Daten ***
; *************
InsPtrs:        dw ?
PatPtrs:        dw ?
PatLine:        db
OrderAdr:       dw ?
PatAdr:         dw ?
ChannelNr:      db
S3Mcnttmr:      db
S3McntOld:      db
S3Moldint:      dd ?
