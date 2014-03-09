;TMA macro assembler
;(c)1998 S.Klose
;Memory functions

; Memory handler
;
; This memory handler is based on binary trees. I didn't choose simplicity
; to prevent slowing down the system and to make this module reusable.
; (I hope...)

	include "global\btree.inc"

	.data
memoryNode struc
	dd ?,?,?
	tn_start   dd ?
	tn_size    dd ?
sizeof_memoryNode:
	ends

; The first tree contains information about free space sorted by the gap's
; sizes, as we want to allocate blocks whose size is close to the one we want.
; The second one contains information about the allocated blocks sorted by
; address. Latter allows us to deallocate a block very fast and to perform a
; nice garbage collection algorithm.
;
; Both trees share the same segment for storing their nodes. The following
; variables contain the positions of their roots:

; !!! Please take care that you pass a reference, the address, of one of this
; variables to a function instead of the root node address.
tr_allocdBlocks 	dd ?
tr_freeBlocks		dd ?

;Unused nodes will have tn_parent set to -1.
treemem = 2k	;Size of tree node segment

memstart		dd ?
memsize 		dd ?
seg_memtree		dw ?

	.code
; >>> Main memory functions

; >>> Rebuild tree of free blocks
; In:	-
; Out:	-
tRebuildFree:
	push ds
	pushad
	mov ds,[fs:seg_memtree]
	mov ebp,offset tr_freeBlocks
	mov eax,fs:memstart
	mov ebx,fs:tr_AllocdBlocks
	or ebx,ebx
	jz >n1		;Nothing to collect...
	mov esi,ebx
	call tScanFirst
	jmp >l2
l1:	mov esi,ebx
	mov eax,tn_start[esi]
	add eax,tn_size[esi]
	mov ebx,esi
	call tNextAbove
	jc >n1		;No more nodes...
l2:	cmp tn_start[ebx],eax
	je l1
	call tAllocNode
	mov tn_start[edi],eax
	neg eax
	add eax,tn_start[ebx]
	mov tn_size[edi],eax
	push ebx
	mov ebx,offset tn_size
	mov ebp,offset tr_freeBlocks
	call tLinkNode
	pop ebx
	jmp l1
	;
n1:	mov ebx,fs:memstart	;last block => end of memory
	add ebx,fs:memsize
	sub ebx,eax
	call tAllocNode
	mov tn_start[edi],eax
	mov tn_size[edi],ebx
	mov ebx,offset tn_size
	mov ebp,offset tr_freeBlocks
	call tLinkNode
	popad
	pop ds
	ret

mCorrSeg macro segm
	mov dx,segm
	cmp dx,ax
	if e mov segm,bx
	endm

; >>> Correct segment references at segment_references and all segment registers.
; In:	ESI	Former linear address the segment points to
;	EDI	New linear address
; Out:	-
mCorrectSegRefs:
	push eax,bx,cx,edx
	mov cx,(end_segment_references-segment_references)/2
	mov bx,segment_references
	mov eax,esi
	shr eax,4
	mov edx,edi
	shr edx,4
l1:	cmp [fs:bx],ax
	if e mov [fs:bx],dx
        add bx,2
	debug
	loop l1
	;
	mcorrseg ds
	mcorrseg es
	mcorrseg fs
	mcorrseg gs
	pop edx,cx,bx,eax
	ret

; Garbage collection
;
; In:	EAX	Size of gap we need.
; Out:	Cc	Gap may be allocated.
;	Cs	Not enough memory but gaps were collected.
GarbageCollection:
	pushad
	mov ds,fs:[seg_memtree]
	mov ebp,offset tr_FreeBlocks
	call tClearTree
	;
	mov ebp,eax
	mov esi,fs:tr_AllocdBlocks
	or esi,esi
	jz >e1		;Nothing to collect...
	;
	call tScanFirst
	mov esi,ebx
	mov edi,fs:memstart
	jmp >l1
	;
m1:	call tNextAbove
	jc >e1		;No more nodes...

	mov edi,tn_start[esi]
	add edi,tn_size[esi]
l1:	cmp tn_start[ebx],edi
	je >n1

	push esi
	mov esi,tn_start[ebx]
	mov eax,tn_size[ebx]
	mov tn_start[ebx],edi	;New address of the block
	push ds,es
	xor cx,cx
	mov ds,cx
	mov es,cx
	add ebp,esi
	sub ebp,edi
	if c xor ebp,ebp
	shr eax,2
	call mCorrectSegRefs
l1:	mov ecx,3ff0h
	cmp eax,ecx
	if l mov ecx,eax
	sub eax,ecx
	SEGOFSALIGN ds,esi	;!!! no protected mode support
	SEGOFSALIGN es,edi
	rep movsd
	or eax,eax
	jnz l1
	pop es,ds

	pop esi
n1:	mov esi,ebx
	jmp m1

e1:	call tRebuildFree
	or ebp,ebp
	mov ds,fs
	jz >o1
	popad
	stc
	ret
o1:	popad
	clc
	ret

new	macro siz,selec
	mov eax,siz
	call _new
	shr eax,4
	mov [selec],ax
	endm

; >>> Allocate a memory block
; In:	EAX	Block size
; Out:	Cs	No block of this size found
;	Cc	Block found
;		EAX = linear address
_new:	push ebx
	mov ebx,eax
	call _new2
	jnc >n1
	call GarbageCollection
	mov eax,ebx
	call _new2
n1:	pop ebx
	ret

_new2:	push ds,es,ebx,ecx,ebp,esi,edi
	mov bx,[fs:seg_memtree]
	mov es,bx
	mov ds,bx
	mov ebp,offset tr_freeBlocks
	mov ebx,offset tn_size
	call tSeekNode
	xor ecx,ecx
	cmp edi,eax
	jl >n1		;Smaller block available, give up
	push d tn_start[esi]
	jne >e1 		;Delete the found block
	call tUnlinkNode
	call tFreeNode
	jmp >e2
	;
e1:	add tn_start[esi],eax ;Correct the free block entry and add it again.
	sub tn_size[esi],eax
	call tUnlinkNode
	mov edi,esi
	mov ebp,offset tr_freeBlocks	;Add free block sorted by size.
	mov ebx,offset tn_size
	call tLinkNode
	;
e2:	call tAllocNode
	mov tn_size[edi],eax
	pop d tn_start[edi]

	mov ebp,offset tr_allocdBlocks	;Add allocated block...
	mov ebx,offset tn_start 	;...sorted by address.
	call tLinkNode
	mov eax,tn_start[edi]
	clc
n2:	pop edi,esi,ebp,ecx,ebx,es,ds
	ret
	;
n1:	xor eax,eax
	stc
	jmp n2

delete	macro addr
	xor eax,eax
	mov ax,addr
	shl eax,4
	call _delete
	endm

; >>> Free a memory block
; In:	EAX	Block's address
; Out:	Cs	No block of this address
;	Cc	Block found
;		EAX = linear address
_delete:push ds,esi,ebp,ebx,edx
	mov ds,fs:[seg_memtree]
	mov ebp,offset tr_allocdBlocks
	mov ebx,offset tn_start
	call tSeekNode
	cmp edi,eax
	jne >e1 	;Block not found
	call tUnlinkNode	;Remove this block from the alloc tree.
	;
	mov edi,esi	;Add this node to the free tree
	mov ebp,offset tr_freeBlocks
	mov ebx,offset tn_size
	call tLinkNode
	clc
l1:	pop edx,ebx,ebp,esi,ds
	ret
e1:	stc
	jmp l1

InitMemory:
	push ds,es
	pushad
	mov ds,fs:[seg_memtree]
	mov eax,treemem
	mov ebx,sizeof_memoryNode
	call tInit
	; Store our free memory
	call tAllocNode
	mov fs:tr_freeBlocks,edi
	mov eax,fs:memstart
	mov tn_start[edi],eax
	mov eax,fs:memsize
	mov tn_size[edi],eax
	; 
	xor edi,edi
	mov d fs:tr_allocdBlocks,edi
	popad
	pop es,ds
	ret

