        .code
; ===================================================
; >>> unlock Oak Mozart 16 pro extended registers <<<
; ===================================================
WrMPW:  mov dx,0f8fh            ;MC0
	mov al,0e2h
	out dx,al
	mov dx,cx
	in al,dx
	ret

; --------------------------------------------------------------------------
; Test auf Oak Mozart Karte
; --------------------------------------------------------------------------
GetOak: mov ds,cs
        mov cx,0f8dh
	call WrMPW              ;Passwort an Mozart-Register
        cmp al,-1
        je NoMozart             ;no Oak chip found
        push ax
	mov dx,TXT_mozart
	mov ah,9
	int 21h
        pop ax
        test al,80h
        jnz OAK_getWSS          ;no SB mode
        ;
        and al,1000b
        mov ah,2                ;Portnummer holen
	add al,20h
	mov w[SBport],ax
	shr al,4
	add al,"0"
	mov b[TXT_sbport],al
        call getoakirqdma

	call sbReset
        call sbReset            ;just to be careful
        mov b[sdev_bits],8+128
        mov w[sdev_maxrate],48000
        mov dx,TXT_sbfound2
        jmp Gotmozartsbmode

; WSS mode check
OAK_getwss:
        mov ah,2                ;Portnummer holen
        shr al,4
        and al,3
        mov bl,al
        xor bh,bh
        shl bx,1
        mov ax,w[oakportids+bx]
        mov w[WSSport],ax
        call getoakirqdma
        jmp gotmozartwssmode

NoMozart:
        stc
        ret

getoakirqdma:
        mov cx,0f8fh
	call WrMPW
	mov ah,al
	and al,11000000b        ;IRQ-Nummer
	mov dl,7
	or al,al
	jz MZgotIRQ
	cmp al,01000000b
	jnz MZIRQ5
	mov dl,11
	jmp MZgotIRQ
MZIRQ5: mov dl,5
MZgotIRQ:
	mov b[SBIRQNR],dl
        mov b[wssirq],dl
	add dl,"0"
	mov b[TXT_sbirq],dl

	and ah,00110000b        ;DMA-Nummer
	shr ah,4
	inc ah
	mov b[SBDMANR],ah
        add ah,2
        mov b[wssdma],ah
        sub ah,2
	add ah,"0"
	cmp ah,"4"
	jne MZTdmaOk
	mov dx,TXT_mozartDMAerr
	mov ah,9
	int 21h
	jmp NoMozart
MZTdmaOk:
	mov b[TXT_sbdma],ah
        add ah,2
        mov b[txt_wssdma],ah
        ret

Oakportids:
        dw 530h,0e80h,0f40h,604h

TXT_Mozart:
        db"Oak Mozart 16 pro found at port $"
TXT_mozartDMAerr:
	db"Warning : Oak Mozart 16 pro found with DMA disabled !",10,13
	db"Continuing with Soundblaster mode.",10,13,"$"
