	.code

tmMODNAM = 080h
tmMODEND = 08ah
tmSEGSYM = 096h
tmSEGDEF = 098h
tmGRPDEF = 09ah
tmRELOC  = 09ch
tmLEDAT  = 0a0h
tmPUBLIC = 090h
tmEXTRN  = 08ch

tmopen:
tmclose:
tmword:
tmblock:
	ret
tmasciiz:
	call getlabelln
	mov cl,dl
	mov al,dl
	stosb
	rep movsb
	ret

objCreateSegmentInfo:
	mov al,tmSEGDEF
	call tmopen
	;
	mov cx,[segments]
	mov si,segflags
l1:	mov al,seg_attr[si]
	stosb
	mov ax,seg_max[si]
	call tmword
	mov al,seg_index[si]	;segment/group index
	stosb
	mov al,seg_class[si]
	stosb
	mov al,seg_overlay[si]
	stosb
	add si,seg_size
	loop l1
	call tmclose
	ret

; In:	DS:SI	List of names
;	AL	T module code
objCreateNameList:
	call tmopen
	xor al,al
	stosb
l1:	call tmasciiz
	lodsb
	cmp al,_end
	jne l1
	call tmclose
	ret

; >>> Generate group information from tab_groups
objCreateGroupDef:

; >>> Create an public fixup
; In:	EDI	Pointer on label entry
objCreatePublic:

; >>> Create an external fixup
; In:	EDI	Pointer on label entry
objCreateExtrn:

; >>> Create data record
; In:	seg_code:ESI	Offset in segment
;	CX		Block size
objCreateLEDAT:

; >>> Create fixup record
; In:	seg_fixup:EAX	Pointer on fixupinfo
;	ESI		Position in segment to fix up
;	EBP		Pointer on segment info
objCreateFixup:

; >>> Create fixup list for last data
; In:	seg_code:ESI	Position in Segment%10000h
;	EBP	Pointer on segment info
;	ECX	Size of block to fix up
objCreateFixupList:
	ret

objCreateData:
	mov si,tab_segments
	mov al,tmLEDAT
	call tmopen

	call GetCodeFragment
	mov ds,[seg_code]
	mov edx,[seg_max+bp]
	mov ecx,edx
	shr edx,10
	xor esi,esi
l1:	push esi,edx,ecx
	and ecx,1023
	call objCreateLEDAT	;Write a k.
	pop ecx,edx,esi
	call objCreateFixupList ;Create fixup records for this k.
	add esi,1023
	dec edx
	jnz l1
	ret

create_OBJ_file:
	mov dx,destname
	call savefile_open
	mov al,tmMODNAM 	;Store module name
	call tmopen
	mov si,destname
	call tmasciiz
	call tmclose
	;
	mov si,tab_segments
	call objCreateNameList
	;
	call objCreatePublic
	call objCreateExtrn
;	 mov si,tab_groups
;	 call objCreateNameList
	;
	call objCreateSegmentInfo
	;
	mov al,tmMODEND
	call tmopen
	call tmclose
	ret

