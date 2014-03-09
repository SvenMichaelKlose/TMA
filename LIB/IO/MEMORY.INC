; ***************************************
; *** Routinen zur Speicherverwaltung ***
; ***************************************
; Diese Routine reserviert sÑmtlichen freien Speicher und setzt
; FS und GS auf 0 und weist ihnen 4Gbyte Adressraum zu
; Routinen:
; AllocMem:     Reserviert DOS & XMS-Speicher.
;               Die nîtigen Werte stehen in DOSsegstart/DOSsize & XMSbase/XMSsize
;               ausserdem wird die Routine
; GETREALBIG    aufgerufen, welche das FS+GS-Register fÅr sÑmtlichen Speicher
;               nutzbar macht.
        .code
descriptor struc
	limit0_15:      dw ?
	basis0_15:      dw ?
	basis16_23:     db ?
	zugriff:        db ?
	extra:          db ?
	basis24_31:     db ?
ends
;        org $ and 0fffbh+8
gdt_adr:dw 32
	dd ?
gdt_start:
	db 8 dup 0	;Dummy
gdt1:	dw 0ffffh,0
	db 0,9ah,0,0	;Codesegment

	dw 0ffffh,0	;DS,ES,FS+GS, 4Gbyte ab Adresse 00000000h verfÅgbar
	db 0,92h,8fh,0

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
	push ds,es,cs
	pop ds
	push ax
	or al,al
	jz allocmem1	;no message
	mov dx,TXT_allocmem
	mov ah,9
	int 21h
allocmem1:
;	 call defragm	 ;Speicher defragmentieren
	mov cl,33
	mov al,128	;Test auf 386 CPU
	shr al,cl
	or al,al
	jnz AllocMem2
	mov dx,ERR_no386
	jmp AllocMemErr
AllocMem2:
        mov eax,cr0     ;Test auf real mode
	test al,1
	jz AllocMem3
	mov dx,ERR_virtual
	jmp AllocMemErr
AllocMem4b:
	jmp AllocMem4
NoXMSb: jmp NoXMS
AllocMem3:
        push cs
        pop es
	mov ah,4ah      ;Programmspeicher auf 64k setzen
	mov bx,1000h
	int 21h
	jc AllocMem4b   ;Fehler, Ende.
        xor ebx,ebx
	mov ah,48h      ;Grîsse des Speichers holen
	mov bx,-1
	int 21h
	cmp al,8
	jne AllocMem4b
        mov eax,ebx
        shl eax,4
        mov [DOSsize],eax
	mov ah,48h
	int 21h
	jc AllocMem4b
        mov w[DOSsegstart],ax

	mov ax,4300h    ;XMS installiert ?
	int 2fh
	cmp al,80h
	jne NoXMSb
	mov ax,4310h    ;Adresse fÅr Aufrufe holen.
	int 2fh
	mov w[XMSdradr],bx
	mov w[XMSdradr+2],es
	mov ah,3        ;A20 freischalten
	call XMSdriver
	mov ah,8        ;freien Speicher holen
	call XMSdriver
	mov dx,ax       ;freien Speicher reservieren
        push ax
        xor eax,eax
        pop ax
        shl eax,10
        mov [FreeXMS],eax ;freien Speicher vermerken
	mov ah,9
	call XMSdriver
	mov w[XMShandle],dx
	mov ah,0ch      ;Block sichern und lineare Adresse holen
	call XMSdriver
	mov w[XMSbase],bx
	mov w[XMSbase+2],dx

        pop ax
        or al,al
        je nomemsize
        mov eax,[DOSsize]
        add eax,[FreeXMS]
        mov ebx,10
        xor cx,cx
l1:     xor edx,edx
        div ebx
        push dx
        inc cx
        db 66h
        or ax,ax
        jnz l1
l2:     pop ax
        add al,"0"
        int 29h
        loop l2

        mov dx,TXT_found
        mov ah,9
        int 21h
nomemsize:
        pop es,ds

; *********************************
; *** Real Big Mode einschalten ***
; *********************************
GetRealBig:
	pushf
	push ds,es
        xor eax,eax
	mov ax,cs
	mov ds,ax
	mov w[rseg],ax          ;Codesegmentdeskriptor vorbereiten
        shl eax,4
        mov ebx,eax
        shr eax,16
	mov w[gdt1+basis0_15],bx
	mov b[gdt1+basis16_23],al
        xor eax,eax
	mov ax,cs               ;24-Bit-Adresse der GDT vorbereiten
        shl eax,4
        add eax,gdt_start
        mov [gdt_adr+2],eax
        lgdt w[gdt_adr]
	cli
; ---- causes triple fault reset
mmreset:mov eax,cr0 ;Ab in den Protected mode
        or al,1
        mov cr0,eax
	db 0eah
	dw offset pmode
	dw 8
Pmode:  mov ax,16
	mov es,ax
	mov ds,ax
        mov fs,ax           ;FS+GS laden
        mov gs,ax
        mov eax,cr0     ;zurÅck in den Real mode
        and al,0feh
        mov cr0,eax
	db 0eah
        dw rmode
rseg:   dw 0
rmode:  pop es,ds
	popf
	ret                     ;Das war's.

NoXMS:  mov ax,3
	int 10h
	mov dx,ERR_noXMS
	mov ah,9
	int 21h
	xor ax,ax
	int 16h
	cmp al,"r"
	je mmreset
	cmp al,"R"
	je mmreset
	call ReallocDOS
	mov ax,4cffh
	int 21h

AllocMem4:
	push ax
	mov ax,3
	int 10h
	pop ax
	jmp DOSerrorRoutine

; *****************************************
; *** Gibt reservierten Speicher zurÅck ***
; *****************************************
ReallocMem:
	call ReallocXMS
	call ReallocDOS
	ret
ReallocXMS:
        push cs         ;XMS zurÅckholen
        pop ds
	mov dx,w[XMShandle]
	mov ah,0dh
	push dx         ;Block entsichern
	call XMSdriver
	pop dx
	mov ah,0ah      ;Block freigeben
	call XMSdriver
	ret
ReallocDOS:
	mov ah,49h      ;DOS zurÅckholen
        mov es,w[cs:DOSsegstart]
	int 21h
	jc AllocMem4
	ret

; ***********************************
; *** Defragmentiert den Speicher ***
; ***********************************
defragm:mov ah,52h
	int 21h
	mov bx,w[es:bx-2]
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
dfm4:   mov cx,ds
	sub cx,bx
	dec cx
	push ds
	mov ds,bx
	cmp cx,w[3]
	je dfm5
	inc si
	mov w[3],cx
	mov b[0],dh
dfm5:   pop bx
	jmp dfm2
dfm6:   inc cx
	add bx,cx
	jmp dfm2
dfm7:   push cs
	pop ds
	ret

dfmcb:  mov al,b[0]
	cmp al,"M"
	je dfmcb2
	cmp al,"Z"
	jne dfmcb3
dfmcb2: mov cx,w[3]
dfmcb3: ret

XMSdriver:
	db 9ah
XMSdradr:
	dw 0,0
	ret

TXT_allocmem:
        db"Spaceman's integrated mode extender v1.0 - (c)1995-1996 Spaceman/Tetrabyte",10,13,"$"
TXT_found:
        db" bytes of memory found and used.",10,13,"$"
ERR_noXMS:
	db"I didn't find any XMS driver like HIMEM.SYS.",10,13
	db"Please install one and try again.",10,13,10
	db"Press 'R' to reboot now or hit any key to continue...$"
ERR_no386:
	db"No 386 CPU present. Time to upgrade !",10,13,"$"
ERR_virtual:
	db"Virtual mode detected ! Please remove any memory drivers like",10,13
	db"EMM386 or QUEMM and don't run this program under Windows or DESQview.",10,13,"$"

        .data
XMSbase:dd ?
XMSsize:dd ?
FreeXMS:dd ?
XMShandle:dw ?
DOSsegstart: dw ?
DOSsize:dd ?
