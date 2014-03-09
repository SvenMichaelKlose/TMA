; *********************************************
; *** FÅhrt Y-pack durch (FUCKING FAST !!!) ***
; *********************************************
Ypack:  cli
        cld
        push ax,bx,cx,dx,si,di,ds,es
        xor si,si       ;Alle nîtigen Werte in Register laden.
        mov di,si
        mov cx,32000
        mov bl,b[cs:RawByte]
        mov es,w[cs:packseg]
        mov ds,w[cs:Unpackseg]
YGetByte:
        cmp si,cx       ;Ende des Quellblocks erreicht ?
        je YEnd         ;Ja,Ende der Routine.
        es:lodsb        ;Byte aus Quellblock holen.
        mov ah,al       ;Wert fÅr Stringtest merken.
        cmp al,bl       ;Entspricht das Byte dem Code ?
        jne YNoCode     ;Nein,weiter.
        xor bh,bh       ;Ja,Code als Dummystring Åbergeben.
        mov ax,bx
        xchg es,ds
        es:stosw
        xchg es,ds
        jmp YGetByte
YNoCode:
YTestOnString:          ;nachfolgende Bytes auf Kette testen.
        mov dx,si       ;Alte Position im Quellblock merken.
        mov bh,1        ;ZÑhler fÅr KettenlÑnge auf 1 setzen.
YGetStringByte:
        cmp si,cx       ;Ende des Quellblocks erreicht ?
        jne YNoEnd      ;Nein,weiter.
        jmp YWriteCode  ;Ja,Kettenangabe schreiben.
YNoEnd: es:lodsb        ;Nachfolgendes Byte holen.
        cmp al,ah       ;Ist das Byte gleich Testwert ?
        jnz YWriteCode2 ;Nein,Kettenangabe schreiben.
        inc bh
        cmp bh,-1       ;Ende der mîglichen KettenlÑnge erreicht ?
        jne YGetStringByte;Nein,weitertesten.
YWriteCode2:
        dec si
YWriteCode:             ;Schreibt Kettenangabe wenn mîglich.
        cmp bh,1        ;KettenlÑnge zu klein ?
        je YStringNotOk   ;Nein,Angabe schreiben
        cmp bh,2
        je YStringNotOk
        cmp bh,3
        jne YStringOk
YStringNotOk:
        mov al,ah       ;Ja,normalen Wert schreiben,
        xchg es,ds
        es:stosb
        xchg es,ds
        mov si,dx       ;Quellzeiger korrigieren
        jmp YGetByte    ;und weiterpacken.
YEnd:   mov w[cs:SourceSize],di
        sti
        pop es,ds,di,si,dx,cx,bx,ax
        ret
YStringOk:
        xchg es,ds
        Push ax
        mov ax,bx       ;Code und LÑnge schreiben
        es:stosw
        pop ax          ;Kettenwert schreiben
        mov al,ah
        es:stosb   
        xchg es,ds
        jmp YGetByte    ;wahrscheinlich durch falschen Wert erreicht ist.

; *********************************
; *** Entpackt einen Datenblock ***
; *********************************
YUnpack:push ax,bx,cx,dx,si,di,es,ds
        xor si,si       ;Register mit nîtigen Werten laden
        mov di,si
        mov bl,b[RawByte]
        mov dx,w[SourceSize]
        mov es,w[UnpackSeg]
        mov ds,w[PackSeg]
YUGetByte:
        cmp si,dx       ;Ende der Daten erreicht ?
        je YUEnd        ;Ja,Ende.
        es:lodsb        ;Nein,Byte holen.
        cmp al,bh       ;Ist es eine Kette ?
        je YUWriteChain ;Ja,Schreiben
YUWriteByte:
        xchg ds,es
        es:stosb
        xchg ds,es
        jmp YUGetByte
YUWriteChain:
        es:lodsb        ;KettenlÑnge holen.
        or al,al        ;Dummy string ?
        jnz YUChainOK   ;Nein,weitermachen.
        mov al,bl       ;Ja,Code schreiben
        jmp YUWriteByte
YUChainOk:
        mov cl,al       ;LÑnge zum SchleifenzÑhler
        xor ch,ch
        es:lodsb
        xchg es,ds      ;Kette schreiben
        rep es:stosb
        xchg es,ds
        jmp YUGetByte   ;Weitermachen
YUEnd:  pop ds,es,di,si,dx,cx,bx,ax
        ret

SourceSize:     
        dw 10000
RawByte:db 1fh
PackSeg:dw 0
Unpackseg:dw 0

