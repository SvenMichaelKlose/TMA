; Modul zum Starten anderer Programme

	.code
get_comspec:
	push es
	mov es,[2ch]	;Startsegment des Environmentblocks
	xor di,di
m1:	mov si,comspectag
	cmp b[si],0
	je >n1		;Kein "COMSPEC=" gefunden
	mov cx,size_comspectag
	repe cmpsb
	je >f1		;Gefunden, weiter
	mov cx,-1
	xor al,al
	repne scasb	;NÑchsten Eintrag suchen
	jmp m1
	;
f1:	mov cs_seg,es
	mov cs_adr,di
	pop es
	stc
	ret
	;
n1:	xor eax,eax
	mov cs_adr,eax
	pop es
	clc
	ret

;FÅhrt Programm aus
;si=Programmparameter
;cs_adr/cs_seg=Programmname & Pfad
;al=0	Start eines Programms
;al=1	Laden eines Programms
;al=3	Laden eines Overlays
exec:	push ds
	mov es,data
	mov ds,es
	;
	push ax
	mov bx,ParamBlock
	mov [bx+4],cs	; Load the location of the command tail
	mov w[bx+2],buf_run
	xor eax,eax
	mov [bx],ax
	mov [bx+6],eax
	mov [bx+0ah],eax
	;
	pop ax
	mov cs:oldss,ss
	mov cs:oldsp,sp
	mov dx,cs_adr
	mov ds,cs_seg

	mov ah,4bh
	int 21h
	mov ss,cs:oldss
	mov sp,cs:oldsp
	pop ds
	ret

oldss	dw ?
oldsp   dw ?

#if !_mdltiny
_text   segment
#endif
comspectag:db"COMSPEC="
	size_comspectag=$-comspectag

	.data
cs_adr	dw ?		;Zu startendes Programm
cs_seg	dw ?
	align 16
buf_run: db 256 dup ?
paramblock: dw 7 dup ?


