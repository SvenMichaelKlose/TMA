;TMA macro assembler
;File handling
; (c)1996-1997 S.Klose

	.data
rmb_fileaddr:dd ?
rmb_filerest:dd ?
filenameptr dw ?		;Pointer to free space in filename list.
filenames:  db 1k dup ? 	;List of loaded files.
filedims:   dq 64 dup ? 	;Information on cached files.
files_loaded:dw ?		;# of currently loaded files.
outhandle	dw ?

	.code
sizeof_seg_input = 31k

_filedims struc
	i_xmspos dd ?	;Current position in XMS block.
	i_len	 dd ?	;File length.
	i_pgpos  dw ?	;Position within accessible block in lower RAM.
	i_lines_local dw ?
	i_current_filenr      db ?
	ends

; >>> Gets file name
;
; In:	DS:SI	Address of ASCIIZ name
; Out:	DS:DI	ASCIIZ name
GetName:delspace
	push fs,fs
	pop es,ds
	xor cx,cx
l1:	lodsb
	cmp al,21h
	jb >e1
	stosb
	inc cx
	jmp l1
e1:	cmp al,0dh
	if e dec si
	mov b[di],0
	ret

; >>> read source block
;
; In:	ES:DI	Destination segment
;	CX	Block size
readsource:
	push ds,bx,cx,dx
	push cx
	xor ecx,ecx
	pop cx
	mov ds,fs
	xor si,si
l5:	mov eax,[rmb_filerest]
	or eax,eax
	jz >l3			;End of file, check if it's included...
	cmp eax,ecx	     ;Block <=32k
	if a mov eax,ecx;maxbuf
	mov w[inputsize],ax
	add w[inputsize],di
	sub [rmb_filerest],eax
	mov edx,[rmb_fileaddr]
	mov [rmb_source],edx
	add [rmb_fileaddr],eax
	test al,11xb
	jz >l2
	and al,11111100xb
	add ax,4
l2:	mov [rmb_copylen],eax
	xor ax,ax		;Copy file block from XMS.
	mov w[rmb_desthandle],ax
;	 mov w[inputptr],di
	mov w[rmb_dest],di;ax
	mov ax,es
	mov w[rmb_dest+2],ax
	mov ax,w[xmshandle]
	mov w[rmb_srchandle],ax
	push di
	call xmscopy
	pop di
	mov ax,w[inputsize]
	sub ax,di		;Azahl der wirklich geladenen Bytes
	pop dx,cx,bx,ds
	ret
	;
	;End of included file.
l3:	cmp w[includes],0	;Are there some more files?
	je >l4			;No, return 0.
	;
	mov bx,w[stc_includes-2];Switch back to parent file
	sub bx,16
	mov eax,[bx+i_xmspos]	;Position in XMS.
	mov [rmb_fileaddr],eax
	mov eax,[bx+i_len]	;Rest of file.
	mov [rmb_filerest],eax
	mov ax,w[bx+i_pgpos]	;Return file position.
	mov w[inputptr],ax
	mov ax,i_lines_local[bx]
	mov [lines_local],ax
	mov al,i_current_filenr[bx]
	mov [current_filenr],al
	mov si,ax
	mov w[stc_includes-2],bx
	dec w[includes]
	jmp l5

l4:	xor ax,ax
	mov w[inputsize],ax
	pop dx,cx,bx,ds
	ret

; =================
; >>> Save file <<<
; =================
;
; In:	DS:DX	ASCIIZ file name.
;	ES:DI	Source segment
;	ECX	Length of block.
; Returns nothing.
savefile:
	call savefile_open
	call savefile_write
	call savefile_close
	ret

; >>> Creates a new file
;
; In:	DS:DX	ASCIIZ file name
; Out:	Cc	BX = Handle
;	Cs	AX = DOS error code
SaveFile_open:
	push cx
	mov ah,3ch	;Create file
	xor cx,cx
	int 21h
	mov bx,ax
	mov [fs:outhandle],bx
	pop cx
	ret

; >>> Store a block
;
; In:	ES:DI	Block addr
;	BX	handle
;	ECX	Size
; Out:	Cs    Error
savefile_write:
SaveFile3:
	push cx
	cmp ecx,8000h	;More than 32k available?
	jb SaveFile2
	mov cx,8000h	;Yes, save as 32k blocks.
SaveFile2:
	mov ah,40h	;Write block.
	push ds,es
	pop ds
	mov dx,di
	push bx
	mov bx,[fs:outhandle]
	int 21h
	pop bx
	pop ds,cx
	mov ax,es
	add ax,800h
	mov es,ax
	sub ecx,8000h
	jnc SaveFile3	;Continue if there's some more...
	clc
	ret

; >>> Close an output file
;
; In:	BX	Handle
; Out:	Cc	-
;	Cs	DOS error code in AX
savefile_close:
	mov bx,[fs:outhandle]
	mov ah,3eh	;Close file.
	int 21h
	ret

; >>> Read file to XMS
;
; In:	DS:DX	ASCIIZ file name
;
; Breaks on error.
open_file:
	mov ax,fs
	mov ds,ax
	mov es,ax
	mov w[inputptr],0	;Pass 2-x
	mov si,dx		;Check, if file is already in XMS.
l2:	mov di,offset filenames
	call getstring2
	jnc >l1
	mov b[current_filenr],al	;Save file number for error messages.
	mov bx,ax		;Initialise file info.
	shl bx,4
	mov eax,[bx+filedims+i_xmspos]
	mov [rmb_fileaddr],eax
	mov eax,[bx+filedims+i_len]
	mov [rmb_filerest],eax
	ret
	;
l1:	push si
	mov si,dx		;Store filename to list.
	mov di,w[filenameptr]
i1:	lodsb
	stosb
	or al,al
	jnz i1
	mov w[filenameptr],di
	mov al,_end		;Mark end of list.
	stosb
	
	mov bx,w[files_loaded]	;Update file info.
	shl bx,4
	mov eax,[rmb_freeptr]
	mov [bx+filedims+i_xmspos],eax
	xor eax,eax
	mov [bx+filedims+i_len],eax
	
	mov ax,3d00h		;Open source file.
	int 21h
	jnc >s1
o1:	pusha			;DOS error.
	call reallocmem 	;Free memory...
	popa
	jmp doserrorroutine	;...and quit program.
	;
s1:	mov bx,ax
o4:	push bx,cx,dx
	mov ah,3fh	;Read block.
	mov cx,sizeof_seg_input ;As much as possible.
	push ds
	mov fs,ds
	mov ds,w[seg_input]
	xor dx,dx
	mov w[fs:inputptr],dx
	int 21h
	pop ds
	or ax,ax
	jz >l3
	push ax
	xor ecx,ecx
	mov cx,ax
	mov edx,ecx
	test cl,11b
	jz >l1
	and cl,11111100xb
	add cx,4
l1:	mov [rmb_copylen],ecx   ;Copy block to XMS
	mov eax,[rmb_freeptr]
	add [rmb_freeptr],ecx
	mov [rmb_dest],eax
	xor ax,ax
	mov w[rmb_source],ax
	mov w[rmb_srchandle],ax
	mov ax,w[xmshandle]
	mov w[rmb_desthandle],ax
	mov ax,w[seg_input]
	mov w[rmb_source+2],ax
	call xmscopy
	mov bx,w[files_loaded]	;Update file info.
	shl bx,4
	add [bx+filedims+i_len],edx
	pop ax
l3:	pop dx,cx,bx
	or ax,ax
	jnz >o5
	mov ah,3eh
	int 21h 		;Reload file.
	pop si
	inc w[files_loaded]
	jmp l2			;Get file from XMS.
o5:	jmp o4

; >>> Write buffer to file if it's already large enough.
wnb:	cmp di,shiftbuf
	jl >n1
	push cx
	mov cx,di
	xor di,di
	push es
        call savefile_write
	pop es
	pop cx
n1:	ret

; >>> Load next source block
lnb:	cmp b[fs:flag_macro],1
	je >l1		;Don't check buffer range,source is a macro...
	cmp w[fs:inputsize],maxbuf
	je >n2
	cmp si,w[fs:inputsize]
	jl >l1
	push es,di
	mov es,w[fs:seg_input]
	xor di,di
	mov cx,maxbuf
	call readsource ;Load next source block.
	jmp >n1
n2:	cmp si,shiftbuf
	jl >l1
	push es,di
	mov es,w[fs:seg_input]
	xor di,di
	mov ds,es
	mov cx,w[fs:inputsize]
	mov ax,si
	and ax,3	;New position in input buffer
	and si,-4	;Position that goes to the segment start
	sub cx,si	;Size of block...
	rep movsb

	mov si,ax
	push ax
	mov cx,maxbuf	;Try to fill as much as possible.
	sub cx,di
	call readsource ;Load next source block.
	pop w[fs:inputptr]
n1:	pop di,es
	mov ax,[fs:inputptr]
	cmp ax,[fs:inputsize]
	jae >l2
	mov si,ax
l1:	clc
	ret
l2:	stc
	ret


