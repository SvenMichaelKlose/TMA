;TMA macro assembler
;Stupid XMS handling that should be replaced by something better. FIXME.
; (c)1996-1997 Sven Klose

	.code
;check_xmspos:
        pushf
        pusha
n1:     mov ax,crsr_addr
        add ax,distance         ;Cursor am Ende des Puffers ?
	cmp ax,filesize
        jbe >n1                 ;Nein, weiter...
        mov ebx,windowpos       ;šberlappt das aktuelle Fenster
        add ebx,windowsize      ;die XMS-Grenze ?
        cmp ebx,windowend
        jae >e1                  ;Ja, Ende.
        call insert_window      ;Aktuelle Seite ins XMS kopieren
        mov eax,distance
        sub crsr_addr,ax        ;Cursorposition korrigieren
        sub screen_addr,ax      ;Screenanfang korrigieren
        mov ebx,windowpos       ;Seitenposition nach hinten schieben
        add ebx,eax
        call read_window        ;und neues Fenster einlesen
e1:     popa
        popf
        ret

n1:	sub ax,distance
        cmp ax,distance         ;Cursor am Anfang der Seite ?
        jae >e1                 ;Nein, weiter
        mov ebx,windowpos       ;Seite bereits am Anfang des XMS ?
        or ebx,ebx
        jz >e1                  ;Ja, Ende
	call insert_window	;Aktuelle Seite ins XMS
        sub ebx,distance
        call read_window        ;Neue Seite einlesen
        mov ax,distance
        add crsr_addr,ax        ;Cursorposition korrigieren
        add screen_addr,ax      ;Screenanfang korrigieren
e1:     popa
        popf
        ret

; >>> Fgt neue Seite ins XMS ein
insert_window:
        pushad
        mov ax,xmshandle        ;Daten hinter Fenster auf neue Position kopieren
        mov xmshandle2,ax
        mov xmshandle3,ax
	mov eax,windowpos
	mov ebx,eax
        add eax,windowsize      ;Quellposition: Ende des alten Fensters
        mov ecx,windowend       ;Gr”sse der Restdaten
        sub ecx,eax
        add ebx,filesize        ;Ziel: Ende hinter neuem Window
        cmp eax,ebx
	je >l1
        mov xmssource,eax
        mov xmsdest,ebx
        test cl,11b
        je >l3
        add ecx,100b
        and cl,11111100b
        mov xmscopylen,ecx
        call xmscopy
        ;
        xor ax,ax               ;Neue Seite ins XMS kopieren
        mov xmshandle2,ax
        mov xmssource,ax
        mov ax,seg_text
        mov xmssource+2,ax
        ;
	mov eax,windowpos
	mov xmsdest,eax
        mov eax,filesize
        mov xmscopylen,eax
        call xmscopy            ;Neue Textseite einkopieren
        mov eax,filesize        ;Neue Textgr”sse berechnen
        sub eax,windowsize
	add windowend,eax
;        mov eax,filesize        ;und neue Fenstergr”sse bergeben
;        mov windowsize,eax
l1:     popad
        ret

; >>> Liest Seite aus dem XMS in dem Puffer
read_window:
        pushad
        mov xmssource,ebx       ;Kopierstruktur initialisieren
        mov windowpos,ebx
        mov ax,seg_text
        mov xmsdest+2,ax
        mov ax,xmshandle
        mov xmshandle2,ax
        xor ax,ax
	mov xmshandle3,ax
	mov xmsdest,ax
        ;
        mov eax,distance*4      ;Ende der Seite im XMS nach EBX
        add ebx,eax
        cmp ebx,windowend       ;šber das XMS-Ende hinaus ?
        jbe >l1                 ;Nein, weiter
	sub ebx,windowend	;Anzahl der berschssigen Bytes von
        sub eax,ebx             ;EAX abziehen
l1:     mov xmscopylen,eax      ;L„nge bergeben
        push ax
        call xmscopy
        pop ax
        push ds
        xor esi,esi
        mov si,ax               ;Nach Ende der letzen Zeile im Puffer suchen
        mov ds,seg_text
        std
        dec si
l1:     or si,si
        je >l2
        lodsb
        cmp al,lf
	jne l1
	;
l2:     add si,2
        cld
        pop ds
        mov windowsize,esi      ;und Gr”ssen bergeben
        mov filesize,si
	popad
        ret

; >>> Ruft XMS-Kopierfunktion auf <<<
xmscopy:push ds
        push cs
        pop ds
        pushad
        mov si,xmsstruc
        mov ah,0bh
        call xmsdriver
        or ax,ax
        if e nop
        popad
        pop ds
        ret

; ***************************************
; *** Routinen zur Speicherverwaltung ***
; ***************************************
; AllocMem:     Reserviert DOS-Speicher.
;               Das Ergebnis steht in dseg/DOSsize
; ram2xms:      

AllocMemErr:
        mov ax,3
        int 10h
        mov ah,9
        int 21h
        mov ax,4cffh
        int 21h

; *****************************************
; *** Reserviert Platz von DOS-Speicher ***
; *****************************************
AllocMem:
        push ds
        push es
        push cs
	pop ds
	call defragm	;Speicher defragmentieren
        mov cl,33
        mov al,128      ;Test auf 386 CPU
        shr al,cl
        or al,al
        jnz AllocMem2
	mov dx,ERR_no386
        jmp AllocMemErr
AllocMem4b:
        jmp AllocMem4
NoXMSb: jmp NoXMS
AllocMem2:
        mov ax,4300h    ;XMS installiert ?
        int 2fh
        cmp al,80h
        jne NoXMSb
        mov ax,4310h    ;Adresse fr Aufrufe holen.
        int 2fh
        mov XMSdradr,bx
        mov XMSdradr+2,es
        mov ah,3         ;A20 freischalten
        call XMSdriver
	mov ah,8	;freien Speicher holen
	call XMSdriver
        mov dx,ax       ;freien Speicher reservieren
        movzx eax,ax
        shl eax,10
        mov FreeXMS,eax ;freien Speicher vermerken
        mov ah,9
	call XMSdriver
        mov XMShandle,dx
        pop es
        pop ds
        ret                     ;Das war's.

NoXMS:  mov ax,3
        int 10h
        mov dx,ERR_noXMS
        mov ah,9
        int 21h
        xor ax,ax
        int 16h
        call capital
        cmp al,"R"
        je mmreset
	call Reallocmem
x1:	mov ax,4cffh
        int 21h

mmreset:int 19h

AllocMem4:
	push ax
        mov ax,3
        int 10h
        pop ax
        jmp x1

; *****************************************
; *** Gibt reservierten Speicher zurck ***
; *****************************************
ReallocMem:
;        mov ah,49h      ;DOS zurckholen
;        mov es,cs:dseg
;        int 21h
;        jc AllocMem4
;        ret

ReallocXMS:
	mov ds,cs	;XMS zurckholen
        mov dx,XMShandle
        mov ah,0dh
        mov ah,0ah      ;Block freigeben
        call XMSdriver
        ret

; ***********************************
; *** Defragmentiert den Speicher ***
; ***********************************
defragm:mov ah,52h
        int 21h
        mov bx,[es:bx-2]
        xor si,si
dfm2:   mov ds,bx
        call dfmcb
        jne dfm7
        mov dh,al
        cmp w[1],0
        jne dfm6
dfm3:   mov ax,ds
        inc ax
	add ax,cx
	mov ds,ax
        call dfmcb
        jne dfm4
        mov dh,al
        cmp w[1],0
        je dfm3
dfm4:	mov cx,ds
        sub cx,bx
        dec cx
        push ds
        mov ds,bx
        cmp cx,[3]
        je dfm5
        inc si
        mov [3],cx
        mov [0],dh
dfm5:   pop bx
        jmp dfm2
dfm6:   inc cx
        add bx,cx
        jmp dfm2
dfm7:   push cs
	pop ds
	ret

dfmcb:  mov al,[0]
        cmp al,"M"
        je dfmcb2
        cmp al,"Z"
	jne dfmcb3
dfmcb2: mov cx,[3]
dfmcb3: ret

; ======================
; >>> Gets file name <<<
; ======================
GetName:xor cx,cx
        cmp b[si],20h
        jb >l1
l2:     cmp b[si],21h
        jnb >l1
        inc si
        jmp l2
        ;
l1:     lodsb
	cmp al,21h
	jb >l2
        stosb
        inc cx
        jmp l1
l2:     mov b[di],0
        ret

