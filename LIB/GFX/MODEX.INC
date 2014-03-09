	.code
; *****************************
; *** Videomodus einstellen ***
; *****************************
SetScrollMode:
        mov dx,03c4h    ;chain-4-adr off
        mov al,4
        out dx,al
        inc dx
        in al,dx
        and al,(not 8)
        out dx,al
        mov dx,3d4h     ;double word off
        push dx
        mov al,14h
        out dx,al
	inc dx
        in al,dx
        and al,(not 40h)
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
        mov al,640/8
        out dx,al
        ret
  
SetSplitScreen:
        ;Scoreanzeige einschalten
        ;BL=H”he der Anzeige
        call retrace
	mov ah,low(400)
        sub ah,bl
        mov dx,3d4h
        mov al,18h
        out dx,ax

        mov al,7
	out dx,al
	inc dx
        in al,dx
        or al,10h
        out dx,al
        dec dx
  
        mov al,9
        out dx,al
        call retrace
        inc dx
        in al,dx
        and al,not 40h
        out dx,al

        mov dx,3dah;dummy
	in al,dx

        mov dx,3c0h
        mov al,10h
        out dx,al
        inc dx
        in al,dx
	or al,20h
	dec dx
        out dx,al
        mov al,20h
        out dx,al
        ret

SetHires:
	mov dx,3d4h
	mov al,9
	out dx,al
	call retrace
	inc dx
	in al,dx
	and al,-40h
	out dx,al
	ret
