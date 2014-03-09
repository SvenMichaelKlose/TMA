        cli             ;Interrupt sperren
        xor ax,ax       ;DS auf Vektorsegment
        push ax
        pop ds
        mov ax,w[08h*4]  ;Vektor sichern
        cs:mov w[spoldoff],ax
        mov ax,w[08h*4+2]
        cs:mov w[spoldseg],ax
        mov ax,speakout ;und auf eigene Routine richten
        push cs
        mov bx,08h*4
        mov w[bx],ax
        pop ax
        mov w[bx+2],ax
        mov al,00110100b ;Samplingrate ca. 11KHz 
        out 43h,al
        mov al,069/2     
        out 40h,al      ;(INT 08H)
        mov al,0
        out 40h,al
        mov al,10010000b ;Timer auf Empfang
        out 43h,al
        in al,61h       ;Lautsprecher an
        or al,3
        out 61h,al
        sti
        ret


; **********************************************
; *** ôffnet WAV-Datei und lÑdt ersten Block ***
; **********************************************
speakstart:
        push cs         ;DS = Codesegment
        pop ds
        mov dx,tunename ;TUNE.WAV îffnen
        mov ax,3d00h
        int 21h
        jnc SpeakFileOk
        mov dx,ERR_SpTune
        jmp Error
SpeakFileOk:
        mov w[speakhandle],ax ;Handle sichern
        mov ax,8000h
        mov w[SplAdress],ax


; ************************************
; *** LÑdt 32k Block der WAV-Datei ***
; ************************************
speakload:              ;32K-Block laden
        mov bx,w[speakhandle]
        mov cx,32*1024
        mov dx,w[spladress] ;Neue Adresse berechnen
        add dx,32*1024
        mov w[spladress],dx
        push w[spseg]   ;Segmentregister laden
        pop ds
        mov ah,3fh
        int 21h
        or ax,ax
        jz speaktuneoff            ;Ende der Datei ?
        ret


; **************************************************
; *** Prueft ob Block geladen werden muss/tut es ***
; **************************************************
testonspload:
        push ax,bx,cx,dx,si,di,es,ds
        mov ds,cs
        mov al,b[sploadit]
        mov ah,b[spsi+1]
        and ah,128
        cmp al,ah
        jne testonspend
        push ax
        call speakload
        pop ax
        mov ds,cs
        sub al,128
        mov b[sploadit],al
testonspend:
        pop ds,es,di,si,dx,cx,bx,ax
        ret


; ********************************
; *** Schaltet Playroutinen ab ***
; ********************************
speaktuneoff:
        cli
        push ds
        mov al,110100b
        out 43h,al
        mov al,0ffh     ;Alte Interruptgeschwindigkeit
        out 40h,al
        out 40h,al
        mov ah,3eh      ;WAV-Datei schliessen
        mov bx,w[speakhandle]
        int 21h
        xor ax,ax       ;Alte Vektoren wiederholen
        push ax
        pop ds
        cs:mov ax,w[spoldoff]
        mov w[08h*4],ax
        cs:mov ax,w[spoldseg]
        mov w[08h*4+2],ax

        in al,61h       ;Lautsprecher aus
        and al,11111100b
        out 61h,al
        pop ds
        sti
        ret


; *****************************************************
; *** Gibt WAV-Byte an internem Lautsprecher weiter ***
; *****************************************************
speakout:
        cli
        push ax,si,ds,dx ;Register retten
        push w[cs:spseg]
        pop ds
;        mov si,w[cs:spsi]
;        lodsb
;        jmp NoSoundBlaster
        mov dx,22ch     ;Abspielbefehl an die SB-Karte
spo2:   in al,dx
        or al,al
        js spo2
        mov al,10h
        out dx,al
        
        mov si,w[cs:spsi] ;Wert von Sample holen
        lodsw
        mov ah,al
spo3:   in al,dx
        or al,al
        js spo3
        mov al,ah
        out dx,al       ;Soundblaster spielen lassen ...
        
NoSoundBlaster:
;        shr al,1        ;Frequenzmodulation
;        out 42h,al      ;Signal Åber internen Speaker ausgeben
        mov w[cs:spsi],si
        mov al,20h
        out 20h,al
        pop dx,ds,si,ax
        sti
        iret


; *******************************
; *** Daten fuer Playroutinen ***
; *******************************
spsi:   dw 0
spseg:  dw 0
sploadit:
        dw 128;1
spoldoff:
        dw 0
spoldseg:
        dw 0
speakhandle:
        dw 0
spladress:
        dw 0

