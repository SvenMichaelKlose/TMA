; ======================
; >>> Flammeneffekte <<<
; >>> Orgasmusersatz <<<
; ======================
        .code
InitFlames:
        mov ax,13h
        int 10h
        call getcolors
        mov di,FlameBuffer
        mov cx,320*4
InFl2:  push cx
        mov si,FlamePart
        mov cx,FlamePrtEnd+1
        sub cx,FlamePart
        rep movsb
        pop cx
        loop InFl2
        mov al,0c3h
        stosb
        mov ax,cs
        add ax,1000h
;mov ax,0a000h
        mov flamesegA,ax
        mov es,ax
        add ax,800h
        mov flamesegB,ax
        xor ax,ax
        mov cx,8000h
        rep stosw
        mov ax,16*3
        mov w[Fades],ax
        mov ax,768
        mov w[Fades],ax
        call FullFade
FlameLoop:
        in al,60h
        cmp al,1
        je Ende
        call DoFlame
        jmp FlameLoop
Ende:   call ClearNewCols
        call FullFade
        mov ax,3
        int 10h
        mov ax,4c00h
        int 21h

FlameIntensity:dw 50

DoFlame:push cs
        pop ds
        mov es,flamesegA
        mov di,32000-320
        xor eax,eax
        mov cx,80
        mov cx,w[FlameIntensity]
        push 40h
        pop ds
        mov si,w[6ch]
        push 0f000h
        pop ds

DoFl2:  lodsb                   ;Zuf„llig Punkte setzen
        mov bl,al
        xor bh,bh
        xor ah,ah
        shr ax,2
        add bx,ax
        add bx,32000-320
        xor al,al
        mov b[es:bx+3],al
        mov ax,0fefeh
        mov w[es:bx-1],ax
        mov w[es:bx+1],ax
        add si,bx
        add si,cx
        loop DoFl2
        
        std
        mov di,99*320-1         ;Neue Seite berechnen
        mov cx,25
        mov ds,w[cs:FlameSegA]
        mov es,w[cs:FlameSegB]
        xor ah,ah
DoFl3:  call FlameBuffer
        loop DoFl3
        cld
        push es                 ;Flammen darstellen
        pop ds
        push 0a000h
        pop es
        xor si,si
        mov di,32320
        mov cx,8000
        rep movsd
        push cs
        pop ds
        push flamesegA,flamesegB
        pop flamesegA,flamesegB
        ret

FlamePart:
        xor bx,bx
        mov al,b[di+319]
        add bx,ax
        mov al,b[di+320]
        add bx,ax
        mov al,b[di+321]
        add bx,ax
        mov al,b[di]
        add ax,bx
        shr ax,2
        jz FlamePrtEnd
        dec al
FlamePrtEnd:    
        stosb


NME_FlamePalette:
  db"FLAMES.PAL",0
        .data
FlameSegA       dw 0
FlameSegB       dw 0
FlameBuffer:    dd 32*320 dup 0,0
