;TMA macro assembler
; Includes all nesseccary modules for DOS platforms

	include"dos\screenio.inc" ;Screen I/O(text)
	include"dos\dosmem.inc"   ;DOS/XMS memory allocation
	include"dos\xms.inc"	  ;Real mode memory block transfer via XMS
	include"dos\fileio.inc"   ;Filehandling and XMS cache

	.code
platform_init:
	mov eax,platform_exit	;Last destruction will return to DOS
	call register_module

	; Initialize memory handler
	xor ax,ax		;Allocate memory
	call allocmem
	xor eax,eax
	xor ebx,ebx
	mov ax,[dossegstart]
	mov seg_memtree,ax
	shl eax,4
	add eax,treemem
	mov memstart,eax
	mov bx,[doslen]
	shl ebx,4
	sub ebx,treemem
	mov memsize,ebx
	ret

platform_exit:
	mov ah,4ch
	int 21h


