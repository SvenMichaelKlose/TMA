;TMA macro assembler
;Binary tree module
; (c)07/01/1998 Sven Klose

	.data
tNode	struc
	tn_below   dd ?
	tn_above   dd ?
	tn_parent  dd ?
	ends

tTree	struc
	tr_entries	dd ?	;Number of entries that can be stored in
				;this tree.
	tr_segsize	dd ?	;Size of the node segment
	tr_nodesize	dd ?	;Size of a single node
	tr_rr		dd ?	;Round-Robin pointer
	tr_data:
	ends

	.code
; General parameters needed by the tree functions
; EBP	 Root node of the tree
; ESI	 The node to process, allocate a new node with tAllocNode
; EDI	 A node that's not linked with a tree.


; Initalize tree
; In:	DS	Tree segment
;	EAX	Size of the segment
;	EBX	Size of a node
tInit:	push ds
	pushad
	mov tr_segsize,eax
	mov tr_rr,eax
	mov tr_nodesize,ebx
	xor edx,edx
	div ebx
	mov tr_entries,eax
	mov ecx,eax
	mov edi,ofs tr_data
	xor eax,eax
	mov ebx,eax
	dec ebx
l1:	mov tn_parent[edi],ebx
	mov tn_below[edi],eax
	mov tn_above[edi],eax
	add edi,tr_nodesize
	dec ecx
	jnz l1
	popad
	pop ds
	ret
	
; >>> Allocate a node entry
; In:	DS	Node segment
; Out:	CS	No node available/no register content changed
;	CC	Free node found
;		EDI	Address of the free node
tAllocNode:
	push ecx,eax,ebx,edi
	xor eax,eax
	dec eax
	mov edi,tr_rr
	mov ebx,tr_segsize
	sub ebx,tr_nodesize
	mov ecx,tr_entries
l1:	add edi,tr_nodesize
	cmp edi,ebx
	jl >l2
	mov edi,ofs tr_data
l2:	cmp tn_parent[edi],eax
	jz >f1
	dec ecx
	jnz l1
	stc
	pop edi
	jmp >l1
f1:	clc
	mov tr_rr,edi
	inc eax
	mov tn_parent[edi],eax
	pop eax
l1:	pop ebx,eax,ecx
	ret

tFreeNode:
	push eax
	xor eax,eax
	dec eax
	mov d tn_parent[esi],eax
	pop eax
	ret

; >>> Add a node to a tree
; In:	EBP	Root entry
;	EBX	Sort index
;	EDI	Node to add
;	DS	Node segment
; Out:	-
tLinkNode:
	push eax,ebx,ecx,edx,esi
	mov esi,ebp
	mov ebp,[fs:esi]
	or ebp,ebp
	jnz >n1
	mov [fs:esi],edi       ;There's no root for this tree, so we add one.
	xor eax,eax
	mov tn_parent[edi],eax
	jmp >f2
n1:	mov esi,ebp	  ;The first node we want to process
	mov eax,[edi+ebx] ;The value we want to compare
	xor edx,edx

l1:	cmp [esi+ebx],eax ;Choose child node
	jle >a1
	mov dx,offset tn_below
	jmp >n1
a1:	mov dx,offset tn_above

n1:	mov ecx,[esi+edx] ;continue with it or...
	or ecx,ecx
	jz >f1
	mov esi,ecx
	jmp l1
f1:	mov tn_parent[edi],esi
	mov [esi+edx],edi ;...add the new node.
f2:	pop esi,edx,ecx,ebx,eax
	ret

; >>> Seek a node in a tree
; In:	EBP	Root entry
;	EBX	Sort index
;	EAX	Value to seek
;	DS	Node segment
; Out:	ESI	The node we found
;	EDI	The value of this node
tSeekNode:
	push ebx,ecx,esi,ebp
	mov esi,ebp
	xor edi,edi		;Just in case there is no root...
	cmp d[fs:esi],0
	je >f1			;...like this.
	mov ebp,[fs:esi]
	xor edx,edx	;temporary
	mov esi,ebp	;The first node we want to process
	xor ebp,ebp
l1:	mov edi,[esi+ebx]
	cmp edi,eax	;Choose child node
	je >f1
	jl >a1
	mov ebp,esi	;Save this node as the next one could be too small.
	mov dx,offset tn_below
	jmp >n1
a1:	mov dx,offset tn_above

n1:	mov ecx,[esi+edx] ;continue with it or...
	or ecx,ecx
	jz >e1
	mov esi,ecx
	jmp l1
	;
e1:	cmp edi,eax		;This value is too small, so we take the
	ja >f1			;last one that is big enough.
	or ebp,ebp
	jz >f1
	mov esi,ebp
	mov edi,[esi+edx]
	;
f1:	pop ebp,ecx,ecx,ebx ;...stop.
f2:	ret

tClearLinks:
	push eax
	xor eax,eax
	mov tn_above[esi],eax
	mov tn_below[esi],eax
	mov tn_parent[esi],eax
	pop eax
	ret

tLinkParent:
	mov eax,tn_parent[esi]	    ;link new node to the current parent
	or eax,eax
	jnz >k1
	push ebx
	mov ebx,ebp
	mov fs:[ebx],ecx
	pop ebx
	jmp >n2
k1:	cmp tn_below[eax],esi
	jne >n1
	mov tn_below[eax],ecx
	jmp >n2
n1:	mov tn_above[eax],ecx
n2:	ret

; Paste nodes in tree. This function is neccessary for tDeleteNode
;
; In:	ESI	old node
;	EDI	new node
;	EBP	tree root reference
tXchgNode:
	push ecx
	mov ecx,edi
	call tLinkParent
	mov tn_parent[ecx],eax
	call tClearLinks
	pop ecx
	ret

; Deletes a node from the tree
;
; In:	ESI	node
;	EBP	root address reference
;	DS	Node segment
; Out:	-
tUnlinkNode:
	push esi,edi,ebx,eax,ecx
	mov ebx,tn_above[esi]
	or ebx,ebx
	jne >n1
	mov edi,tn_below[esi]
	or edi,edi
	je >e2		;Nothing linked...
	call tXchgNode
	jmp >e3
n1:	mov ebx,tn_below[esi]
	or ebx,ebx
	jne >n2
	mov edi,tn_above[esi]
	call tXchgNode
	jmp >e3
	;
;Old node fully linked...
n2:	xor eax,eax	;seek biggest link of the smallest one
	mov ecx,ebx
l1:	mov eax,tn_above[ebx]
	or eax,eax
	je >l3
	mov ebx,eax
	jmp l1
	;
l3:	call tLinkParent
	mov tn_parent[ecx],eax
	;
	mov eax,tn_above[esi]
	mov tn_above[ebx],eax
	mov tn_parent[eax],ebx
	jmp >e1
	;
e2:	xor ecx,ecx
	call tLinkParent
	;
e1:	call tClearLinks
e3:	pop ecx,eax,ebx,edi,esi
	ret

; Seek first node in the tree.
; In:   DS:ESI  root node pointer(!)
; Out:  DS:EBX  least node
;       ESI destroyed.
tScanFirst:
o1:     mov ebx,esi
	mov esi,tn_below[ebx]
	or esi,esi
	jnz o1                  ;Node found.
	ret

; Seek last node in the tree.
; In:   DS:ESI  root node pointer(!)
; Out:  DS:EBX  least node
;       ESI destroyed.
tScanLast:
o1:     mov ebx,esi
	mov esi,tn_above[ebx]
	or esi,esi
	jnz o1                  ;Node found.
	ret

; >>> Seek a node above the current one.
; In:   DS:EBX  Current node
; Out:  DS:EBX  Next node
;       Cs      No more nodes found.
tNextAbove:
	push esi
	mov esi,tn_above[ebx]
	or esi,esi
	jnz >o1                 ;Bigger node found.
l1:     mov esi,tn_parent[ebx]  ;Get parent node.
	or esi,esi
	je >n1                  ;No more nodes...
	cmp tn_above[esi],ebx   ;Linked with tn_above
	jne >o2
	mov ebx,esi
	jmp l1
	;
o2:     mov ebx,esi
	jmp >o3
	;
o1:     call tScanFirst
	;
o3:     pop esi
	clc
	ret
n1:     pop esi
	stc
	ret

; Seek a node below the current one.
; In:   DS:EBX  Current node
; Out:  DS:EBX  Next node
;       Cs      No more nodes found.
tNextBelow:
	push esi
	mov esi,tn_below[ebx]
	or esi,esi
	jnz >o1                 ;Smaller node found.
l1:     mov esi,tn_parent[ebx]  ;Get parent node.
	or esi,esi
	je >n1                  ;No more nodes...
	cmp tn_below[esi],ebx   ;Linked with tn_below
	jne >o2
	mov ebx,esi
	jmp l1
	;
o2:     mov ebx,esi
	jmp >o3
	;
o1:     call tScanLast
	;
o3:     pop esi
	clc
	ret
n1:     pop esi
	stc
	ret

; >>> Clear Tree.
; In:   EBP     Tree root reference
;       DS      Node segment
; Out:  Cs      Tree is already empty.
;       Cc      Tree is empty.
tClearTree:
	push esi,ebx
	mov ebx,ebp
	mov esi,[fs:ebx]
	or esi,esi
	jz >n1          ;Tree already empty.
	mov d fs:[ebx],0
	call >l1
	clc
n2:     pop ebx,esi
	ret
n1:     stc
	jmp n2
	;
l1:     push ebx
	mov ebx,tn_below[esi]
	or ebx,ebx
	jz >n1
	push esi
	mov esi,ebx
	call l1
	pop esi
n1:     mov ebx,tn_above[esi]
	or ebx,ebx
	jz >n1
	push esi
	mov esi,ebx
	call l1
	pop esi
n1:     call tClearLinks
	xor ebx,ebx
	mov tn_start[esi],ebx
	pop ebx
	ret

