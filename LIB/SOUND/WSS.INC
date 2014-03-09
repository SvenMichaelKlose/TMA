	.code

;Wait for Codec to be finished
WaitCODEC:
        ; initializing. Carry set if error
	clc
        mov dx,WSSport          ;dx = CODEC Index Address Register
        add dx,4
	mov cx,2000h
waitcodec2:
        in al,dx
	or al,al		;wait until bit 7 is zero or 2000h
        jns waitcodec3          ;reads
        loop waitcodec2
        stc                     ;if read 2000h times, there is a
                                ;problem with the CODEC
waitcodec3:
	ret

wsserr: mov ah,9
        push cs
        pop ds
	int 21h
	call reallocmem
	mov ax,4cffh
	int 21h

;*******************
;*** Startet WSS ***
;*******************
getwss: push cs
	pop ds
        mov dx,WSSport
        add dx,4                ;dx = CODEC Index Address Register
	in al,dx		;is the CODEC busy?
        or al,al
        jns wss_notbusy
        stc
        ret

wss_notbusy:
	mov al,0Ch		;select misc. register
	out dx,al
	inc dx
	in al,dx		;AL = CODEC version
	mov bl,al
	xor al,al
	out dx,al		;write 0 to misc. register
	in al,dx		;and read it back
	cmp al,bl		;if value changed this is not a
	je wssfound		;CODEC
	stc			;value changed - not a CODEC
	ret

wssfound:
	mov al,wssdma
	add al,"0"
	mov [txt_wssdma],al
	mov dx,TXT_wssfound
	mov ah,9
	int 21h
gotmozartwssmode:
	mov b[sdev_bits],16+128
	mov w[sdev_maxrate],48000
	mov ax,w[wssport]
	call wssprinthex
	mov dx,TXT_wssfound2
	mov ah,9
	int 21h
	mov ax,wssinit
	mov [sdev_start],ax
	mov ax,wssclose
	mov [sdev_stop],ax
	;now search for closest match of the mixing rate from the sdev_rates
	;table

	mov cx,16		;16 possible values
	xor si,si		;pointer to rate table
	mov dx,32767		;distance from best match
	xor bx,bx		;rate number for best match

wssrl:	mov ax,wssrates[si]	;get a rate from table
	add si,2		 
	sub ax,[DMSresolution]	;distance from wanted mixing rate
	js wss1 		;if this rate is smaller, ignore
	cmp ax,dx		;is distance greater than best match?
	jae wss1		;if is, ignore

	mov bx,wssrates[si]	;rate number for this match
	mov dx,ax		;distance

wss1:	add si,2		;next rate
	loop wssrl

	mov wssSpd,bl		;store rate number
	shl bx,2
	mov ax,wssrates[bx]	;get actual mixing rate from table
	mov [DMSresolution],ax	;store actual mixing rate
	clc			;WSS found
	ret

; ========================
; >>> Start WSS output <<<
; ========================
wssinit:mov ds,cs
	mov ax,wsstimingcheck
	mov [modtimingcheck],ax

	mov bl,WSSdma
	cmp bl,0
	je wssDMA0
	cmp bl,1
	je wssDMA1
	cmp bl,3
	je wssDMA3
	mov dx,ERR_wssinvdma	;invalid DMA number
	jmp wsserr

wssDMA0:or al,01h		;DMA value for CODEC configuration
	jmp wssDMAd

wssDMA1:or al,02h		;DMA value for CODEC configuration
	jmp wssDMAd

wssDMA3:or al,03h		;DMA value for CODEC configuration

wssDMAd:mov dx,WSSport
	out dx,al		;set IRQ and DMA numbers

	;WSS does _NOT_ seem to use any interrupts if using autoinit
	;DMA, so setting a IRQ-handler is unnecessary.


	;Start playing the DMA buffer:
	xor ax,ax
	mov [DMAbufferAdr],ax
	mov [DMAbufferlen],ax
	mov al,WSSdma
	call StartDMA

	mov dx,WSSport
	add dx,4
	mov al,0Ah
	out dx,al
	inc dx
	in al,dx		;external mute on
	or al,40h
	out dx,al

	mov cx,1200h		;delay to prevent clicks (value from
				; CODEC.ASM, ExtMute, WSS SDK 1.0)
wssw1:	in al,84h		;a "safe" I/O port
	loop wssw1


	mov si,2

	;For some unknown reason this has to be done twice. Don't ask me
	;why. Apparently something isn't initialized quite as it should
	;be, but this seems to work fine this way.

wssratelp:
	call WaitCODEC

	mov dx,WSSport
	add dx,4		;enable MCE and select Clock and
	mov al,48h		;Data Format Register (08h)
	out dx,al
        inc dx
	in al,dx
	in al,dx
	mov al,wssSpd		;Clock Frequency Source & Divide
	cmp b[DMSmode],1
	if z or al,40h		;16-bit signed linear (0 - 8-bit
				;unsigned linear)
	test b[sdev_bits],128	;stereo?
	if nz or al,10h 	;if yes, set stereo bit 1
	out dx,al
        in al,dx
	in al,dx
	call WaitCODEC

	dec si			;do it again...
	jnz wssratelp

	mov dx,WSSport
	add dx,4
	mov al,49h		;retain MCE
	out dx,al
	inc dx
	mov al,04h or 08h	;single DMA channel, enable
	out dx,al		;autocalibration

	call WaitCODEC

	mov dx,WSSport
	add dx,4
	mov al,08h		;disable MCE
	out dx,al

wsswacal:
	mov dx,WSSport
	add dx,4
	mov al,11
	out dx,al		;wait until autocalibration is
	inc dx			;finished
	in al,dx
	test al,32
	jnz wsswacal


	mov cx,1200h		;delay to prevent clicks (value from
				; CODEC.ASM, ExtMute, WSS SDK 1.0)
wssw2:	in al,84h		;a "safe" I/O port
	loop wssw2

	mov dx,WSSport
	add dx,4
	mov al,0Ah
	out dx,al
	inc dx			;external mute off
	in al,dx
	and al,NOT 40h
	out dx,al


	mov dx,WSSport
	add dx,6		;acknowledge CODEC interrupt (just
	xor al,al		; for safety...)
	out dx,al


	mov dx,WSSport
	add dx,4		;select the lower base count
	mov al,0Fh
	out dx,al
	inc dx
	mov al,255		;set the low byte of count (DMAC
	out dx,al		;takes care of wrapping)

	dec dx
	mov al,0Eh		;select the upper base count
	out dx,al
	inc dx
	mov al,255		;set the high byte of count
	out dx,al


	mov dx,WSSport
	add dx,4		;write to the Interface Configuration
	mov al,09h
	out dx,al

	inc dx
	mov al,05h		;use DMA playback
	out dx,al

	dec dx
	mov al,06h
	out dx,al		;mute off from left channel
	inc dx
	in al,dx
	mov wssOLMute,al
	and al,NOT 128
	out dx,al

	dec dx
	mov al,07h
	out dx,al		;mute off from right channel
	inc dx
	in al,dx
	mov wssORMute,al
	and al,NOT 128
	out dx,al
	ret

;*****************
;*** Stops WSS ***
;*****************
wssClose:
	mov ds,cs
	mov dx,WSSport
	add dx,4

	mov al,06h
	out dx,al		;old mute setting to left channel
	inc dx
	mov al,wssOLMute
	out dx,al

	dec dx
	mov al,07h
	out dx,al		;old mute setting to right channel
	inc dx
	mov al,wssORMute
	out dx,al

	dec dx			;Pin Control Register
	mov al,0Ah
	out dx,al
	inc dx
	xor al,al		;turn off interrupts
	out dx,al

	inc dx
	out dx,al		;acnowledge outstanding interrupts

	sub dx,2
	mov al,09h		;Interface Configuration Register
	out dx,al
	inc dx
	xor al,al		;turn off CODEC's DMA
	out dx,al
	ret

; =======================================
; >>> framecheck for playing routines <<<
; =======================================
wsstimingcheck:
	mov al,wssdma
	call GetDMApos
	mov bx,ax
	mov ax,w[modoldmixpos]
	sub ax,bx
	mov bx,w[DMSoutlen]
	cmp ax,bx
	ja >l1
	clc			;no frame to be played
	ret
l1:	stc			;frame needed
	ret

wssprinthex:
	rol ax,8
	call wssprdigit
	rol ax,4
	call wssprdigit
	rol ax,4
	call wssprdigit
	ret

wssprdigit:
	push ax
	and al,0fh
	mov bl,"0"
	cmp al,10
	jb >n1
	mov bl,"A"-10
n1:	add al,bl
	int 29h
	pop ax
	ret

#if !_mdltiny
_text   segment
#endif

;*** GÅltige Samplingraten fÅr WSS ***
wssRates:
	dw 8000,0
	dw 5513,1
	dw 16000,2
	dw 11025,3
	dw 27429,4
	dw 18900,5
	dw 32000,6
	dw 22050,7
	dw 0,8			;not supported
	dw 37800,9
	dw 0,10
	dw 44100,11
	dw 48000,12
	dw 33075,13
	dw 9600,14
	dw 6615,15

txt_wssfound:
	db"Windows sound system detected at port $"
txt_wssfound2:
	db"h using DMA "
txt_wssdma:
	db"?.",10,13,"$"

wssPortAddresses:
	dw 530h, 604h, 0E80h, 0F40h

ERR_wsscodecbusy:
	db"Windows sound system initialization failed - CODEC is busy",10,13,"$"
ERR_wssnocodec:
	db"WSS: No CODEC found - check base port.",10,13,"$"
ERR_wssinvdma:
	db"WSS: Invalid DMA number - please correct it.",10,13,"$"
ERR_wssdmaerr:
	db"Couldn't detect WSS DMA channel !",10,13,"$"

	.data
wssSpd		db ?   ;WSS speed value
wssport 	dw ?
wssOLMute	db ?   ;WSS old left channel mute value
wssORMute	db ?   ;WSS old right channel mute value
wssDMAPos	dw ?	;DMA playing position
wssdma		db ?
wssirq		db ?

