; *** SVGA User interface
; *** (c)1996 Sven Klose
        code segment
menuitem struc
menuitemstart:
        mittype:        db ?    ;item id
        mitnr:          db ?    ;additional info
        mitxpos:        dw ?    ;starting position
        mitypos:        dw ?
        mitheight:      dw ?    ;dimensions
        mitwidth:       dw ?
        mitstring:      dw ?    ;item's string adress
        mituser:        dw ?    ;adr of user defined routine
menuitemend:
ends
mptrstack=mptrstack2+256

mselwidth=172

; ========================
; >>> Switch mouse off <<<
; ========================
MouseOff:
        cli
        mov ax,0ch
        xor cx,cx
        int 33h
        sti
        ret

; =======================
; >>> Switch mouse on <<<
; =======================
MouseOn:cli
        push ds,es
        mov ax,3
        int 33h
        mov w[mouseposx],cx
        mov w[mouseposy],dx
        call ReadBackground
        call Writemouse
        mov ax,7
        xor cx,cx
        mov dx,639
        int 33h
        mov ax,8
        xor cx,cx
        mov dx,399
        int 33h
        mov ax,cs
        mov es,ax
        mov ax,0ch
        mov cx,111111b
        mov dx,Mousehandler
        int 33h
        pop es,ds
        sti
        ret

; =====================
; >>> Mouse handler <<<
; =====================
; Input from mouse driver:
;  AX=initial action
;  BX=button status
;  CX=X-Pos
;  DX=Y-Pos
noclickb:jmp noclick
Mousehandler:
        cli
        mov ax,cs
        mov ds,ax
        push ax,bx,cx,dx
        call ClearOldMouse
        pop dx,cx,bx,ax
        mov w[mouseposx],cx
        mov w[mouseposy],dx
        mov al,b[oldbut]
        mov b[oldbut],bl
        test al,1
        jz noclickb
        test bl,1
        jnz noclickb
        mov bp,Menustack
scanpanel:
        push bp
        mov ax,w[bp+mitxpos]    ;PrÅfen, ob Zeiger Åber einem Item liegt.
        cmp cx,ax
        jb Noclick2
        add ax,w[bp+mitwidth]
        cmp cx,ax
        ja noclick2
        mov ax,w[bp+mitypos]
        cmp dx,ax
        jb noclick2
        add ax,w[bp+mitheight]
        cmp dx,ax
        ja noclick2
        mov al,b[bp+mittype]
        ;
        cmp al,msel
        jne mhnosel
; ----- handles selectors
        mov al,b[bp+mitnr]
        xor ah,ah
        mov bx,w[bp+mituser]    ;move item nr to selector flag
        mov w[bx],ax
        mov si,w[bp+mitstring]
        mov cx,w[bp+mitxpos]
        mov bp,ax
        mov dx,326
        xor bx,bx
mdrms3: cmp b[si],0     ;end of selector ?
        jz noclick3
        mov b[printcolor],9     ;Farbe fÅr nicht aktiven Punkt
        cmp bp,bx
        jne mdrms6
        mov b[printcolor],15    ;Aktiver Punkt
mdrms6: add dx,12
        db 66h
        push w[cursorpos]
        mov w[Cursorpos],cx
        mov w[Cursorpos+2],dx
        push bp
        call Printtext          ;Text ausgeben
        pop bp
        db 66h
        pop w[cursorpos]
        inc bx
        jmp mdrms3

noclick2:
        pop bp
        add bp,menuitemend-menuitemstart
        cmp bp,w[freespace]
        jb scanpanel2
        jmp noclick
noclick3:pop bp
NoClick:call ReadBackground
        call WriteMouse
        retf

scanpanel2:
        jmp scanpanel

; ----- jump into sub menu
mhnosel:cmp al,msub
        jne mhnosub
        mov ax,w[menupos]
        pushs mptrstack,ax      ;save old adr on stack
        mov si,w[bp+mitstring]
        call drmj               ;scan end of string
        mov w[menupos],si
        call Drawmenu
        pop bx
        jmp noclick

; ----- jump to previous menu
mhnosub:cmp al,mback
        jne mhnoback
        pops mptrstack,ax
        mov w[menupos],ax
        call Drawmenu
        pop bx
        jmp noclick
mhnoback:
; ----- call sub menu
        cmp al,mgosub
        jne mhnogosub
        mov ax,w[menupos]
        pushs mptrstack,ax      ;save old adr on stack
        mov ax,w[bp+mituser]
        mov w[menupos],ax
        call Drawmenu
        pop bx
        jmp noclick
mhnogosub:
; ----- call user defined routine and sub menu
        cmp al,mcgosub
        jne mhnocgosub
        mov ax,w[menupos]
        pushs mptrstack,ax      ;save old adr on stack
        mov ax,w[bp+mituser]
        mov w[menupos],ax
        call Drawmenu
        pop bx
        jmp noclick
mhnocgosub:
; ----- call user defined routine
        cmp al,mcall
        jne mhnocall
        mov bx,w[bp+mituser]    ;call user defined routine (button click)
        pusha
        push ds,es
        call bx
        pop es,ds
        popa
        pop bx
        jmp noclick
mhnocall:
; ----- handles file box/calls user defined file handler
        cmp al,mbox
        jne mhnobox

        mov ax,cs
        mov es,ax
        mov ax,w[mouseposx]
        sub ax,w[bp+mitxpos]
        mov bx,w[boxcolwidth]
        xor dx,dx
        div bx
        xor dx,dx
        mov bx,w[boxrows]
        mul bx
        mov bx,w[mouseposy]
        sub bx,2
        sub bx,w[bp+mitypos]
        shr bx,3
        add ax,bx
        add ax,w[boxofs]
        mov si,auswahl+1
        mov cx,ax
        or cx,cx                ;string found
        jz >l1
        push ax
        dec si
        mov di,si
l2:     push cx
        xor al,al
        mov cx,-1
        repne scasb
        pop cx
        loop l2
        mov si,di
        pop ax

; ----- call user routine with entry number in AX and string adr in SI
l1:     mov bx,w[bp+mituser]
        pusha
        push ds,es
        call bx
        pop es,ds
        popa
        pop bp
        jmp noclick
mhnobox:pop bp
        jmp noclick

; ====================
; >>> Draws Button <<<
; ====================
btw=96
bth=16
DrawButton:
        push si,cx,dx
        mov ax,cs
        mov ds,ax
        xor al,al       ;outer border
        mov bx,w[mbtw]
        call DrHline
        mov bx,w[mbth]
        call DrVline
        push dx
        add dx,w[mbth]
        mov bx,w[mbtw]
        inc bx
        call DrHline
        pop dx
        add cx,w[mbtw]
        mov bx,w[mbth]
        call Drvline
        ;
        pop dx,cx
        inc cx
        inc dx
        mov al,b[mbth]
        dec al
        mov bx,w[mbtw]
        sub bx,2
        call Drhline    ;obererm weisser Rand
        mov bx,w[mbth]
        dec bx ;sub bx,2
        call DrVline    ;linker, weisser Rand
        push cx,dx,dx
        add dx,w[mbth]
        sub dx,2
        mov al,8
        mov bx,w[mbtw]
        sub bx,2
        inc cx
        call Drhline   ;unterer, grauer Rand
        pop dx
        add cx,w[mbtw]
        sub cx,3
        mov al,8
        mov bx,w[mbth]
        dec bx
        call Drvline            ;rechter, grauer Rand
        pop dx,cx
        inc cx,dx
        mov al,7
        mov bx,w[mbtw]
        sub bx,3
        mov bp,w[mbth]
        sub bp,3
        call FillSqr
        inc cx
        add dx,3
        mov w[cursorpos],cx
        mov w[cursorpos+2],dx
        mov b[Printcolor],0

        pop si
        mov di,si
        mov es,cs
        mov cx,100
        xor al,al
        repnz scasb
        mov bx,100
        sub bx,cx
        shl bx,3
        mov ax,w[mbtw]
        sub ax,bx
        shr ax,1
        add ax,2
        add w[cursorpos],ax
        xor al,al
        call Printtext
        ret

; =========================================
; >>> Text auf den Bildschirm schreiben <<<
; =========================================
PrintText:
        push cx,dx,ds
; ----- Zeichen aus String holen
PrintPlane3:
        cs:lodsb
        or al,al                ;Ende des Strings
        jz PrintEnd
        cmp al,1
        jne PPnoNewLine
        mov w[cs:CursorPos],0
        add w[cs:CursorPos+2],8
        jmp PrintPlane3
PPnoNewLine:
        call PrintChar
        jmp PrintPlane3
printEnd:
        pop ds,dx,cx
        ret

PrintChar:
        pusha
        push ds,es
        push ax
        mov ax,w[cs:CharSetSeg]
        mov ds,ax
        mov es,0a000h
; ----- Bildschirmadresse brerechnen
PrintPlane2:
        mov ax,w[cs:CursorPos+2]
        shl ax,5
        mov di,ax
        shl ax,2
        add di,ax
        mov ax,w[cs:CursorPos]
        mov bh,al               ;actual plane number
        shr ax,2
        add di,ax
        pop ax
        xor ah,ah
        shl ax,3
        add ax,w[cs:CharSetAdr]
        mov bp,ax
        mov bl,b[cs:PrintColor]
        mov cx,4                ;4 Planes
        mov dx,3c4h             ;plane setzen
pplane: push cx,bp,di
        push cx
        and bh,3
        mov ax,102h
        mov cl,bh
        shl ah,cl
        out dx,ax
        pop ax
        mov cl,4
        sub cl,al
        mov ch,8                ;8 Zeilen ausgeben
pp2:    mov al,[ds:bp]          ;Zeile aus Zeichensatz holen
        shl al,cl
        test al,128             ;Ist das Bit gesetzt ?
        jz >B1                  ;Nein, weiter
        mov b[es:di],bl
        ;
b1:     inc di
        test al,8              ;Ist das Bit gesetzt ?
        jz >B1                  ;Nein, weiter
        mov b[es:di],bl
        ;
b1:     inc bp                  ;Zeiger auf nÑchste Zeile
        add di,160-1
        dec ch
        jnz pp2
        pop di,bp,cx
        inc bh
        test bh,4
        jz >b1
        inc di
b1:     dec cx
        jnz pplane
        add w[cs:CursorPos],8
        pop es,ds
        popa
        ret

; =======================
; >>> Initialisierung <<<
; =======================
init:   mov ax,cs               ;Zeichensatz holen (8*8)
        mov ds,ax
        xor ax,ax               ;Test on mouse
        int 33h
        or ax,ax
        jnz mousefound
        mov dx,ERR_nomouse
        mov ah,9
        int 21h
        jmp bye2                ;end of program
mousefound:
        mov ax,0fh
        mov cx,4
        mov dx,4
        int 33h
        mov ax,1130h
        mov bh,3
        int 10h
        mov w[CharsetAdr],bp
        mov w[CharsetSeg],es
        ret

ERR_nomouse:
        db"This program requires a mouse !",10,13,"$"

; ==================================
; >>> Zeichnet horizontale Linie <<<
; ==================================
; CX/DX=Pos AL=color BX=LÑnge
; ----- line's left edge
DrHline:push cx,dx
        shl dx,5        ;scr adr
        mov di,dx
        shl di,2
        add di,dx
        push bx,cx,ax
        mov ax,cx
        xor al,3
        and ax,3
        sub bx,ax       ;substract left edge from line length
        dec bx
        ;
        mov ax,cx
        shr ax,2
        add di,ax
        ;
        mov ah,15
        and cl,3
        shl ah,cl
        and ah,15
        mov al,2
        mov dx,3c4h
        out dx,ax
        pop ax
        mov es,0a000h   ;guess
        stosb
; ----- middle part
        mov cx,bx
        shr cx,2
        push ax
        mov ax,0f02h
        out dx,ax
        pop ax
        mov ah,al
        push ax,ax
        db 66h
        pop ax
        shr cx,1
        jnc >l1
        stosb
l1:     shr cx,1
        jnc >l2
        stosw
l2:     jcxz >l3
        db 66h
        rep stosw
l3:
        pop cx,bx
        add cl,bl
        and cx,3
        jcxz >l4
; ----- line's right edge
        mov ah,0f0h
        rol ah,cl
        and ah,15
        push ax
        mov al,2
        out dx,ax
        pop ax
        mov b[es:di],al
l4:     pop dx,cx
        ret

; ================================
; >>> Zeichnet vertikale Linie <<<
; ================================
; CX/DX=Pos AL=color BX=LÑnge
DrVline:push cx,dx
        shl dx,5
        mov di,dx
        shl di,2
        add di,dx
        push ax
        mov ax,cx
        shr ax,2
        add di,ax
        mov ah,1
        and cl,3
        shl ah,cl
        mov al,2
        mov dx,3c4h
        out dx,ax
        pop ax
        mov cx,bx
        mov es,0a000h
drvline2:
        mov b[es:di],al
        add di,160
        loop drvline2
        pop dx,cx
        ret

; ====================
; >>> FÅllt FlÑche <<<
; ====================
; CX/DX=Pos BX=breite BP=hîhe
Fillsqr:push cx,dx
        mov si,cx
        mov cx,bp
fillsqr2:
        push cx
        mov cx,si
        push bx
        call DrHline
        pop bx
        inc dx
        pop cx
        loop Fillsqr2
        pop dx,cx
        ret

; ================================
; >>> Zeichnet geladene Bitmap <<<
; ================================
; DS:SI=Quelle CX=Hîhe DI=Ziel im Screen
DrawBitmap:
        mov ax,0a000h
        mov es,ax
l1:     push cx                 ;zeilen
        mov cx,4
l2:     push cx,si,di
        mov al,4
        sub al,cl
        mov cl,al
        mov ah,1
        shl ah,cl
        mov al,2
        mov dx,3c4h
        out dx,ax
        mov cx,160
l3:     mov al,[si]
        stosb
        add si,4
        dec cx
        jnz l3
        pop di,si,cx
        inc si
        dec cx
        jnz l2
        add si,639
        add di,160
        pop cx
        dec cx
        jnz l1
        ret

; ======================
; >>> Zeichnet Menue <<<
; ======================
DrawMenu:
        cli
; ----- draw borders
        mov ds,cs
        mov dx,315
        xor cx,cx
        mov bx,640
        mov al,9
        call DrHline
        xor cx,cx
        mov dx,316
        mov bx,640
        mov bp,82
        mov al,1
        call fillsqr
        mov dx,398
        xor al,al
        mov bx,640
        call drhline
        inc dx
        mov al,9
        mov bx,640
        call drhline
; ----- Initialize data for new menu
        mov si,w[menupos]
        mov w[freespace],menustack
        mov w[panelrightbrdr],640
        mov w[msgpos],2
        mov w[my],320
        mov b[gosubmode],0
        xor bp,bp               ;menu level indicator
        mov w[mx],4
drm2:   lodsb                   ;get item id
        call SetMITTYPE         ;store item id
; ----- back from submenu
        cmp al,mback
        jne drnoback
        cmp b[gosubmode],0
        jz drnoreturn
        pops mptrstack,si
        mov b[gosubmode],0
        jmp drm2
drnoreturn:
        call DrawMButton
        dec bp
        jc drm2b
        jmp drm2
drm2b:  mov b[rewritemode],0
        sti
        ret
drnoback:
; ----- user defined button (calls routine)
        cmp al,mcall
        jne drnocall
        lodsw                   ;adress of user routine
        call DrawMButton
        jmp drm2
drnocall:
; ----- jump into submenu
        cmp al,msub
        jne drnosub
        call DrawMbutton
        inc bp                  ;level+
        jmp drm2
; ----- selector
drnosub:cmp al,msel
        jne drnosel
        call DrawMenuSelector   ;draw selector
        jmp drm2
; ----- call user defined routine and jump into submenu
drnosel:cmp al,mcsub
        jne drnomcsub
        lodsw
        call DrawMbutton
        inc bp
        jmp drm2
drnomcsub:
; ----- dummy button
        cmp al,mdummy
        jne drnodummy
        call DrawMbutton
        jmp drm2
drnodummy:
; ----- Message at screen bottom
        cmp al,mshow            ;not finished yet
        jne drnoshow
        call drawmessage
        jmp drm2
drnoshow:
; ----- steps into subdata
        cmp al,mgosub
        jne drnogosub           ;this'll need some consideration
        lodsw                   ;adr of new menu
        pushs mptrstack,si
        mov b[gosubmode],1
        mov si,ax
        jmp drm2
drnogosub:
; ----- call user defined routine and jump into subdata
        cmp al,mcgosub
        jne drnocgosub
        lodsw                   ;user routine
        mov di,ax
        lodsw                   ;new menu data
        xchg di,ax
        call DrawMbutton
        jmp drm2
drnocgosub:
; ----- Menu description ends
        cmp al,mend
        jne drnoend
        mov b[rewritemode],0
        sti
        ret
drnobox2:
        jmp drnobox
; ----- initializes file box
drnoend:cmp al,mbox
        jne drnobox2
        or bp,bp
        jz >l1
        add si,14               ;ignore, not this level
        jmp drm2
l1:     push bp
        mov bp,w[freespace]     ;adress of free menu item entry
        lodsw                   ;upper left position
        mov w[boxxpos],ax
        mov w[bp+mitxpos],ax
        lodsw
        mov w[bp+mitypos],ax
        mov w[boxypos],ax
        lodsw                   ;width in pixels/column
        mov w[boxcolwidth],ax
        mov bx,ax
        lodsb                   ;columns in a row
        xor ah,ah
        mov di,ax
        shl di,1
        mov w[boxwidth],di      ;width vertical lines between columns
        mov w[bp+mitwidth],ax
        push bx
        xor ah,ah
        xor dx,dx
        mul bx
        pop bx
        add w[boxwidth],ax      ;+ columns themselves
        add w[bp+mitwidth],ax
        lodsw
        mov w[boxcolheight],ax
        mov bx,ax
        lodsb
        xor ah,ah
        mov w[boxrows],ax    ;rows
        xor dx,dx
        mul bx
        add ax,4
        mov w[boxheight],ax
        mov w[bp+mitheight],ax
        lodsw                   ;Initialize box
        mov bx,ax
        pusha
        push ds,es
        call bx                 ;Output:AX=Adress of table
        pop es,ds
        mov w[boxadr],ax
        popa
        lodsw
        mov w[boxjump],ax       ;user defined file handler
        mov w[bp+mituser],ax
        add bp,menuitemend-menuitemstart
        mov w[freespace],bp
        pop bp
        call drawbox
        jmp drm2

drnobox:cmp al,mbutton
        jne drnobutton
        or bp,bp
        jz >l1
        add si,10
        call drmj
        jmp drm2
l1:     mov al,mcall            ;define page buttons
        call Setmittype
        lodsw                   ;xpos
        mov cx,ax
        lodsw                   ;ypos
        mov dx,ax
        lodsw                   ;width
        mov w[mbtw],ax
        lodsw                   ;height
        mov w[mbth],ax
        lodsw                   ;handler
        call DefMIT
        pusha
        call Drawbutton
        popa
        call drmj
        jmp drm2
drnobutton:
        cmp al,mhline
        jne drnohline
        or bp,bp
        jz l1
        add si,7
        jmp drm2
l1:     lodsw
        mov cx,ax
        lodsw
        mov dx,ax
        lodsw
        mov bx,ax
        mov bx,100
        lodsb
        pusha
        push ds,es
        call DrHline
        pop es,ds
        popa
        jmp drm2

drnohline:
        mov dx,ERR_undefdmenuitem
        jmp error
drms1b: jmp drms1

SetMITTYPE:
        mov bx,w[freespace]
        mov b[bx+mittype],al
        mov b[hmittype],al
        ret

; =====================
; >>> Draw selector <<<
; =====================
DrawMenuSelector:
        or bp,bp        ;is it the right menu level ?
        jnz drms1b
        db 66h
        push w[Cursorpos]
        mov cx,w[panelrightbrdr] ;get position of selector
        sub cx,mselwidth              ;selector width
        mov w[panelrightbrdr],cx
        mov dx,315
        mov bx,84
        mov al,0
        call drvline
        inc cx
        mov al,9
        mov bx,84
        call drvline
        add cx,8
        mov dx,322
        mov b[Printcolor],0
        lodsw
        mov w[hmituser],ax      ;flag adr
        mov bp,ax
        mov bp,w[bp]
        add cx,2
        mov w[Cursorpos],cx
        mov w[Cursorpos+2],dx
        push si                 ;draw selector title double
        call PrintText
        pop si
        sub cx,2
        sub dx,2
        mov b[printcolor],15
        mov w[Cursorpos],cx
        mov w[Cursorpos+2],dx
        call PrintText
        mov w[hmitstring],si      ;text adr
        mov dx,326
        xor bx,bx
drms3:  cmp b[si],0     ;end of selector ?
        jz drmsend
        mov b[printcolor],9
        cmp bp,bx
        jne drms6
        mov b[printcolor],15
drms6:  add dx,12
        mov w[Cursorpos],cx
        mov w[Cursorpos+2],dx
        mov di,w[freespace]     ;define item
        mov al,b[hmittype]
        mov b[di+mittype],al
        mov b[di+mitnr],bl               ;(item number)
        mov w[di+mitxpos],cx
        mov w[di+mitypos],dx
        mov w[di+mitheight],12
        mov w[di+mitwidth],120
        mov ax,w[hmituser]
        mov w[di+mituser],ax
        mov ax,w[hmitstring]
        mov w[di+mitstring],ax
        add di,menuitemend-menuitemstart
        mov w[freespace],di
        push bx,bp
        call Printtext
        pop bp,bx
        inc bx
        jmp drms3

drmsend:xor bp,bp
        inc si
        db 66h
        pop w[Cursorpos]
        ret

drms1:  add si,2        ;scan end of menu selector
drms2:  call drmj
        cmp b[si],0
        jne drms2
        inc si
        ret

; =====================
; >>> Draws message <<<
; =====================
DrawMessage:
        or bp,bp
        jnz drnomsg
        db 66h
        push w[cursorpos]
        mov ax,w[msgpos]
        inc ax
        mov w[cursorpos],ax
        mov dx,390
        mov w[cursorpos+2],dx
        mov b[Printcolor],0
        push si
        call PrintText
        pop si
        mov ax,w[msgpos]
        mov w[cursorpos],ax
        mov dx,389
        mov w[cursorpos+2],dx
        mov b[printcolor],15
        call PrintText
        mov ax,w[cursorpos]
        mov w[msgpos],ax
        db 66h
        pop w[cursorpos]
        ret
drnomsg:jmp drmj

DrawMbutton:
        or bp,bp                ;only draw buttons on actual menu level
        jnz drmj
        mov cx,w[mx]
        mov dx,w[my]
        mov w[mbtw],btw
        mov w[mbth],bth
        call Defmit
        push bp,ds
        call DrawButton
        pop ds,bp
        add w[my],bth
        cmp w[my],380
        jb drmb2
        mov w[my],320
        add w[mx],btw
drmb2:  ret

drmj:   push cx
        mov di,si
        mov es,cs               ;move si to string end
        xor al,al
        mov cx,-1
        repnz scasb
        mov si,di
        pop cx
        ret

DefMIT: mov bx,w[freespace]
        mov w[bx+mitstring],si
        mov w[bx+mituser],ax
        mov w[bx+mitxpos],cx
        mov w[bx+mitypos],dx
        mov ax,w[mbth]
        mov w[bx+mitheight],ax
        mov ax,w[mbtw]
        mov w[bx+mitwidth],ax
        add bx,menuitemend-menuitemstart
        mov w[freespace],bx
        ret

DrawBox:pusha
        push si
;        mov cx,w[boxxpos]
;        mov dx,w[boxypos]
;        mov bx,w[boxwidth]
;        mov bp,w[boxheight]
;        mov al,1
;        call Fillsqr
        mov ax,cs
        mov es,ax
        xor dx,dx
        mov di,si
l1:     mov cx,-1
        repne scasb
        inc dx
        cmp b[di],0
        jne l1
        dec dx
        mov w[boxcnt],dx
        call Fillbox
        pop si
        popa
        ret

Fillbox:
; ----- Get first shown entry
        mov si,w[Boxadr]
        mov cx,w[boxofs]
        or cx,cx
        jz fnoofs
        mov di,si
        mov ax,cs
        mov es,ax
fofs:   push cx
        mov cx,-1
        xor al,al
        repne scasb
        pop cx
        loop fofs
        mov si,di

fnoofs:
        mov bx,w[boxwidth]
        mov cx,w[boxxpos]
        mov dx,w[boxypos]
        mov bp,w[boxheight]
        xor al,al
        push si
        call Fillsqr
        pop si
        add cx,5                ;text start
        add dx,2
fillbox2:
        mov w[cursorpos],cx
        mov w[cursorpos+2],dx
        lodsb
        and al,127              ;ignore the highest bit
        mov b[Printcolor],al
        call Printtext
        cmp b[si],0             ;end of items ?
        je boxfilled2
        add dx,8
        mov ax,w[boxypos]       ;next line possible ?
        sub ax,4
        add ax,w[boxheight]
        cmp dx,ax
        jb nonewcolumn
        mov dx,w[boxypos]       ;no, new column
        add cx,w[boxcolwidth]
        sub cx,5
        mov bx,w[boxheight]
        inc bx
        mov al,1
        call DrVline
        inc cx
        call DrVline
        mov ax,cx
        add ax,w[boxcolwidth]
        mov bx,w[boxxpos]       ;next column possible ?
        add bx,w[boxwidth]
        cmp ax,bx
        jae boxfilled
        add dx,2
        add cx,6
nonewcolumn:
        jmp fillbox2
boxfilled2:
        add cx,w[boxcolwidth]
        sub cx,4
boxfilled:
        mov dx,w[boxypos]
        dec cx
        mov bx,w[boxheight]
        inc bx
        mov al,1
        call DrVline
        inc cx
        call DrVline
        mov cx,w[boxxpos]       ;draws bottom line in lt. blue
        mov dx,w[boxypos]
        mov bx,w[boxwidth]
        dec bx
        inc cx
        add dx,w[boxheight]
        mov al,9
        call DrHline
        add cx,w[boxwidth]      ;draws right line in lt. blue
        sub cx,2
        mov bx,w[boxheight]
        mov dx,w[boxypos]
        inc dx
        call drvline
        mov dx,w[boxypos]
        xor cx,cx
        mov bx,640
        sub dx,2
        mov al,9
        call Drhline
        dec dx
        xor al,al
        call drhline
        ret

; ==============================================
; >>> Internal error messages of menu system <<<
; ==============================================
error:  mov ax,3
        int 10h
        push dx
        mov dx,MSG_savingWork
        mov ah,9
        int 21h
        call Line
        pop dx
        mov ah,9
        int 21h
        mov al,">"
        int 29h
        mov al," "
        int 29h
        dec si
        mov cx,78
error2: lodsb
        cmp al,32
        jae error3
        mov al,"."
error3: int 29h
        loop error2
        jmp bye2

ERR_undefdmenuitem:
        db"Undefined menu item.",10,13,"$"

; ===============================
; >>> Restores old background <<<
; ===============================
Clearoldmouse:
        mov si,OldBackground
        mov ax,cs
        mov ds,ax
        mov ax,0a000h
        mov es,ax
        mov di,w[mouseposy]
        shl di,5
        mov ax,di
        shl ax,2
        add di,ax
        mov ax,w[mouseposx]
        mov bl,al
        shr ax,2
        add di,ax
        mov bp,3c4h
        mov bh,4
com1:   push di
        and bl,3
        mov cl,bl
        mov ah,1
        shl ah,cl
        mov al,2
        mov dx,bp
        out dx,ax
        mov dl,16
com2:   db 66h
        movsw
        add di,156
        dec dl
        jnz com2
        inc bl
        pop di
        test bl,4
        je com3
        inc di
com3:   dec bh
        jnz com1
        ret

; ===========================
; >>> Read old background <<<
; ===========================
ReadBackground:
        mov di,OldBackground
        mov ax,cs
        mov es,ax
        mov ax,0a000h
        mov ds,ax
        mov si,w[cs:mouseposy]
        shl si,5
        mov ax,si
        shl ax,2
        add si,ax
        mov ax,w[cs:mouseposx]
        mov bl,al
        shr ax,2
        add si,ax
        mov bp,3ceh
        mov bh,4
rob1:   push si
        and bl,3
        mov ah,bl
        mov al,4
        mov dx,bp
        out dx,ax
        mov dl,16
rob2:   db 66h
        movsw
        add si,156
        dec dl
        jnz rob2
        inc bl
        pop si
        test bl,4
        je rob3
        inc si
rob3:   dec bh
        jnz rob1
        ret

; =========================
; >>> Draws mouse arrow <<<
; =========================
Writemouse:
        mov si,Arrow_1
        mov ax,cs
        mov ds,ax
        mov ax,0a000h
        mov es,ax
        mov di,w[mouseposy]
        shl di,5
        mov ax,di
        shl ax,2
        add di,ax
        mov ax,w[mouseposx]
        mov bl,al
        shr ax,2
        add di,ax
        mov bp,3c4h
        mov bh,4
wm1:    push si,di
        and bl,3
        mov cl,bl
        mov ah,1
        shl ah,cl
        mov al,2
        mov dx,bp
        out dx,ax
        mov dl,16
wm2:    mov cx,4
wm3:    mov al,[si]
        cmp al,"0"
        je wm4
        mov b[es:di],15
wm4:    inc di
        add si,4
        loop wm3
        add di,156
        dec dl
        jnz wm2
        pop di,si
        inc bl
        test bl,4
        jz wm5
        inc di
wm5:    inc si
        dec bh
        jnz wm1
        ret

; ==============
; > Mauszeiger <
; ==============
Arrow_1:db"1000000000000000"
        db"1110000000000000"
        db"0111100000000000"
        db"0111111000000000"
        db"0111111110000000"
        db"0011111100000000"
        db"0011111000000000"
        db"0011111100000000"
        db"0001100110000000"
        db"0001100011000000"
        db"0001000001100000"
        db"0000000000100000"
        db"0000000000000000"
        db"0000000000000000"
        db"0000000000000000"
        db"0000000000000000"
        code ends
        
        data segment
oldbut:dw ?
mbtw:dw ?
mbth:dw ?
boxofs:dw ?
boxxpos:dw ?
boxypos:dw ?
boxadr:dw ?
boxcolwidth:dw ?
boxwidth:dw ?
boxcolheight:dw ?
boxheight:dw ?
boxrows:dw ?
boxlastx:dw ?
playdir:dw ?
gosubmode:db ?
freespace:dw ?
menupos:dw ?
msgpos:dw ?
panelrightbrdr:dw ?
CharsetAdr:dw ?
CharsetSeg:dw ?
PrintColor:dw ?
CursorPos:dd ?
mx:     dw ?
my:     dw ?
mouseposx:dw ?
mouseposy:dw ?
hmituser:dw ?
hmitstring:dw ?
hmittype:db ?
boxjump:dw ?
boxcnt:dw ?
rewritemode:db ?
oldbackground:
        db 256 dup ?
mptrstack2:
        dw 256 dup ?
menustack:dw 1024 dup ?
        data ends
