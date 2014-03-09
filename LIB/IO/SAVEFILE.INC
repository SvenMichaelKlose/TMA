; ***********************************
; *** Speichern eines Datensatzes ***
; ***********************************
;DS:DX=Dateiname
;ES:DI=Quellesegment
;ECX  =L„nge des Datenblocks

SaveFile:
        push cx
        mov ah,3ch      ;Datei erstellen
        xor cx,cx
        int 21h
        mov bx,ax
        pop cx

SaveFile3:
        push cx
        db 66h
        cmp cx,8000h    ;Ist mehr als 32k da ?
        dw 0
        jb SaveFile2
        mov cx,8000h    ;Ja,32k Block speichern
SaveFile2:
        mov ah,40h      ;Block in Datei schreiben
        push ds,es
        pop ds
        mov dx,di
        int 21h
        pop ds,cx
        mov ax,es
        add ax,800h
        mov es,ax
        db 66h
        sub cx,8000h
        dw 0
        jnc SaveFile3   ;Weiter wenn noch was da ist

        mov ah,3eh      ;Sonst Datei schliessen.
        int 21h
        ret
