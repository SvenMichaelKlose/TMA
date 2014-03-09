;Structures for programming an x86 CPU

#if CPU386

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Descriptor
desc namespace\
     struc
	limit	dw ?
	baseL	dw ?

	baseH	dd ?
	big	= bit 22
	free	= bit 21
	gran	= bit 23
	present = bit 15
	app	= bit 12

	limitsh = 16
	DPLsh	= 12
	typsh	= 8
	
        gatedesc namespace public\
		 struc union
		offsetL 	dw ?
		segmnt		dw ?
		dwordcnt	db ?
				db 0
		offsetH 	dw ?
	namespace ends

namespace ends

; >>> Descriptor types
enum desctype\
	,\
	avTSS286,\	available 286 TSS
	LDT,\		local descriptor table
	acTSS286,\	active 286 TSS
	callgate286,\	286 call gate
	taskgate,\	286/386 task gate
	intgate286,\	286 interrupt gate
	trapgate286,\	286 trap gate
	,\		reserved
	avTSS386,\	available 386 TSS
	,\		reserved
	acTSS386,\	active 386 TSS
	callgate386,\	386 call gate
	,\		reserved
	intgate386,\	386 interrupt gate
	trapgate386	386 trap gate

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> EFLAGS
eflag namespace
	nested_task	= bit 14
	trap		= bit 8
	IOPLshift	= 12
	#if CPU gt 386
		align	= bit 18
namespace ends

; Control register 0
cr0flags namespace
	pe	= bit 0 	;protection enable
	mp	= bit 1 	;math cpu present
	me	= bit 2 	;math cpu emulation on
	ts	= bit 3 	;task switched
	pg	= bit 31	;paging enable
	#if CPU gt 386
		cd	= bit 30	;cache disable
		pcd	= bit 30	; also called page cache disable
		nw	= bit 29	;no write-through
		am	= bit 17	;alignment mask, disables
					; alignment exceptions
		wp	= bit 16	;also protect read-only pages
					;for the superuser
	#endif
namespace ends

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> 386 task gate
tss386	struc
	last		dw ?	;Pointer to last TSS
			dw 0

	cplstacks namespace public
		cpl0stack	dq ?
		cpl1stack	dq ?
		cpl2stack	dq ?
		cpl3stack label word dq ?
		cplstack struc
			pointer dd ?
			segmnt	dw ?
				dw 0
			ends
	namespace ends

        r namespace public\
	  dword label struc
		eip
		eflags
		eax
		ecx
		edx
		ebx
		esp
		ebp
		esi
		edi
	namespace ends

	;This namespace includes word labels only
        s namespace public\
	  word label struc
		es
		cs
		ss
		ds
		fs
		gs
	namespace ends

			dw 0

	iomapbase	dd ?
namespace ends

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

; >>> Page table entry
pte namespace\
    struc
		dd ?
	p	= bit 1 ;Page is present
	w	= bit 2 ;Page is writeable
	ur	= bit 3 ;Page is readable for CPL 3
	a	= bit 5 ;Page was accessed
	d	= bit 6 ;Page is modified
	AVLsh	= 8	;3 free bits for the OS
	framesh = 12	;Frame number shift
namespace ends

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
#endif

