;TMA macro assembler
;(c)1998 Sven Klose
; Remote memory block functions for DOS/XMS

	.code
; >>> Allocate remote memory block
; In:	DX	Size in k
; Out:	"       Handle
; Note: Will terminte on error.
rmb_allocate:
	mov ah,9
	push bx,bp
	call xmsdriver
	or ax,ax
	je xmserror
	pop bp,bx
	ret

rmb_APIinit:
	push ds,es
#if !_mdltiny
	mov ds,data
#else
	mov ds,fs
#endif
	mov ax,4300h	;XMS installied ?
	int 2fh
	cmp al,80h
	jne NoXMS
	mov ax,4310h	;Get XMS function
	int 2fh
	mov w[XMSdriver],bx
	mov w[XMSdriver+2],es

ka:	mov ah,3	;free A20 gate
	callf [XMSdriver]
	mov ah,8	;alloc XMS
	callf [XMSdriver]
	mov dx,ax
	shr dx,1	;Take a half of available XMS
	push ax
	xor eax,eax
	pop ax
	shl eax,10
	mov d[rmb_free],eax
	mov ah,9
	callf [XMSdriver]
	mov w[XMShandle],dx
	mov eax,offset reallocmem
	call register_module
	pop es,ds
	ret

rmb_APIdeinit:		;free XMS
	mov dx,w[XMShandle]
	mov ah,0ah
	callf [XMSdriver]

free_segments:
	mov cx,[segments]	;Free segments
	mov bx,segflags
l1:	push cx,bx
	mov dx,seg_xmshandle[bx]
	mov w seg_xmshandle[bx],0
	or dx,dx
	je >n1
	mov ah,0ah
	callf [XMSdriver]
n1:	pop bx,cx
	add bx,seg_size
	loop l1
	ret

NoXMS:	mov ax,3
	int 10h
	mov dx,ERR_noXMS
	mov ah,9
	int 21h
	xor ax,ax
	int 16h
	cmp al,"r"
	je mmreset
	cmp al,"R"
	je mmreset
	call Reallocmem
	mov ax,4cffh
	int 21h

xmscopy:push ds
#if !_mdltiny
	mov ds,data
#else
	mov ds,cs
#endif
	pushad
	mov si,struc_xms
	mov ah,0bh
	callf [XMSdriver]
	or ax,ax
	je xmserror
	popad
	pop ds
	ret

xmserror:
#if !_mdltiny
	mov ds,data
#else
	mov ds,cs
#endif
	cld
	mov es,ds
	mov si,err_xms
l1:	lodsb
	cmp al,-1
	je >e1
	cmp al,bl
	je >f1
	xor al,al
	mov cx,-1
	mov di,si
	repne scasb
	mov si,di
	jmp l1
f1:	call asciiz
e1:	jmp break

#if lang_english
ERR_noXMS:
	db"No XMS driver (e.g. HIMEM.SYS) found.",10,13
	db"Please install one and try again.",10,13,10
	db"'R'=Reset; [ENTER]=Continue anyway :$"
err_xms:db"XMS-Error ",0
#endif

#if lang_german
ERR_noXMS:
	db"Es ist kein XMS-Treiber (z.B. Himem.Sys) installiert.",10,13
	db"Bitte installieren Sie einen und versuchen Sie es nochmals.",10,13,10
	db"'R'=Reset; [ENTER]=Weiter :$"
err_xms:db"XMS-Fehler ",0
#endif

xmserrorlist:
	db 80h,"Function not implemented",0
	db 81h,"VDISK device detected",0
	db 82h,"A20 error occured",0
	db 8Eh,"General driver error occured",0
	db 8Fh,"Unrecoverable driver error occured",0
	db 90h,"HMA does not exist",0
	db 91h,"HMA already in use",0
	db 92h,"DX is less than the /HMAMIN= parameter",0
	db 93h,"HMA is not allocated",0
	db 94h,"A20 line is still enabled",0
	db 0A0h,"All extended memory is allocated",0
	db 0A1h,"All available extended memory handles are in use",0
	db 0A2h,"Handle is invalid",0
	db 0A3h,"SourceHandle is invalid",0
	db 0A4h,"SourceOffset is invalid",0
	db 0A5h,"DestHandle is invalid",0
	db 0A6h,"DestOffset is invalid",0
	db 0A7h,"Length is invalid",0
	db 0A8h,"Move has an invalid overlap",0
	db 0A9h,"Parity error occured",0
	db 0AAh,"Block is not locked",0
	db 0ABh,"Block is locked",0
	db 0ACh,"Block's lock count overflowed",0
	db 0ADh,"Lock failed",0
	db 0B0h,"Smaller UMB is available",0
	db 0B1h,"No UMBs are available",0
	db 0B2h,"UMB segment number is invalid",0
	db -1

	.data
;[xmscopy]
struc_xms:
rmb_copylen	dd ?
rmb_srchandle	dw ?
rmb_source	dd ?
rmb_desthandle	dw ?
rmb_dest 	dd ?

rmb_size:	dd ?
rmb_freeptr: 	dd ?		 ;Pointer to free memory in XMS
rmb_free:	dd ?
XMShandle:	dw ?

