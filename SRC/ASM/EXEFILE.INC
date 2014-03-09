;TMA macro assembler
;EXE file creation
; (c)1997 Sven Klose

; The relocation table is created in CONSTANT.INC. Instead of segment addresses
; offsets are saved which will be copied to the EXE header here.
; In case the destination segment is changed, created code will be buffered
; in XMS.

	.data
reloc_items	dw ?	;# of relocation indexes.
exe_size	dd ?
exe_out 	dd ?
execorr 	dw ?	;Start of first segments.

exe_header struc
	exe_sig 	dw ?	;dw 5a4dh ;"MZ"
	exe_modpages	dw ?	;modulo 512 of pages
	exe_pages	dw ?	;# of 512 byte pages.
	exe_relocitems	dw ?	;# of relocation indexes.
	exe_headersize	dw ?	;Size of header in paragraphs.
	exe_minmem	dw ?	;Minimum memory needed in paragraphs.
	exe_maxmem	dw ?
	exe_stackpos	dw ?	;Relative stack position from file start.
	exe_spinit	dw ?	;intial SP value.
	exe_chksum	dw ?	;Negative sum of all bytes in EXE.
	exe_ipinit	dw ?
	exe_codepos	dw ?
	exe_relocstart	dw ?	;Byte offset of first index.
	exe_overlay	dw ?	;# if overlay
	ends

        .code

getCodeFragment:
	mov ax,seg_xmshandle[bp] ;Get segment from XMS...
	mov [rmb_srchandle],ax
	mov [rmb_dest+2],es
	xor eax,eax
	mov [rmb_desthandle],ax
	mov [rmb_dest],ax
	mov [rmb_source],eax
	mov d[rmb_copylen],64k
	call xmscopy
	ret

; Create EXE file of generated code.
; No arguments.
; Returns nothing.
create_exefile:
	; Generate segment positions and take care of aligments.
	push dx
	xor eax,eax
	mov d [sizeof_executable],eax
	mov w execorr,ax
	mov es,[seg_input]	;Use input buffer as free space.
	mov eax,exe_out
	add eax,20h+200h
	xor ebx,ebx
	mov bx,reloc_items
	shl bx,2
	add eax,ebx
	push eax
	;
	;Create header.
	xor di,di
	mov ax,"ZM"		;EXE-Kennung
	stosw
	;
	pop eax 		;# of pages.
	push ax
	and ax,511
	stosw
	pop ax
	shr eax,9
	stosw

	mov ax,reloc_items
	stosw
	;Headersize=relocitems/4
	mov cx,ax
	shl cx,2
	add cx,20h
	mov ax,cx
	shr ax,4
	test cl,15
	if nz inc ax
	stosw
	;
;	 xor ax,ax		 ;minmem
	mov eax,[segflags+seg_size+seg_ptr]
	shr eax,4
	mov bx,[stack_size]
	shr bx,4
	add ax,bx
	stosw
	mov ax,-1
	stosw			;maxmem
;	 xor ax,ax
;mov ax,0fff0h
	mov eax,exe_size
	shr eax,4
	sub ax,execorr
	stosw			;relative stack position
	mov ax,[stack_size]
	stosw			;Initial SP
	xor ax,ax
	stosw			;Checksum
	mov ax,100h
	stosw			;Program entry point.
;mov ax,0fff0h
	xor ax,ax
	sub ax,execorr
	stosw			;relative CS position
	mov ax,20h
	stosw			;Offset of first index.
	xor ax,ax
	stosw			;# of overlay.
	
	mov eax,"!AMT"
	;xor eax,edi
	stosd

	mov cx,reloc_items
	jcxz >y2
	push ds
	mov ds,[seg_reloc]
	xor si,si
y1:	lodsd
	ror eax,16
	sub ax,fs:execorr
	ror eax,16
	stosd
	loop y1
	pop ds
y2:
	pop dx
	mov ax,reloc_items	;Align segments on paragraphs-
	shl ax,2
	add ax,20h		;Minimum header size.
	mov cx,16
	and al,15
	jz >h1
	sub cl,al
	add es:exe_modpages,cx	;Correct # of pages.
	test w es:exe_modpages,512
	jz >k1
	and w es:exe_modpages,511
	inc w es:exe_pages
k1:	shr cx,2
	mov eax,TMASEQ by "A" by "M" by "T"
	rep stosd
h1:	push di
	call savefile_open
	;
	xor ecx,ecx
	pop cx
	mov d[sizeof_executable],ecx
	xor di,di
	call savefile_write
	mov cx,[segments]
	mov bp,segflags
	mov si,arranged_segments
l1:	mov bp,[si]
	add si,2
	mov al,b[ds:bp+seg_attr] ;Data and stack segments are not written!
	test al,_data or sa_stack
	jnz >o1
        push cx,si
        call getCodeFragment
	xor ecx,ecx
	mov cx,seg_max[bp]
	xor di,di		;...and write it to the destination file.
	cmp bp,segflags
	jne >a1
	mov di,execorr
	shl di,4
a1:	sub cx,di
	add d[sizeof_executable],ecx
	call savefile_write
	pop si,cx
o1:	loop l1 	;Next segment...
	call savefile_close
	ret

