; *** VESA interface
; *** (c)1996 Sven Klose
	
	.data
Modeinfoblock:
; mandatory information
	ModeAttributes	    dw	?  ; mode attributes
	WinAAttributes	    db	?  ; window A attributes
	WinBAttributes	    db	?  ; window B attributes
	WinGranularity	    dw	?  ; window granularity
	WinSize 	    dw	?  ; window size
	WinASegment	    dw	?  ; window A start segment
	WinBSegment	    dw	?  ; window B start segment
	WinFuncPtr	    dd	?  ; pointer to windor function
	BytesPerScanLine    dw	?  ; bytes per scan line

; formerly optional information (now mandatory)

	XResolution	    dw	?  ; horizontal resolution
	YResolution	    dw	?  ; vertical resolution
	XCharSize	    db	?  ; character cell width
	YCharSize	    db	?  ; character cell height
	NumberOfPlanes	    db	?  ; number of memory planes
	BitsPerPixel	    db	?  ; bits per pixel
	NumberOfBanks	    db	?  ; number of banks
	MemoryModel	    db	?  ; memory model type
	BankSize	    db	?  ; bank size in kb
	NumberOfImagePages  db	?  ; number of images
	nowt		    db	?  ; reserved for page function

; new Direct Color fields

	RedMaskSize	    db	?  ; size of direct color red mask in bits
	RedFieldPosition    db	?  ; bit position of LSB of red mask
	GreenMaskSize	    db	?  ; size of direct color green mask in bits
	GreenFieldPosition  db	?  ; bit position of LSB of green mask
	BlueMaskSize	    db	?  ; size of direct color blue mask in bits
	BlueFieldPosition   db	?  ; bit position of LSB of blue mask
	RsvdMaskSize	    db	?  ; size of direct color reserved mask in bits
	DirectColorModeInfo db	?  ; Direct Color mode attributes
	nowt2		    db	216 dup(?)	; remainder of ModeInfoBlock

vesalinewidth	dw ?	;bytes per scan line
vesasave:	dw ?
vesasave2:	dw 257 dup ?
vesastack:	dw 1032 dup ?
vesabuffer:	db 1256 dup ?

	.code
GetVESA:mov ds,data
	mov ax,4f00h	;get SVGA info
	call vesa
	cmp al,4fh
	je vesapresent
	mov dx,ERR_novesa
vesaerr:push dx
	mov ax,3
	int 10h
	pop dx
	mov ds,data
	mov ah,9
	int 21h
	call vesadeinit
	mov ax,4cffh
	int 21h
vesapresent:
	mov ax,4f01h	;get mode information
	mov cx,101h
	mov es,data
	mov di,Modeinfoblock
	call vesa
	or ah,ah
	jnz vesaferr
	mov ax,4f02h	;set videomode 640*400*256
	mov bx,101h
	call vesa
	or ah,ah
	jnz vesaferr
ret
	mov ax,4f06h		;set logical resolution
	xor bl,bl
	mov cx,640
	call vesa
	or ah,ah
	jnz vesaferr
	cmp dx,800
	jb noresolution
	mov ds,data
	mov w[VESAlinewidth],cx ;bytes per scan line
	ret

noresolution:
	mov dx,ERR_noresolution
	jmp vesaerr
vesaferr:
	mov dx,ERR_vesacallfailed
	jmp vesaerr

; *****************
; *** Scrolling ***
; *****************
; CX=X-Pos DX=Y-Pos
SetDisplaystart:
	mov ax,4f07h
	xor bx,bx
	call vesa
	ret
	
; ************************
; *** Set video window ***
; ************************
; EAX=relative adress
Setwindow:
        xor ebx,ebx
	mov fs,data
	mov bx,w[fs:wingranularity]
	shl ebx,10
	xor edx,edx
	div ebx
	xchg ax,dx
	push ax
	xor bx,bx
	mov ax,4f05h
	call vesa
	ret

vesainit:
	mov ax,data
	mov ds,ax
	mov es,ax
	xor si,si
	mov di,vesabuffer
	mov cx,128
	rep movsw
	ret

vesadeinit:
	mov ax,data
	mov ds,ax
	mov es,ax
	xor di,di
	mov si,vesabuffer
	mov cx,128
	rep movsw
	ret

vesa:	mov fs,data
	mov w[fs:vesasave2],ax
	mov ax,sp
	mov w[fs:vesasave],ax
	mov ax,vesastack
	mov sp,ax
	mov ax,w[fs:vesasave2]
	int 10h
	mov w[fs:vesasave2],ax
	mov ax,w[fs:vesasave]
	mov sp,ax
	mov ax,w[fs:vesasave2]
	ret

#if !_mdltiny
_text	segment
#endif

ERR_novesa:
	db"No VESA graphics driver installed. Please start one of the drivers supported",10,13
	db"with this program in the VESA directory and try again.",10,13
	db"If errors occur please refer to the user's manual.",10,13,"$"
ERR_noresolution:
	db"Your SVGA doesn't provide the logical resolution for double buffering.",10,13
	db"This is important to guarantee smooth scrolling and flicker free",10,13
	db"sprites. Maybe more video RAM will do nicely.",10,13,"$"
ERR_vesacallfailed:
	db 7,"VESA function call failed.",10,13,"$"

