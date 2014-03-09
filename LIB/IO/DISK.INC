; Command line parser
; (c)1995-1996 Sven Klose

; ********************************************************
; *** Holt Dateinamen aus Kommandozeile und îffnet Sie ***
; ********************************************************
OpenFiles:
        mov si,82h      ;Dateinamen suchen
        mov di,SourceName
        call GetEnvName
        mov w[SourceLength],cx
        mov di,DestName
        call GetEnvName
        mov w[DestLength],cx
        mov dx,SourceName
        mov ax,3d00h      ;Datei îffnen
        int 21h
        jnc Open4       ;Kein Fehler,weiter
        mov dx,ERR_SourceNotOpen
        call ErrorBye
Open4:  mov bx,ax       ;Handle Åbergeben
        mov w[SourceHandle],ax
        
        xor cx,cx       ;Grîsse der Datei holen
        xor dx,dx
        mov ax,4202h
        int 21h
        push dx,ax
        xor dx,dx
        mov ax,4200h
        int 21h
        pop ax,dx
        mov bl,b[Pass]
        cmp bl,2
        je NoAdd
        xor dx,dx
        mov bx,100
        div bx
        inc ax
        db 66h
        add w[FileSize],ax
NoAdd:  jmp ReadSourceBlock2

; *** Liest Block aus Datei in den Puffer
ReadSourceBlock:
        db 66h
        mov bx,w[Filesize]
        db 66h
        mov ax,w[Proceed]
        db 66h
        xor dx,dx
        db 66h
        div bx
        call PrintWord
        mov dx,TXT_proceed
        mov ah,9
        int 21h
        mov al,w[Pass]
        xor ah,ah
        call PrintWord
        mov dx,TXT_proceed2
        mov ah,9
        int 21h

ReadSourceBlock2:
        mov bx,w[SourceHandle]
        mov ah,3fh      ;Block einlesen
        mov cx,-1       ;Soviel wie mîglich laden
        push ds,w[Inputseg];Zielsegment Åbergeben
        pop ds
        xor dx,dx       ;Zieladresse
        int 21h         ;Daten laden
        pop ds
        mov bx,ax
        mov w[InputSize],ax
        
        push bx,ax
        db 66h
        xor ax,ax
        pop ax
        shr ax,1
        db 66h
        add w[Proceed],ax

        pop bx
        xor ax,ax
        mov w[InputPointer],ax
        ret

; ******************************************************************
; *** Holt Dateinamen aus String und prÅft dessen Endung auf T64 ***
; ******************************************************************
; Aufruf: DS:SI=Quellstring DS:DI=Ziel
GetEnvName:
        push cs,cs
        pop ds,es
OpenLoop0:
        es:lodsb
        cmp al,' '
        je OpenLoop0
        cmp al,0dh      ;Ende der Zeile ?
        jne Open2       ;Nein,weiter ...
        mov dx,ERR_NoName
        jmp ErrorBye
Open2:  dec si
        xor cx,cx
OpenLoop3:
        es:lodsb
        stosb
        inc cx
        cmp al,' '
        je Open3
        cmp al,0dh
        jne OpenLoop3
Open3:  dec cx
        dec si
        dec di
        xor al,al       ;Ende des Namens kennzeichnen
        stosb
        ret

; ******************************
; *** Schliesst alle Dateien ***
; ******************************
CloseFiles:
        mov bx,w[SourceHandle]
        mov ah,3eh      ;Datei schliessen
        int 21h
        mov bx,w[DestHandle]
        mov ah,3eh
        int 21h
        ret

