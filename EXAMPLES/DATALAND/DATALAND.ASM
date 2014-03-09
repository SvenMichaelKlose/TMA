; ==============================================
; >>> Dataland BBS advertisement by Spaceman <<<
; >>>	Coding & GFX/Spaceman Music/M.C.D.   <<<
; >>>	  Assemble with A386 or DASM 1.0     <<<
; ==============================================


	.data
datastart:
FXcolortab:	db 16*3 dup ?
Oldcolors:      db 768 dup ?
Newcolors:      db 768 dup ?

	align 20h
LineTabM:
LineTab:	dw 320 dup ?
		dw ?

	align 20h
LineTabS:	dw 320*10 dup ?

CharsetAdr	dw ?	;Adresse des 8*8 Pixel Zeichensatzes
CharsetSeg	dw ?

MBseg           dw ?
MBsegA          dw ?
MBsegB          dw ?
MBmode          db ?
CHseg           dw ?
MBcolorseg      dw ?
MBcolorseg2     dw ?

TextAdr         dw ?
TextMode        db ?
TextLine        dw ?
TextPosX        dw ?
TextLen         db ?
TextofsX        db ?
TextHelp        dw ?
oldss           dw ?
MBmode0         dw ?     ;Single screen
MBmode1         dw ?     ;double screen
MBmode2         dw ?     ;double screen (shaded)
MBmode3         dw ?     ;single screen (shaded)
MBzmode         db ?
chkoldint       dd ?
tmrcnt		dw ?

LinePosX        dw ?
LinePosY        dw ?
SlopeX          dw ?
SlopeY          dw ?
MBzoom          dw ?            ;Skalierung des Screens; fractional Int (8:8bit)
				;100h=kein zooming
MBdispAdr       dw ?            ;Aktuelle Position auf Bitmap
MBworkadr       dw ?            ;Zielposition auf dem Bildschirm
MBdispSeg       dw ?
MBworkSeg       dw ?
MB_workadr      dw ?
MB_dispadr      dw ?
MBjumptab:      dw 4 dup ?
MasterLineDiff  dw ?
pgcunt          db ?
pghlp           db ?


PatBuffer:      dw 12 dup ?     ;Puffer fÅr Muster auf DATALAND-Schriftzug
slopes:         dw 180 dup ?
dataend:

; Variablen fÅr Dreheffekt
height          = 200           ;Hîhe des Bildschirmes
CM_movalbbx     = 878ah         ;mov al,[bx+disp]
cm_movahbbx     = 0a78ah        ;mov ah,[bx+disp]
CM_stosb        = 0aah          ;stosb
CM_stosw        = 0abh          ;stosw
CM_ret          = 0c3h          ;retn

CM_oral1        = 0a64h         ;or al,b[fs:di]
CM_oral2        = 05h
CM_orah1        = 0a64h         ;or al,b[fs:di]
CM_orah2        = 25h
CM_orax1        = 0b64h         ;or ax,b[fs:di]
CM_orax2        = 05h
patstart        = 180*3
Linestart       = 32
LineEnd         = LineStart+128

HeadPos         = 50 by 255-80
fxsize          = 12

Dataland:
; ***********************
; *** Initialisierung ***
; ***********************
	.code
Init:   mov ax,cs
	mov es,ax
	mov ds,ax
	mov di,DataStart
	mov cx,DataEnd-DataStart
	xor al,al
	rep stosb

	mov ax,4000h
	mov lineposy,ax
	mov mbworkadr,ax
	mov w lineposx,-6000h
	mov w slopex,100h
	mov w mbzoom,80h
	mov w mbdispseg,0a000h
	mov w mbworkseg,0a400h
	mov w masterlinediff,90

	call SetVideoMode
	mov ax,13
	int 10h
	mov ax,13h
	int 10h
	call BlankScreen

	mov di,slopes           ;Sinustabelle erzeugen
	mov bx,-1
	mov cx,1
	xor dx,dx
	mov bp,10000h/256*90
l1:     add dx,bp
	jnc >l2
	xor ah,ah
	mov al,bh
	inc ax
	stosw
	add di,2
l2:     sub bx,cx
	sub bx,cx
	inc cx
	cmp cx,100h
	jne l1

	mov si,slopes
	mov di,slopes+360-2
	mov cx,90
l1:     mov ax,[si]
	add si,4
	mov [di],ax
	sub di,4
	loop l1

	mov si,IMG_start        ;Korrektur der Farben
	mov di,si
	mov cx,IMG_end-IMG_start
i2:     lodsb
	cmp al,-1
	jne >i3
	mov al,1
	jmp >i4
i3:     cmp al,-2
	jne >i4
	xor al,al
i4:     stosb
	loop i2

	mov ax,1130h            ;Adresse des Zeichensatzes holen
	mov bh,6
	int 10h
	mov cs:CharsetAdr,bp
	mov cs:CharsetSeg,es

	mov ax,cs
	add ax,1000h
	mov MBseg,ax
	add ax,1000h
	mov MBcolorseg,ax ;Segment fÅr Farben bei Motion Blur
	mov MBcolorseg2,ax
	add ax,1000h
	mov CHseg,ax ;Segment der Schrift

	mov es,cs:MBseg
	call ClearSeg
	mov es,cs:CHseg
	call ClearSeg

	in al,70h       ;NMI aus
	and al,7fh
	out 70h,al
	call CheckTiming

	mov si,IMG_data         ;"DATA"-Schriftzug schreiben...
	xor di,di
	mov cx,74
	mov dx,151
	pusha
	call SetImage
	popa
	mov si,IMG_land         ;"LAND"
	mov di,76 by 0
	call SetImage
	mov cx,74
	mov dx,113
	mov si,IMG_bbs          ;"BBS"
	mov di,158 by 20
	call SetImage

	call StartS3M           ;Musik starten
	mov b[cs:MBmode],0      ;1. Modus ohne 2ten Screen

; *************************
; *** Gebiss einblenden ***
; *************************
TeethIn:call SetScrollMode
	xor ax,ax
	call MBwriteScreen
	mov ds,cs
	mov es,cs

	mov si,MBcolors
	mov di,Newcolors
	mov cx,9
	rep movsb
	call FullFade

	mov cx,115
	mov dx,65
	mov si,IMG_teeth
	mov di,HeadPos
	call Setimage
	xor ax,ax
	call MBwriteScreen
	mov ds,cs
	mov es,cs
	mov al,63		;Farbe des Gebisses von weiss
	mov di,Oldcolors+6
	mov cx,9*3
	rep stosb
	mov si,MBcolors        ;in richtige Farbe Åbergehen lassen
	mov di,Newcolors
	mov cx,768
	rep movsb
	call Fullfade

; ***********************
; *** Kopf einblenden ***
; ***********************
HeadIn: call GetFXpal           ;Farben berechnen
	call Retrace
	call SetOldCols         ;und benutzen

	mov ds,cs
	mov es,MBseg
	mov bl,-16
	mov cx,115+32
headin2:push bx,cx
	pusha
	push ds,es
	xor ax,ax
	call MBwriteScreen
	pop es,ds
	popa

	mov si,IMG_head
	mov di,HeadPos
	mov cx,115
h3:     push di,si,cx
	mov cx,65

	or bl,bl
	jnz >h4
	call InitFXline
	jmp >h5

h4:     cmp bl,FXsize
	jne >h6
	rep movsb
	jmp >h5

h6:     cmp bl,FXsize
	ja >h5			;Zeile faden
h7:     cmp b[es:di],11         ;Ist eine Farbe vorhanden ?
	jb nolf
	inc b[es:di]
nolf:   inc di
	loop h7

h5:     pop cx,si,di
	add si,65
	add di,256
	dec bl
	loop h3

	pop cx,bx
	inc bl
	loop Headin2
; ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
	xor bl,bl               ;Hintergrundbalken schreiben
	call Hlines

	mov ds,cs               ;FarbÅbergÑnge fÅr Hintergrund berechnen
	mov es,cs
	mov si,LineStart*3+newcolors
	mov di,LineStart+63*3+newcolors
	mov w[si],0
	mov b[si+2],20h
	mov w[di],2000h
	mov b[di+2],0
	
	mov bx,64
	pusha
	call Compfade
	popa
	mov si,LineStart+64*3+newcolors
	mov di,LineEnd*3+newcolors
	mov w[si],2000h
	mov w[si+2],0
	mov w[di],0
	mov w[di+2],20h
	call Compfade
	
; ----- Introduction
; *********************
; *** Scroll around ***
; *********************
ScrollAround:
	mov ds,cs		;Hintergrundfarben alle auf grÅn setzen !
	mov es,cs
	mov di,LineStart*3+oldcolors
	mov cx,LineEnd-Linestart
S5:     xor al,al
	stosb
	mov al,1ah
	stosb
	xor al,al
	stosb
	loop s5

	call Retrace            ;Neue Farben setzen, Retrace damit's nich
	call retrace
	call SetOldCols         ;blitzt !

	xor ax,ax               ;Grad=0...
	call getDirection       ;Richtung holen...
	mov Slopex,dx           ;...und vermerken
	mov SlopeY,cx

	push Lineposx,lineposy  ;Zielposition des Bildschirms berechnen
	mov w LineposX,8000h
	mov w LineposY,8000h
	call GetDistance
	mov ds,cs
	mov dh,[LinePosX+1]     ;und in DH/CH merken
	mov ch,[LineposY+1]
	pop LinePosY,lineposx   ;alte Positon wiederholen

S2:     xor ax,ax
	call MBwriteScreen      ;Bildschirm zeichnen
	call Fade               ;Farben faden
	mov ds,cs
	cmp b[LinePosX+1],dh    ;Bildschirmposition verschieben
	je >S3
	dec b[LineposX+1]
S3:     cmp b[LinePosY+1],ch
	je >S4
	inc b[LinePosY+1]
	jmp S2
S4:     cmp b[LinePosX+1],dh
	jne S2

; *******************************
; *** Erzeugt Farben auf Logo ***
; *******************************
CreateLogoColors:
	mov ds,cs:MBseg
	xor si,si
	mov cx,-1
	mov bx,cx
C0:     inc bh
	jnz >C3
	inc bl
	cmp bl,4
	if e xor bl,bl
C3:     mov al,b[si]
	cmp al,1
	jne >C2
	mov ax,si
	and al,7
	mov ah,bl
	shl ah,3
	or al,ah
	add al,PatStart/3
	mov b[si],al
C2:     inc si
	loop C0
; ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
	mov si,PAT_00           ;Raster definieren
	call CreatePattern
	call SetOldCols         ;Und alle neuen Farben einblenden

; ----- 2.Teil
; *************************************
; *** Initialisiert Text-Bildschirm ***
; *************************************
InitTextScreen:
	mov ds,cs
	mov w TextAdr,TXT_main
	mov b Textmode,0       ;Anfang der Zeile
	mov b TextLine,-1
	mov w[255*3+oldcolors],3f00h
	mov b[255*3+2+oldcolors],3fh
	call SetOldCols
	mov b MBmode,1

;mov es,mbseg
;        xor di,di
;        xor cx,cx
;l1:     xor al,al
;	 mov ah,cl
;        and ah,ch
;        if z mov al,ch
;        stosb
;        loop l1

; *****************
; *** Rotate FX ***
; *****************
; 2 Bildschirme (gedrehte Graphik/Text) mit Motion Blur

MBlen=10000 ;2400*2                    ;Dauer des Dreheffekts
RotateFX:
	mov ds,cs
	mov es,cs

	xor bp,bp               ;Grad der Drehung
	mov cx,MBlen
mblpm:  push cx,bp
	mov cx,cs:MBmode1    ;Anzahl der Schritte fÅr richtige Geschw.
mbtm1:  push cx
	cmp b cs:MBmode,3
	je NoScrollUp
	call Write2ndScreen	;Text im 2. Bildschirm schreiben
	call Write2ndScreen
	cmp b cs:MBmode,2
	je >l1
	call ScrollUp           ;Colorcycling
	jmp noscrollup
l1:     call fade
noscrollup:
	cmp b MBzmode,0
	jne back
	mov ax,456h
	cmp MBzoom,ax ;456h ;+90      ;Bildschirm kleiner machen
	je ReturnAgain
	inc w MBzoom
	jmp Zoomed
back:   mov ax,-400
	cmp MBzoom,ax ;-400 ;40h
	je ReturnAgain
	dec w MBzoom
	jmp zoomed
ReturnAgain:
	xor b MBzmode,1
	mov ax,-400
	cmp mbzoom,ax ;-400
	jne zoomed
	cmp b[MBmode],2
	jb zoomed
	mov ax,MBseg
	mov bx,MBcolorseg2
	mov MBseg,bx
	mov mbcolorseg2,ax
	mov w masterlinediff,45
Zoomed: pop cx
	loop mbtm1

	mov ds,cs
	mov ax,bp
	call MBwriteScreen      ;Bildschirm neu schreiben
	in al,60h
	cmp al,1
	je mbreak2
	call SetOldCols
	mov ds,cs
	pop bp                  ;neuer Grad
	cmp b MBmode,2
	jb >l1
	inc bp
	jmp >l2
l1:     add bp,MBmode1
l2:     cmp bp,359
	if a sub bp,360
	pop cx
	sub cx,MBmode1
	jnc mblpm2
l1:     mov ds,cs
	mov es,cs

; **********************
; *** Letzter Effekt ***
; **********************
LastOne:mov w MBzoom,100h      ;Zooming abstellen
	mov ax,MBseg
	mov MBsegA,ax
	mov w masterlinediff,45 ;Verzerrung

LO2:    push cx,bp
	mov b MBmode,1         ;1. Bildschirm 
	mov ax,bp
	call MBwriteScreen      ;Graphik auf Screen projizieren.
	pop bp,cx
	add bp,MBmode2
	cmp bp,360
	if a sub bp,360
	;
	cmp cx,64               ;Bei den letzten 64 Screens anfangen zu
	jnb MBfadeOk2           ;faden.
	call Fade
MBfadeOk2:
	in al,60h               ;Bei Tastendruck abbrechen.
	cmp al,1
	je lo4
	sub cx,MBmode2
	jnc LO2
lo4:

; ----- Das war's !
l1:     call StopS3M            ;Musik stoppen
	in al,70h               ;NMI ein
	or al,80h
	out 70h,al
	mov ax,3                ;normalen Textbildschirm anschmeissen
	int 10h
	mov dx,TXT_bye		;Message raus
	mov ah,9
	int 21h
bye:    mov ax,4c00h            ;und das war's
	int 21h

mbreak2:pop bp,cx
	jmp l1

mblpm2: jmp mblpm
	

; ******************************************
; *** Tabellen fÅr Motion Blur berechnen ***
; ******************************************
create_motion_blur_tables:
	push ds,es
	mov ax,cs:MBcolorseg
	mov es,ax
	xor di,di
	mov cx,100h	;Farbumwandlungstabelle berechnen
l1:	push cx
	xor ah,ah
	sub ah,cl
	mov cx,100h
l2:	push ax
	xor al,al
	sub al,cl
	mov bl,al	;Grundfarben erniedrigen (alte Farbe)
	mov bh,al
	shr al,6	;rot
	shr bl,3	;grÅn
	and bl,7
	and bh,7	;blau
	mov dl,ah	;neue Farbwerte aufteilen
	mov dh,ah
	and dh,7	;blau
	shr ah,6	;rot
	shr dl,3	;gruen
	and dl,7
	add al,ah
	add bl,dl
	add bh,dh
	shr al,1	;und den Durchschnitt berechnen
	shr bl,1
	shr bh,1
	shl al,6	;Grundfarben wieder zusammenfÅgen
	shl bl,3
	or al,bl
	or al,bh
	stosb
	pop ax
	loop l2
	pop cx
	loop l1
	;
	mov bx,oldcolors
	mov w[cs:bx],0
	mov b[cs:bx+2],0
	mov ax,cs:MBseg      ;Screen auf neue Farbpalette umlegen
	mov ds,ax
	mov es,ax
	xor si,si
	xor di,di
	mov cx,0ffffh
l1:	lodsb		;Farbwert holen
	xor ah,ah	;und seine Adresse in der Farbpalette berechnen
	mov bx,ax
	add ax,ax
	add bx,ax
	add bx,oldcolors
	mov al,b[cs:bx]    ;rot
	shl al,2
	and al,11000000b
	mov dl,b[cs:bx+1]  ;grÅn
	and dl,111000b
	or al,dl
	mov dl,b[cs:bx+2]  ;blau wie ich
	shr dl,3
	or al,dl
	stosb           ;und neuen Farbwert retour
	loop l1
	;
	mov ax,cs       ;Neue Farbpalette berechnen
	mov es,ax
	mov di,newcolors
	mov cx,100h
l1:     xor al,al
	sub al,cl
	mov ah,al
	and al,11000000b
	shr al,2
	mov dl,al
	shl dl,2
	or al,dl
	shl dl,2
	or al,dl
l2:     stosb
	mov al,ah
	and al,111000b
	mov dl,al
	shr dl,3
	or al,dl
l2:     stosb
	mov al,ah
	and al,111b
	mov dl,al
	shl al,3
	or al,dl
l2:     stosb
	loop l1
	mov di,oldcolors
	mov cx,768
	mov al,63
	rep stosb
	call setoldcols

	pop es,ds
	ret

; *************************************
; *** Berechnet Richtung aus Graden ***
; *************************************
; Grade werden in 1024/360¯ angegeben.
; Iput:         AX=Grad
; Output:       DX/CX=Richtung
GetDirection:
	call getDirectionB
	push cx
	mov ax,dx
	call GetDirectionZoom   ;Richtung * Zoomfaktor
	mov Slopex,ax
	pop ax
	call GetDirectionZoom
	mov SlopeY,ax
	ret

GetDirectionZoom:
	movsx eax,ax
	xor edx,edx
	movsx ebx,w[MBzoom]
	mul ebx
	shr eax,8
	ret

GetDirectionB:
	cmp ax,359              ;Gradzahl zu hoch ?
	if a sub ax,360         ;Ja,korrigieren...
	mov ds,cs
	mov bx,ax
	xor bp,bp
	cmp ax,90
	jb >g5
	mov bx,90
	xor dx,dx
	div bx
	mov bx,dx
	mov bp,ax
g5:     shl bx,2
	mov dx,slopes[bx]
	mov cx,slopes[bx+2]

	or bp,bp
	jz >g6
	mov ax,bp
	or al,al
	je >g6
	dec al
	jne >g3
	xchg cx,dx
	neg dx
	ret
g3:     dec al
	jne >g4
	neg dx
	neg cx
	ret
g4:     xchg cx,dx
	neg cx
g6:     ret

; ***********************************************************
; *** Berechnet den Abstand zum Mittelpunkt eines Kreises ***
; ***********************************************************
GetDistance:
	mov eax,188              ;relativer Abstand * Zoomfaktor
	xor ebx,ebx
	movsx edx,w[MBzoom]
	push edx
	mul edx
	;
	xor ebx,ebx
	movsx edx,w[SlopeX]
	mul edx
	shr eax,8
	neg ax
	add LineposX,ax      ;neue X-Position
	;
	mov eax,100              ;relativer Abstand * Zoomfaktor
	xor ebx,ebx
	pop edx
	mul edx
	;
	xor ebx,ebx
	movsx edx,w[SlopeY]
	mul edx
	shr eax,8
	neg ax
	add LineposY,ax      ;neue Y-Position
	ret

; **************************************
; *** Zeichnet kompletten Bildschirm ***
; ***     mit angegebener Drehung    ***
; **************************************
; AX=Grad
MBwriteScreen:
	pusha                   ;Erstmal alles retten, man weiss ja nie wofÅr
	push Lineposx,lineposy,ax ;Bildschirmmitte & Gradzahl retten
	call getDirection       ;Winkel umrechnen, wir kînnen nix mit Grad
				;anfangen
	call GetDistance        ;Richtige Bildschirmmitte berechnen
	mov di,LineTabS         ;Zeichnende Linie erstellen
	call CreateLineAdrs
	call CreateLinePrg
	pop ax
	add ax,MasterLineDiff   ;Abweichung der fÅhrenden Linie
	call GetDirection       ;Winkel fÅr fÅhrende Linie berechnen
	mov di,LineTabM
	call CreateLineAdrs     ;und selbige schreiben

	mov ax,cs:MBcolorseg
	mov gs,ax               ;GS - Segment fÅr Farbumwandlung
	mov ax,cs:CHseg         ;FS mit Segment des 2. Bildschirms laden
	mov fs,ax
	mov ax,cs:MBseg
	mov ds,ax
	mov ax,cs:MBworkSeg
	mov es,ax
	mov bp,MBjumptab
	mov cx,4                ;Insgesamt 4 Planes wollen beschrieben werden
MBlp2:  push cx
	mov ah,4
	sub ah,cl
	mov al,4
	mov dx,3ceh             ;Plane setzen
	out dx,ax
	mov cl,ah
	mov ax,102h
	shl ah,cl
	mov dx,3c4h
	out dx,ax

	mov si,LineTabM         ;Zeiger auf Bildschirmpos. der fÅhrenden
				;Linie setzen
	xor di,di
	mov cx,height           ;200 Zeilen (!)
MBlp3:  push cx
	mov bl,[cs:LineposX+1]
	mov bh,[cs:LineposY+1]
	add bl,[cs:si]
	add bh,[cs:si+1]
	push si
	cmp b cs:MBmode,2
	if nb mov si,bx
	cs:call w[bp]    ;Generiertes Programm, welches 1 Zeile vom Arbeits-
			;ins Screensegment ÅbertrÑgt aufrufen.
	pop si,cx
	add si,2
	loop MBlp3
	pop cx
	mov ax,fs
	add ax,400h
	mov fs,ax
	add bp,2
	loop MBlp2

	mov ds,cs
	push MBdispAdr,MBworkAdr
	pop MBdispAdr,MBworkAdr
	push MBdispSeg,MBworkSeg
	pop MBdispSeg,MBworkSeg
	mov bx,MBdispAdr
	mov dx,3d4h
	mov al,0ch
	out dx,al
	inc dl
	mov al,bh
	out dx,al
	dec dl
	mov al,0dh
	out dx,al
	mov al,bl
	dec dl
	out dx,al
	call Retrace
	pop LinePosY,LineposX
	popa
	ret

; ***********************
; *** Berechnet Linie ***
; ***********************
CreateLineAdrs:
	mov es,cs
	mov ds,cs
	mov di,LineTab  ;Zeiger auf Positionen

	xor dx,dx
	xor bx,bx
; Berechnung der POSITIONEN auf der Linie
	mov cx,320
CrLinTab2:
	add dx,SlopeX
	add bx,SlopeY
	mov al,dh
	mov ah,bh
	stosw
	loop CrLintab2
	ret

; *****************************************************************
; *** Aufteilung der Linie in Planes und Erstellung der Routine ***
; *****************************************************************
; Motion Blur in Farbe
; mov bl,[es:di]        ;alter Wert
; mov bh,disp[si]       ;Neuer Wert (Graphik)
; or bh,disp[fs:si]     ;Neuer Wert (Textscreen)
; mov al,[gs:bx]        ;Neuer, berechneter Wert
; stosb                 ;Fertig

; GS kann zwei Paletten benutzen.
; Einmal um zu dekrementieren und zu mischen; einmal um nur zu mischen
; um die Spuren lÑnger auf dem Bildschirm zu lassen.
; Intersectioning testen !
CreateLinePrg:
	mov si,LineTab
	mov di,LinetabS
	mov bp,MBJumptab
	mov cx,4
CrLinTab4:
	mov [bp],di
	add bp,2
	push si,cx
l1:     mov cx,40
CrLinTab3:
	cmp b cs:MBmode,2
	jb crlintab7
DoShadowing:
	mov ax,8b26h    ;mov cx,[es:di]
	stosw
	mov al,0dh
	stosb
	mov ax,0d98ah   ;mov bl,cl
	stosw
	mov ax,0bc8ah   ;mov bh,[bp+disp]
	stosw
	mov ax,w[si]
	stosw
	add si,8
	mov al,65h      ;gs:
	stosb
	mov ax,078ah    ;mov al,[bx]
	stosw
	mov ax,0dd8ah   ;mov bl,ch
	stosw
	mov ax,0bc8ah   ;mov bh,[bp+disp]
	stosw
	mov ax,w[si]
	stosw
	mov al,65h
	stosb
	mov ax,278ah    ;mov ah,[bx]
	stosw
	cmp b cs:MBmode,3
	je >l2
	mov al,64h      ;or ax,[fs:di]
	stosb
	mov ax,050bh
	stosw
l2:     mov al,cm_stosw
	stosb
	add si,8
	jmp >l1

CrLinTab7:
	mov ax,cm_movalbbx
	stosw
	mov ax,w[si]
	add si,8
	stosw
	mov ax,cm_movahbbx
	stosw
	mov ax,w[si]
	add si,8
	stosw

	cmp b cs:MBmode,0
	je No2ndScreen
	mov ax,CM_orax1
	stosw
	mov al,cm_orax2
	stosb
No2ndScreen:
	mov al,cm_stosw
	stosb
l1:     loop CrLinTab3
l1:     mov al,cm_ret
	stosb
	pop cx,si
	add si,2
	loop CrLinTab4_b
	ret
crlintab4_b:
	jmp crlintab4

; ****************************************
; *** Konvertiert Farben fÅr Shadowing ***
; ****************************************
ConvertCols:
	mov ds,cs:MBseg
	mov es,ds
	xor di,di
	mov cx,-1
ConvertCols2:
	mov al,b[di]
	xor ah,ah
	mov bx,ax
	add ax,ax
	add bx,ax
	add bx,Oldcolors
	mov al,[cs:bx]
	add al,[cs:bx+1]
	adc ah,0
	add al,[cs:bx+2]
	adc ah,0
	shr ax,1
	mov bx,ax
	shr bx,1
	add al,bl
	stosb
	loop ConvertCols2
	ret

; ***************************************************
; *** öberprÅft die Geschwindigkeit des Rechners, ***
; *** und richtet die Schrittgrî·en dafÅr aus.    ***
; ***************************************************
CheckTiming:
	cli
	mov ds,cs
	xor ax,ax
	mov es,ax
	mov ax,cs
	shl eax,16              ;Interrupt auf neue Routine
	mov ax,ChkInt
	xchg [es:8*4],eax
	mov Chkoldint,eax
	mov bx,1193180/100      ;Timer 100 mal die Sek.
	call chkwrtmr
	mov b MBmode,0          ;Single screen testen
	call CheckOnce
	mov MBmode0,ax

	mov b MBmode,1          ;Double screen testen
	call CheckOnce
	mov MBmode1,ax

	mov b MBmode,2          ;Text screen testen
	call CheckOnce
	mov MBmode2,ax

	mov b MBmode,3          ;Shaded screen testen
	call CheckOnce
	mov MBmode3,ax
	
	xor bx,bx               ;Timer 18.2 mal die Sekunde
	call chkwrtmr
	xor ax,ax
	mov es,ax
	mov eax,Chkoldint
	mov [es:8*4],eax
	sti
	ret

; Testen einzelnen Modus
CheckOnce:
	call retrace
	mov w TmrCnt,0
	sti
	call fade
	xor ax,ax
	call MBwriteScreen
	call retrace
	cli
	mov ds,cs
	mov ax,TmrCnt
	shr ax,1
	or ax,ax
	jne co2
	inc ax
co2:    inc ax
	ret

chkint: push ax
	inc w cs:TmrCnt
	mov al,20h
	out 20h,al
	pop ax
	iret

ChkWrTmr:
	mov al,110110b
	out 43h,al
	mov al,bl
	out 40h,al
	mov al,bh
	out 40h,al
	ret
	
; ********************************
; *** Scrollt Muster nach oben ***
; ********************************
ScrollUp:
	mov ds,cs
	mov es,cs
	mov si,Oldcolors+PatStart      ;oberste Zeile in Puffer
	mov di,PatBuffer
	mov cx,8*3
	rep movsb

	mov di,Oldcolors+Patstart      ;Den Rest nach "oben" kopieren
	mov cx,3*8*3    ;64
	rep movsb
	mov di,3*8*3+oldcolors+patstart;56;Anfang aus dem Puffer ans Ende
	mov si,PatBuffer
	mov cx,8*3
	rep movsb
	mov dx,[LineStart*3+oldcolors]
	mov bl,[LineStart*3+2+oldcolors]
	mov si,LineStart*3+3+oldcolors
	mov di,LineStart*3+oldcolors
	mov cx,LineEnd-LineStart*3/2
	rep movsw
	mov [LineEnd*3-3+oldcolors],dx
	mov [LineEnd*3-1+oldcolors],bl
	ret

; ***********************
; *** Muster fÅr Logo ***
; ***********************
; CS:SI=Quelle
CreatePattern:
	mov ds,cs
	mov es,cs
	lodsw           ;Anzahl der Werte holen
	mov bp,si       ;Zeiger auf die Farbtabelle
	add si,ax
	mov di,Oldcolors+PatStart
	mov cx,8*4      ;Anzahl der zu bestimmenden Farben
CrPa:   lodsb           ;Farbe holen
	sub al,"A"      ;Tabellenposition berechnen
	xor ah,ah
	mov bx,ax       ;*3
	add bx,bx
	add bx,ax
	add bx,bp
	mov ax,[bx]
	stosw
	mov al,[bx+2]
	stosb
	loop CrPa
	ret


; *****************************
; *** Berechnet Farbverlauf ***
; *****************************
; SI/DI=Quelle/Ziel
; DX=Anzahl der Farben
CompFade:
	push ds,es
	mov ds,cs
	mov es,cs
	mov cx,3
C2:     call CompFade3
	loop C2
	pop es,ds
	ret

Compfade3:
	push si,di,bx,cx
	mov ah,[di]     ;Verlauf berechnen
	sub ah,[si]
	xor al,al
	xor bp,bp
	or ah,ah
	jns >C6
	neg ah
	inc bp
C6:     xor dx,dx
	mov cx,bx
	or ah,ah
	jz >C5
	div bx
	or bp,bp
	jz >C5
	neg ax
C5:     mov dx,ax
	mov ah,[si]
	xor al,al
C4:     add si,3
	add ax,dx
	mov [si],ah
	loop C4
	pop cx,bx,di,si
	inc si
	inc di
	ret


; ************************************
; *** Kreiert horizontale Streifen ***
; ************************************
Hlines: push bx
	mov ds,cs
	mov es,cs
	mov bx,Oldcolors
	mov dh,[bx]
	mov bx,[bx+1]
	mov cx,64
	mov di,64*3+oldcolors
h2:     mov al,dh
	stosb
	mov ax,bx
	stosw
	loop h2
	call SetOldCols
	pop bx  

	mov ds,cs:MBSeg
	mov es,ds
	xor si,si
	mov ah,LineStart
	mov cx,256
h4:     push cx
	mov cx,256
H3:     lodsb
	cmp al,bl
	if z mov [si-1],ah
	loop h3
	pop cx
	inc ah
	cmp ah,LineEnd
	if e mov ah,LineStart
	loop h4
	ret

; ****************************************
; *** Lîscht horizontale Balken wieder ***
; ****************************************
ClearHlines:
	mov ds,cs
	mov es,MBseg
	mov cx,-1
	xor di,di
l2:     mov al,[es:di]
	cmp al,LineStart
	jb >l3
	cmp al,LineEnd
	if na xor al,al
l3:     stosb
	loop l2
	ret

; **************************************
; *** Schreibt Text in 2. Bildschirm ***
; **************************************
w2sInitShading:
	call ClearHlines        ;Farbbalken im Hintergrund entfernen..
	call create_motion_blur_tables

	mov b MBmode,2
	mov ax,MBmode2
	mov MBmode1,ax
	jmp w2sNextPage

w2sInitSingleShading:
	mov b Textmode,-1
	mov b MBmode,3
	mov ax,MBmode3
	mov MBmode1,ax
w2send2:jmp w2send

w2sNextPage2:
	cmp b pghlp,1
	je >p2
	inc b Pgcunt
	inc b pghlp
	cmp b pgcunt,2
	je w2sInitShading
p2:     jmp w2sNextPage
Write2ndScreen:
	pusha
	push ds,es
	mov ds,cs
	mov al,TextMode         ;Aktuellen Modus holen
	cmp al,-1
	je w2sEnd2              ;Nix machen...
	cmp al,2
	je w2sInitSingleShading ;Shademode initialisieren
	cmp al,1
	je w2sNextPage2         ;Ende der Textdarstellung
	or al,al
	jnz >w2                 ;Text zeichnen
	mov b pghlp,0
	mov si,TextAdr          ;LÑnge der Zeile zÑhlen
	xor cx,cx
w3:     lodsb
	inc cx
	cmp al,32
	jnb w3
	dec cx
	mov ax,20               ;Abstand zum linken Rand berechnen
	sub ax,cx
	shl ax,3
	mov TextOfsX,ax
	mov w TextPosX,0
	mov b TextMode,3
	inc b TextLine

w2:     mov ax,TextOfsX         ;Zieladresse im 2. Bildschirm
	add ax,TextPosX
	mov bh,al               ;(auch Segment)
	and bh,3
	xor bl,bl
	shl bh,2
	add bx,CHseg
	mov es,bx
	shr ax,2
	mov di,ax

	mov ax,TextLine         ;(Zeilenadresse)
	shl ax,8
	add di,ax
	shl ax,2
	add di,ax
	add di,4*80             ;(Mindestabstand vom oberen Rand)

	mov si,TextAdr          ;Adresse des aktuellen Buchstabens
	mov ax,TextPosX
	shr ax,4
	add si,ax

	mov al,[si]             ;Adresse des Buchstabens holen
	cmp al,32
	jnb >w6
	mov Textmode,al
	inc si
	mov TextAdr,si
	mov w Texthelp,0
	jmp w2sEnd

w6:     xor ah,ah
	shl ax,4
	add ax,CharsetAdr
	mov si,ax
	
	mov cl,TextPosX         ;Bitmaske berechnen
	and cl,15
	shr cl,1
	mov bh,128
	shr bh,cl

	mov ds,CharsetSeg
	mov cx,16               ;16 Zeilen bearbeiten
w4:     lodsb
	mov ah,-1
	and al,bh
	jnz >w5
	xor ah,ah
w5:     mov [es:di],ah
	add di,80
	loop w4
	inc w cs:TextPosX
w2sEnd: pop es,ds
	popa
	ret

w2sNextPage:
	mov ax,cs:CHseg
	mov cx,4
p3:     push ax,cx
	mov es,ax
	mov ax,TextHelp
	shl ax,4
	mov di,ax
	shl ax,2
	add di,ax
	mov cx,80
	xor al,al
	rep stosb
	mov al,-1
	mov cx,80
	rep stosb
	pop cx,ax
	add ax,400h
	loop p3
	inc b TextHelp
	cmp b TextHelp,200
	je w2snpend
	jmp w2sEnd

w2snpend:
	mov b Textmode,0
	mov b TextLine,-1
	jmp w2send

; ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
; ***************************************
; *** Baut Farbpalette fÅr FXfade auf ***
; ***************************************
GetFXpal:
	mov ds,cs
	mov es,cs
	xor bx,bx
	mov cx,115*65

GetPal3:mov al,IMG_head[bx]   ;Beide Farben holen
	or al,al
	je ColorOk              ;Keine Farbe vorhanden
	mov ah,IMG_teeth[bx]
	cmp al,ah
	je ColorOk
	mov di,fxcolortab       ;Und danach in der Tabelle suchen

GetPal2:cmp w[di],0             ;Ende der Tabelle, Farbe einfÅgen
	je insertcolor
	cmp [di],ax
	je ColorOk              ;Farbe wurde bereits erstellt
	add di,2
	jmp GetPal2             ;weitersuchen

ColorOk:inc bx
	loop GetPal3
	ret

InsertColor:
	pusha
	push ax         ;Platz fÅr Farben berechnen
	mov [di],ax
	sub di,fxColortab
	mov ax,di
	shr ax,1
	add di,ax
	push ax,bx,dx
	mov ax,di
	mov bx,FXsize
	xor dx,dx
	mul bx
	mov di,ax
	pop dx,bx,ax
	add di,Oldcolors+33
	pop ax
	xor cx,cx
	call insertB
	inc cx
	call InsertB
	inc cx
	call InsertB
	popa
	jmp ColorOk

; CX=Offset der Grundfarbe
; DI=Zeiger auf Zielbereich
; AX=Farbkombination
InsertB:pusha
	mov bl,ah       ;Farbe des Gebisses nach BX
	xor bh,bh
	xor ah,ah
	mov bp,ax       ;Farbe des Kopfes nach BP
	mov ax,bx
	add bx,bx
	add bx,ax
	mov ax,bp
	add bp,bp
	add bp,ax

	add bx,Oldcolors
	add bp,Oldcolors
	add bx,cx
	add bp,cx
	add di,cx

	xor al,al
	mov ah,[bp]
	sub ah,[bx]
	or ah,ah
	jz ibdontdiv
	xor cl,cl
	or ah,ah
	jns notneg
	inc cl
	neg ah
notneg: push bx
	mov bx,FXsize
	xor dx,dx
	div bx
	pop bx
	or cl,cl
	jz notneg2
	neg ax
notneg2:
ibdontdiv:
	mov dx,ax

	mov ah,[bx]
	xor al,al

	mov cx,16
insertC:add ax,dx
	mov [di],ah
	add di,3
	cmp di,Oldcolors+767
	je insertCerr
	loop insertC
	popa
	ret

insertCerr:
	jmp internalerror

; Fading einer Zeile vorbereiten
InitFXline:
ifxl1:  mov al,[si]
	or al,al
	jz ifxl4
	mov ah,[es:di]
	cmp al,ah
	je ifxl4
	mov bp,fxColorTab
ifxl2:  cmp [bp],ax
	je ifxl3
	add bp,2
	jmp ifxl2

ifxl3:  sub bp,fxcolortab
	mov ax,bp
	push bx,dx
	mov bx,FXsize/2
	xor dx,dx
	mul bx
	pop dx,bx
	add al,11
	stosb
	inc si
	
ifxl5:  loop Ifxl1
	ret

ifxl4:  inc si
	inc di
	jmp ifxl5

; ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
ClearSeg:
	xor di,di
	xor ax,ax
	mov cx,8000h
	rep stosw
	ret

; **********************
; *** Setzt Bildteil ***
; **********************
; SI=Quelle DI=Ziel
; CX=Hîhe DX=Breite
SetImage:
	push ds,es
	mov ds,cs
	mov ax,MBseg
	mov es,ax
	mov bx,256      ;Zeilensprung berechnen
	sub bx,dx
si2:    push cx
	mov cx,dx
	rep movsb
	add di,bx
	pop cx
	loop si2
	pop es,ds
	ret

InternalError:
	mov ds,cs
	mov ax,3
	int 10h
	mov dx,Err_internal
	mov ah,9
	int 21h
	mov ax,4c00h
	int 21h

; *****************************
; *** Videomodus einstellen ***
; *****************************
SetScrollMode:
	mov dx,03c4h    ;chain-4-adr off
	mov al,4
	out dx,al
	inc dx
	in al,dx
	and al,11110111b ;(not 8)
	out dx,al
	mov dx,3d4h     ;double word off
	push dx
	mov al,14h
	out dx,al
	inc dx
	in al,dx
	and al,10111111b ;(not 40h)
	out dx,al
	dec dx          ;byte mode on
	mov al,17h
	out dx,al
	inc dx
	in al,dx
	or al,40h
	out dx,al
	pop dx          ;width
	mov al,13h
	out dx,al
	inc dx
	mov al,320/8
	out dx,al
	ret
   
SetOldCols:
	mov ds,cs
	mov dx,3c8h
	xor al,al
	out dx,al
	inc dl
	mov si,Oldcolors
	mov cx,768
	rep outsb
	ret

; ***************************************************************************
	include"fade.inc"
	include"retrace.inc"
	include"s3madlib.inc"
; ***************************************************************************

code    segment

TXT_cr: db 10,13,"$"
TXT_bye:db"Phew ! -> (c)1996 Tetrabyte !",10,13,"$"
           
ERR_internal:
	db"error$"
           
TXT_main:  
	db"====================",0
	db"DATALAND BBS",0
	db"IN GöTERSLOH/GERMANY",0
	db"THE ULTIMATE SOURCE",0
	db"FOR AMIGA, PC AND",0
	db"MACINTOSH STUFF !",0
	db"5 PUBLIC NODES",0
	db"1 NODE FOR MEMBERS",0
	db"ON LINE 24H A DAY",0
	db"NO DOWNLOAD RATIO !",0
	db"ENTER AND ENJOY !",0
	db"===================="
	db 1
	db"- DATALAND NODES -",0
	db"(PREDIAL ++49/5241)",0
	db" ",0
	db"3xxx   V.FC         ",0
	db"3xxx   V.34-V.FC    ",0
	db"3xxx   VTerbo 19.200",0
	db"3xxx   V.32 14.400  ",0
	db"3xxxxx Zyxel 1496E+ ",0
	db"- Secret port -",0
	db"3XXXXX",0
	db"for Amicon members",0
	db"-",1
	db"- CONNECTED TO -",0
	db"FIDO-NET",0
	db"(x:xxxx/xxx)",0
	db"AMIGA-NET",0
	db"(xx:xxx/xxx)",0
	db"INTERNET",0
	db"(xxxxxxxx.xxx.xx)",0
	db"Sysop: Einstein",0
	db"CoSysop: Heiko",0
	db"***",0
	db"Code&GFX/Spaceman",0
	db"Music/MCD",1,2
           
; *************************
; *** Muster fÅr's Logo ***
; *************************
PAT_00: dw 2*3          ;Anzahl der Farbwerte
	db 32,0,24      ;Farbwerte
	db 32,32,32     ;Farbverteilung
	db "AAAABBBB"
	db "BBAAAABB"
	db "BBBBAAAA"
	db "AABBBBAA"
           
; **************
; *** Bilder ***
; **************
MBcolors:  
	include"dlpal.inc"
S3Mbase:include"dls3m.inc"
IMG_start: 
IMG_head:  
	include"dlhead.inc"
IMG_teeth: 
	include"dlteeth.inc"
IMG_data:  
	include"data.inc"
IMG_land:  
	include"land.inc"
IMG_bbs:include"bbs.inc"
IMG_end:   
	db"Ceterum censeo Cannabis legalisandum esse !"
           
