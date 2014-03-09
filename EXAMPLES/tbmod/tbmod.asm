; niedrige Sampleraten
; SB/SBpro (stereo)/SB16/GUS
	.code
x=0
MODresolution   = 48000
MODintcalls     = 300
dmabuflen       = 65536
_end            = -1
oscofs          = 72*320
osczoom16       = 16
osccol          = 10
osccolb         = 2

; *********************
; *** Hauptprogramm ***
; *********************
DW_mod: ;mov w[dwmod2+1],20cdh
dwmod2: mov di,dwmoddata
	mov cx,dwmodend-dwmoddata
	xor al,al
	rep stosb
	xor al,al
	call AllocMem
	mov w[wssport],530h
	mov b[wssirq],5
	mov b[wssdma],1
	mov w[DMSresolution],modresolution ;46500
	mov b[DMS_sur],1        ;Surround ein
	mov dx,TXT_standard
	mov ah,9
	int 21h
	call line

	mov ax,w[2]             ;Ist Speicher fr DMA-šbertragung frei =
	sub ax,9000h
	jnc MemOk
	mov dx,err_nomemory     ;nope, mit Fehlermeldung abbrechen
	mov ah,9
	int 21h
	jmp bye

Memok:
	mov b[sdev_set],0
	mov si,81h              ;Kommandozeilenparameter parsen.
m1:     cmp b[si]," "
	jne >l1
	inc si
	jmp m1
l1:     cmp b[si],13
	je et
	cmp b[si],"/"
	jne >n1
	inc si
	mov di,LST_cards
	call getstring
	jnc >n1
	or b[sdev_set],1
	mov bx,ax
	add bx,bx
	pusha 
	call w[bx+jmp_cards]
	popa
	jc >j1
	jmp m1

j1:     mov dx,err_notthere
	mov ah,9
	int 21h
	jmp bye2

n1:     mov di,LST_output
	call getstring
	jnc >n1
	mov bx,ax
	add bx,bx
	pusha
	call w[bx+jmp_output]
	popa
	jmp m1
n1:     xor bp,bp                ;Samplingrate auslesen
	lodsb
	cmp al,"0"
	jb >l2
	cmp al,"9"
	ja >l2
	dec si                  ;Auf letzten Buchstaben zurckgehen
l10:    lodsb
	cmp al,"0"
	jb >e3
	cmp al,"9"
	ja >e3
	sub al,"0"              ;Ziffer bestimmen
	mov bx,bp               ;Wert*10
	shl bp,3
	shl bx,1
	add bp,bx
	xor ah,ah               ;Ziffer dazuaddieren.
	add bp,ax
	jmp l10                 ;und n„chsten Buchstaben holen
l1:     dec si
	jmp m1
e3:     mov [DMSresolution],bp
	mov b[sdev_set],2
	cmp bp,20000
	jae l1
	mov dx,TXT_toolow
l1:     mov ah,9
	int 21h
	jmp bye2
l2:     mov dx,txt_plist
	jmp l1

et:     test b[sdev_set],1
	jnz >e2
	call getpas             ;check on PAS,PAS+,PAS 16
	jnc nobye
	call getoak             ;check on Oak Mozart 16 pro
	jnc nobye
	call getsb              ;check on Soundblaster series
	jnc nobye
	call getwss             ;check on Windows Sound System
	jnc nobye
	mov dx,TXT_param
	mov ah,9
	int 21h
	call line
	test b[sdev_set],2
	jne >l1
	mov w[cs:DMSresolution],62000
l1:     call getinternal
e2:
Nobye:  mov ds,cs
	call Line
	call Showsdstate
	push cs
	pop es
	mov dx,TXT_name
	mov ah,9
	int 21h
	mov dx,Namebuffer
	mov w[namebuffer],40h
	mov ah,0ah
	int 21h
	mov dx,TXT_cr
	mov ah,9
	int 21h
	mov si,Namebuffer+2
	mov di,sm
mx:     lodsb
	cmp al,13
	je mx2
	stosb
	jmp mx
mx2:    xor al,al
	stosb
	mov ax,w[DOSsegstart]
	mov w[NewSeg],ax
	mov w[cs:FXchannels],x
	mov dx,TXT_loading
	mov ah,9
	int 21h
	mov si,SourceName       ;Modul laden
	call initMOD
	mov dx,TXT_playing
	mov ah,9
	int 21h
	mov dx,TXT_playing2
	mov al,b[DMSchannels]
	xor ah,ah
	call printword
	call StartMOD           ;Abspielroutine initialisieren
	
	mov ax,13h
	int 10h
wait1:  ;jmp >l3
	;call retrace
	mov ds,9000h
	mov es,0a000h
	mov cx,320
	mov di,oscofs
	mov si,[cs:modoldmixpos]
	cmp b[cs:dmsmode],1
	jne >l2
	mov cx,158
	push cx
	call >l1
	pop cx
	mov di,oscofs+162
	mov si,[cs:modoldmixpos]
	add si,2
	call >l1
	jmp >l3

l1:     lodsw
	sar ax,1
	add si,2+osczoom16
	mov al,ah
	xor ah,ah
	or al,al
	if z mov al,1
	js >g1
p1:     mov dl,al
	mov dh,64
	xor bx,bx
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
	mov dh,64
	sub dh,dl
	mov bx,320*64
	mov al,osccol
j1:     mov b[es:bx+di],al
	add bx,320
	dec dl
	jnz j1
	or dh,dh
	je >o1
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
	jmp >o1
	;
g1:     neg al
	mov dl,al
	mov dh,64
	sub dh,al
	jz >a1
	xor bx,bx
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
a1:     mov bx,320*64
	mov al,osccol
j1:     mov b[es:bx+di],al
	sub bx,320
	dec dl
	jnz j1
	mov dl,64
	mov bx,320*64
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dl
	jnz h1
o1:     inc di
	dec cx
	jnz l1
	ret

l2:     lodsb
	sub al,80h
	sar al,1
	add si,osczoom16/8
	xor ah,ah
	or al,al
	if z mov al,1
	js >g1
p1:     mov dl,al
	mov dh,64
	xor bx,bx
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
	mov dh,64
	sub dh,dl
	mov bx,320*64
	mov al,osccol
j1:     mov b[es:bx+di],al
	add bx,320
	dec dl
	jnz j1
	or dh,dh
	je >o1
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
	jmp >o1
	;
g1:     neg al
	mov dl,al
	mov dh,64
	sub dh,al
	jz >a1
	xor bx,bx
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dh
	jnz h1
a1:     mov bx,320*64
	mov al,osccol
j1:     mov b[es:bx+di],al
	sub bx,320
	dec dl
	jnz j1
	mov dl,64
	mov bx,320*64
	mov al,osccolb
h1:     mov b[es:bx+di],al
	add bx,320
	dec dl
	jnz h1
o1:     inc di
	dec cx
	jnz l2

l3:     mov ah,1
	int 16h
	jz wait1
	mov ds,cs

GoodBye:xor ax,ax
	int 16h
	cmp al,"+"
	jne nonextp
	mov b[MODpatcnt],1
	jmp wait1
nonextp:cmp al,"/"
	jne ns1
	xor ax,ax
	call StartSMP
	jmp wait1
ns1:    cmp al,"*"
	jne ns2
	mov ax,1
	call startsmp
	jmp wait1
ns2:    cmp al,"-"
	jne >n1
	mov ax,2
	call startsmp
	jmp wait1
n1:
bye:    call StopMOD
	mov ax,3
	int 10h
bye2:   call ReallocMem
	mov ax,4c00h
	int 21h

Line:   mov cx,80
line2:  mov al,196
	int 29h
	loop line2
	ret

; =================================
; >>> Zeigt gew„hlte Ausgabe an <<<
; =================================
showsdstate:
	mov ax,cs
	mov ds,ax
	mov ax,w[DMSresolution]
	mov dx,TXT_rate
	call Printword
	mov dx,TXT_mono
	test b[sdev_bits],128
	jz >l1
	mov dx,TXT_stereo
l1:     mov ah,9
	int 21h
	cmp b[dms_sur],1
	jne >l1
	mov dx,txt_sur
	mov ah,9
	int 21h
l1:     mov al,","
	int 29h
	mov al," "
	int 29h
	cmp b[DMS_over],2
	jne >l1
	mov dx,TXT_over
	mov ah,9
	int 21h
l1:     mov al,b[sdev_bits]
	and al,127
	xor ah,ah
	mov dx,TXT_bits
	call Printword
	ret

; ======================
; >>> get string no. <<<
; ======================
; Input:
; BP=input string
; DI=strings
; Output:
; AX=no.
Getstring:
	push cx,bp,si,di
	mov bp,si               ;save origin of string
	xor cx,cx               ;nowt found yet
l3:     mov si,bp
l1:     mov al,b[di]            ;get character
	or al,al
	je >n3                  ;string maybe found
	cmp al,"/"
	je >n3                  ;"
	cmp al,_end             ;end of string table
	je >n3                   ;maybe found
	cmp b[si],al
	jne >l2                 ;not the same char
	inc di
	inc si
	jmp l1

f5:     pop di,si,bp,cx
	clc             ;no string found
	ret

n3:     dec di
	mov al,b[si]
	or al,al
	jz >l4
	cmp al,13
	jz >l4
	cmp al," "
	jnz >l5
l4:     pop di,ax
	mov ax,cx
	pop bp,cx
	stc
	ret

l5:     cmp b[si],_end
	je f5
; update pointers to compare next string
l2:     inc di
l1:     mov al,b[di]
	inc di
	or al,al
	je >n1          ;get next
	cmp al,"/"      ;alternative string
	je l3
	cmp al,_end  ;desc end, no string found
	je f5
	jmp l1

n1:     inc cx
	jmp l3

par_8:  and b[cs:sdev_bits],128
	or b[cs:sdev_bits],8
	ret

par_m:  and b[cs:sdev_bits],127
	ret
par_o:  mov b[cs:dms_over],2
	ret
par_n:  mov b[cs:dms_sur],0
	ret

	.data
dwmoddata:
	.code
	include"\dw\inc\memory.inc"     ;Speicherverwaltung
	include"\dw\inc\doserruk.inc"   ;Fehlermeldungen

	include"\dw\inc\retrace.inc"
	include"\dw\inc\pic.inc"        ;Interruptcontroller
	include"\dw\inc\dma.inc"        ;DMA
	include"\dw\inc\dms.inc"        ;Mixing
	include"\dw\inc\printwrd.inc"   ;Anzeige von Zahlen
	include"\dw\inc\modplay.inc"    ;MOD-abh„ngige Routinen

	include"\dw\inc\pas.inc"        ;Pro Audio Spectrum
	include"\dw\inc\sblaster.inc"   ;SoundBlaster
	include"\dw\inc\wss.inc"        ;Windows Sound System
	include"\dw\inc\oak.inc"        ;Oak Mozart 16 pro
	include"\dw\inc\internal.inc"   ;PC-Speaker

	.code
TXT_rate:       db"Hz sampling rate, $"
TXT_mono:       db"mono, $"
TXT_stereo:     db"stereo$"
TXT_sur:        db" surround$"
TXT_over:       db"oversampled, $"
TXT_bits:       db" bits activated.",10,13,"$"
TXT_standard:   db"Tetrabyte Sound System v0.5 - (c)1994-97 S.Klose",10,13,"$"
TXT_nomore1:    db"A sampling rate set above $"
TXT_nomore2:    db"Hz will problably lead to an explosion of your sound device. ;-)",10,13,"$"
LST_cards:      db"sbpro",0,"pas",0,"oak",0,"wss",0,"int",_end
JMP_cards:      dw getsb,getpas,getoak,getwss,getinternal
LST_output:     db"8",0,"m",0,"o",0,"n",_end
jmp_output:     dw par_8,par_m,par_o,par_n
err_notthere:   db"Sorry, I can't detect your desired sound device.",10,13,"$"
ERR_sourcenotopen:
		db"Can't find your desired source file. Is it the right path ?",10,13,"$"
ERR_noMemory:   db"Ooops...sound buffer at 9000h:0000h doesn't seem to be sensible."
TXT_cr:         db 13,10,"$"
TXT_loading:    db"Loading...$"
TXT_playing:    db 13,"Playing $"
TXT_playing2:   db" channel module. '+' jumps to next pattern, other keys break...",13,"$"
TXT_name:       db "MOD-Name ?:$"
TXT_toolow:     db"Minimum mixing rate is 36000Hz. Sorry....",10,13,"$"
TXT_param:      db" No sound card detected. Following cards are supported :",10,13,10 ; Choose audio device via options if present :",10,13

TXT_plist:
  db" Parameters :",10,13,10
  db"   /SBPRO  Soundblaster pro",10,13
  db"   /PAS    Pro Audio Spectrum [+/16]",10,13
  db"   /WSS    Windows Sound System",10,13
  db"   /INT    Internal speaker",10,13
  db"   /o      Invokes 2x oversampling",10,13
  db"   /8      Force 8 bit output",10,13
  db"   /m      Force mono output",10,13
  db"   /n      No surround",10,13
  db 10,"$"
	db 1," Ceterum censeo Cannabis legalisandum esse ! ",1
ende:
	.data
sdev_set:       db ?
user_rate:      dw ?
Sourcehandle:   dw ?
NewSeg:         dw ?
SourceName:
sm:             db 64 dup ?
namebuffer:  
		db 66 dup ?
dwmodend:
