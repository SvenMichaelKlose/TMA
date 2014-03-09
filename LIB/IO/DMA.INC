; *** DMA routines
; *** (c)1994-1996 Sven Klose

	.code
struc
	DMArequest:	db ?
	DMAsingleMask:	db ?
	DMAmode:	db ?
	DMAflipflop:	db ?
ends
        
; *******************************
; *** Initialize DMA transfer ***
; *******************************
; al=DMA channel
StartDMA:
        call GetDMAchannel
        mov b[DMAnr],al         ;Switch channel off
        push ax
        and al,3
        mov dl,b[bp+DMArequest]
        out dx,al
	pop ax
	and al,3
        or al,8+16+64
        mov dl,b[bp+DMAmode]
        out dx,al               ;Set DMA mode

	mov al,b[DMAbufferseg+1];Page
        shr al,4
        mov bl,b[dmaNr2]
	xor bh,bh
	mov dl,b[DMApagetab+bx]
        out dx,al

        mov dl,b[bp+DMAflipflop]
        out dx,al               ;clear flipflop
        call GetDMAadrPort
        mov ax,w[DMAbufferadr]
        cmp b[dmanr],3
        jbe >l1
        shr ax,1
l1:     out dx,al
        mov al,ah
        out dx,al
	inc dx
	mov ax,w[DMAbufferlen]
        cmp b[DMAnr],3
        jbe >l1
        shr ax,1
        or ax,ax
	jnz >l1
        mov ax,8000h
l1:     dec ax
	out dx,al		;L„nge des Blocks-1
	mov al,ah
        out dx,al

        mov al,b[dmaNr]
        mov dl,[bp+DMAsinglemask]  ;Kanal freigeben
        out dx,al
        ret
        
; ******************
; *** Stoppt DMA ***
; ******************
; al=channel
StopDMA:call GetDMAchannel
	mov al,b[DMAnr]
	and al,3
	or al,4
	mov dl,[bp+DMAsinglemask]
	out dx,al
	ret

GetDMAadrport:
	mov dl,b[dmaNr] 	;Portadresse berechnen
	shl dl,1
	cmp b[dmanr2],3 	;high dma ?
	jna setdma3
	add dl,0c0h
setdma3:xor dh,dh
	ret

GetDMAchannel:
	push cs
	pop ds
	mov b[DMAnr2],al
	mov bp,SlaveDMA
	cmp al,3
	jna SetDMA2
	mov bp,MasterDMA	;High DMA
	sub al,4
SetDMA2:xor dh,dh
	ret

; **********************************
; *** Holt aktuelle DMA-Position ***
; **********************************
; AL=DMA nr. out: AX=pos
GetDMApos:
	pushf
	call GetDMAchannel
	mov dl,b[bp+DMAflipflop] ; clear Flip-Flop port
	out dx,al

	call GetDMAadrPort
	cli
getdmapos2:
	in al,dx
	mov ah,al
	in al,dx		; bx = first word count
	xchg al,ah
	mov bx,ax
	in al,dx
	mov ah,al		; ax = second word count
	in al,dx
	xchg al,ah
	sub bx,ax		; now compare the two results
	cmp bx,2		; if the difference is over 4, read again
	jg getdmapos2
	cmp bx,-2
	jl getdmapos2
	popf
	ret

#if !_mdltiny
_text	segment
#endif

; ***********************************
; *** Portadressen der DMA-Kan„le ***
; ***********************************
DMApageTab:	db 87h,83h,81h,82h,8fh,8bh,89h,8ah
SlaveDMA:	db 09h,0ah,0bh,0ch
MasterDMA:	db 0d2h,0d4h,0d6h,0d8

	.data
DMAnr:		db ?
DMAnr2: 	db ?
DMAbufferlen:	dw ?
DMAbufferadr:	dw ?
DMAbufferseg:   dw ?

