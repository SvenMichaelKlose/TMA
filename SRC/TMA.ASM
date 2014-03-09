;TMA macro assembler
;Main file
; (c)1996-1998 Sven Klose
		.code
TMASEQ=64
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; Options:

_edit		= 1		;Include editor in project
lang_english	= 1		;Speak English
;lang_german	 = 1

;_view		 = 1		 ;Output lines while assembling
;_screen	 = 1		 ;Buffer code in VGA memory 
;_labalign	 = 1		 ;Align records in label database
				;Doesn't seem to speed anything up.
;opt_movr0	 = 1		 ;Convert MOV r,0 to XOR r,r
_mdltiny	= 1		;Assemble project for tiny memory model
;_quiet_mode	  = 1		  ;Don't print anything.

maxbuf	= 32k			;Input/output buffer size (don't change)
shiftbuf= 31k			;Limit within this buffer when new data
				; is loaded or buffered data is written.
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;Memory allocated for particular buffers:
size_localclr	= 600h		;Size of local label clear buffer
size_seg_local	= 32k
size_seg_global = 48k
size_seg_data	= 48k
size_seg_struc	= 16k
size_seg_macro	= 12k
size_seg_macrodata = 16k
size_seg_internal = 8k
size_seg_reloc	= 4k
size_seg_strucsizes = 1k
size_seg_eaoverrides = 1k
size_seg_namespace   = 4k
; You cannot change the sizes of the input and code buffer.
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#if !_mdltiny
	.model small		;Use small memory model if not tiny
	.stack 4k
dataseg group _text,data
#endif

enum	_com,_exe

	.code
main:
#if !_mdltiny
	mov ax,data		;We'll have to initialize this in any memory
	mov ds,ax		;model...
#else
	mov ax,ds		;... except tiny.
#endif
	mov sr_data,ax
	mov es,ax
	mov ax,cs
	mov sr_code,ax
	;
	mov di,fdat		;Clear common data area of assembler and editor
	mov cx,fdate-fdat
	xor al,al
	rep stosb
	jmp start

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

	.data

; >>> Segments
sr_code 	dw ?		;Code segment of assembler.
sr_data 	dw ?		;Data  "      "   "
fdat:
ptr_localclr	dw ?		;Position in localclr reference table
segment_references:
asm_segs:
seg_input:	dw ?		;Input buffer.
seg_code:	dw ?		;Code buffer.
seg_data:	dw ?		;Data buffer.
seg_localclr:	dw ?		; and its segment.
seg_macro:	dw ?		;MACRO sources.
seg_struc:	dw ?		;STRUC definitions.
seg_internal:	dw ?		;Internal Variables.

seg_strucsizes: dw ?
seg_eaoverrides:dw ?

seg_global:	dw ?
seg_local:	dw ?
seg_labelmode:	dw ?

seg_reloc	dw ?	;Relocation table

seg_macrodata	dw ?
;seg_tmp:	 dw ?		 ;new.inc debug

;seg_macrobuffer:dw ?
;seg_macroparms: dw ?
;seg_macroparmnames: dw ?
end_asm_segs:

;Editor segments
seg_text	dw ?		;The text we edit.
seg_buffer	dw ?		;Clipboard buffer
end_segment_references:
sourcename:	db 64 dup ?	;Name of the source someone wants to compile
flag_ed:	db ?

	include"global\global.inc"	;Global definitions
	include"global\register.inc"	;Module registering & termination
	include"global\new.inc" 	;Memory handling
	include"dos\platform.inc"
#if _edit
	include"ide\editor.asm" 	;The generic editor
#endif

	.data
fdate:
datastart:

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	include"misc\string.inc"	;String scanner
	include"..\keys\share.mac"	;Fingerprint generation pattern
	include"..\lib\io\doserruk.inc" ;DOS error messages
	include"..\lib\io\doserrmn.inc"
	include"asm\opcodesc.inc"	;Deskriptors of assembly instructions
	include"asm\dirtable.inc"	;List of direktives
	include"asm\errors.inc" 	;Assembler error messages
	include"asm\comfile.inc"	;COM file creation
	include"asm\exefile.inc"	;EXE file creation
	include"asm\objfile.inc"
	include"asm\labels.inc" 	;Label handler
	include"asm\codeout.inc"	;Code output
	include"asm\direct.inc" 	;Procedures executing directives
	include"asm\constant.inc"	;Reads in constants
	include"asm\modrm.inc"		;ModRM-byte creation
	include"asm\generate.inc"	;Code generation
	include"asm\parser.inc" 	;Parsing
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

	.data
timer_start	dd ?		;BIOS value to remember start of assembler.
sizeof_executable dd ?
inputptr:	dd ?	;Parser position in source
inputsize:	dd ?	;Size of loaded block.
		db ?	;true: store relocation adrs for EXE generation
destname:	db 64 dup ?  ;Name of destination file.
destptr:	dd ?	;Position in output buffer.

symbolend:	dw ?		;End of symbol list
buffer16:	db 64 dup ?	;Buffer for sorting string tables

; Lookup-Tables for fast mnemonic scan
st_mnemonics:	dw 26 dup ?
st_segregisters:dw 26 dup ?
st_mainregisters:dw 26 dup ?
st_maindirs:	dw 26 dup ?
st_sizes:	dw 26 dup ?
st_pointers:	dw 26 dup ?
st_constants:	dw 26 dup ?

symbols:	db 4k dup ?	;Symbol lists

current_filenr: 	db ?
output_mode:	db ?

freespace:			;Free space for anything
dataend:
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	.code
; Puts segments in right order after the 1st pass.
;
; Move non-data segment to the beginning
; Put _data flag on segment of class 'DATA'
; Move the stack to the end
;
; No arguments.
; Returns nothing.
arrange_segments:
	mov es,fs
	mov di,tab_groups
	mov si,txt_data 	;Search for class named 'DATA'
	call getstring
	if nc mov al,-1
	mov dl,al
	mov di,arranged_segments
	mov bx,segflags
	mov cx,[segments]
	xor bp,bp
l1:	test b[seg_attr+bx],_data or sa_stack
	jnz >n1
	cmp [seg_class],dl
	je >n1
	mov ax,bx
	stosw
	inc bp
n1:	add bx,seg_size
	loop l1
	mov code_segments,bp

	mov bx,segflags
	mov cx,[segments]
l1:	cmp [seg_class],dl
	je >y1
	test b[seg_attr+bx],_data
	jz >n1
y1:	mov ax,bx
	stosw
n1:	add bx,seg_size
	loop l1

	mov bx,segflags
	mov cx,[segments]
l1:	test b[seg_attr+bx],sa_stack
	jz >n1
	mov ax,bx
	stosw
n1:	add bx,seg_size
	loop l1
	ret

; Computes segment positions relative to the executable's starting address.
;
; No arguments.
; Returns nothing.
get_segment_positions:
	mov cx,[segments]
	mov si,arranged_segments
	xor eax,eax
	mov bp,code_segments
r1:	mov bx,[si]
	add si,2
	
	mov edx,seg_ptr[bx]	  ;Store size of segment
	mov seg_max[bx],edx
	cmp b[actualmodel],_tiny
	je >n1
	cmp seg_pos[bx],eax	;Starting position of the next segment
	je >n1
	mov seg_pos[bx],eax
	mov b[newpass],1	;Segment position has changed, set flag that'll
				; force the main loop to another pass.
				; (old EXEs)
n1:	add eax,edx
	cmp bp,1		;Don't align last segment
	je >f2
	push eax,bx		;Align segment position
	xor ebx,ebx
	mov bx,16
	call align_eax_addr
	pop bx,edx
	sub edx,eax
	sub seg_max[bx],edx	;Correct size of segment
f2:	cmp b[actualmodel],_tiny
	jne >s1
	push eax
	shr eax,4
	mov seg_ptr[bx],ax
	pop eax
	jmp >f1
s1:	xor edx,edx		 ;Reset pointer within segment for next pass
	mov seg_ptr[bx],edx
f1:	cmp bp,1
	jne >h1
	mov exe_out,eax
h1:	dec bp
	loop r1
	mov exe_size,eax
	ret

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; >>> Main part <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	.code	
start:	mov ax,sr_data
	mov ds,ax
	mov es,ax
	mov fs,ax	;FS ist used globally and contains the segment or
			;selector of the global data area.
			;DO NOT MODIFY IT!
	call init_registering
	call platform_init
	call InitMemory

cr_com:
	mov di,datastart ;Clear data area of assembler
	mov cx,dataend-datastart
	xor al,al
	rep stosb

	mov al,_end
	mov [filenames],al	;Clear filename list
	;
	mov al,_tiny
	call init_std_segments	;Clear segmentname list
	;
	; Stacks
	mov w filenameptr,offset filenames ;Stack for INCLUDEd files
	mov w[stc_includes-2],offset stc_includes
	mov w[stc_macros],offset stc_macros+2 ;Stack for macro pointers
	mov w[ptr_macroparms],offset macroparms

	xor ax,ax
	mov w ptr_nextmacro,ax	       ;Macro buffer

	mov ptr_namespace,ax

	mov b[processormode],_ppro	;CPU version we want to compile
					;our source for
	;
	; Keyword table initalisation
	mov di,map_legalchars	;Create identifier checklist needed by
	mov al,-1		; GETLABELLN in STRINGS.INC.
	mov cx,"/"+1
	rep stosb
	mov di,map_legalchars+":"
	mov cx,"@"-1-":"
	rep stosb
	mov di,map_legalchars+"["
	mov cx,"`"-"["
	rep stosb
	mov b[map_legalchars+"_"],0
	mov di,map_legalchars+"{"
	mov cx,256-"{"
	rep stosb
	mov b[map_legalchars+36],0    ; allows dollars in symbols
	call init_hash
	;
	; Command line analysis
	cmp b[flag_ed],1
	je >w1
	mov si,81h		;Get filename from comand line
	delspace
	cmp w[si],"e-"
	jne >d1
	mov b output_mode,_exe
	add si,2
d1:	mov di,SourceName
	call GetName
	jcxz >e6		;Error, no source file found
	mov di,DestName
	call GetName
	or cx,cx
	jne >e5 		;Destination name is given
	;
	; Create name of destination file
w1:	mov si,sourcename
l0:     mov di,destname
l1:	lodsb
	cmp al,"\"	  ;Jump over pathnames
	je l0
	cmp al,"."
	je >l2
	or al,al
	je >l2
	stosb
	jmp l1
l2:	mov al,"."
	stosb
	cmp b[output_mode],_exe
	je >i1
	mov ax,"oc"
	stosw
	mov ax,"m"
	stosw
	jmp >e5
i1:	mov ax,"xe"
	stosw
	xor ah,ah
	stosw
	jmp >e5

e6:	#if _edit
	jmp edit		;No filenames found, start the editor
	#else
	;
	; Display welcome message
	mov si,TXT_standard
	call asciiz
	call drline             ;Draw a line
	mov si,TXT_help 	;Display help and exit
	call asciiz
	call drline
	jmp bye4
	#endif

e5:     mov es,40h              ;Get time
	mov eax,[es:6ch]
	mov timer_start,eax
	mov si,TXT_standard
	call asciiz
	call drline
	mov si,txt_source
	call asciiz
	mov dx,SourceName
	mov si,dx
	call asciiz
	call cret

#if _screen
	mov ax,13h		;VGA mode 13h 320*200*256.
	int 10h
	pusha
	push ds
	mov ds,fs
	mov dx,txt_scrram
	mov ah,9
	int 21h
	xor ax,ax
	int 16h
	pop ds
	popa

	mov ax,0a000h
	mov [seg_code],ax
#else
	new 64k,seg_code
#endif
	mov ax,[seg_code]	;Assembled code
	mov es,ax
	xor di,di
	xor ax,ax
	mov cx,8000h
	rep stosw
	;
	new size_seg_local,seg_local	   ;(will return real segment in ax)
	mov cx,size_seg_local
	call clearvarseg
	;
	new size_seg_global,seg_global ;PUBLIC variables
	mov [seg_labelmode],ax
	mov cx,size_seg_global
	call clearvarseg
	;
	new 32k,seg_input
	new size_localclr,seg_localclr
	;
	new size_seg_data,seg_data
	mov cx,size_seg_data
	call clearvarseg
	;
	new size_seg_struc,seg_struc
	mov cx,size_seg_struc
	call clearvarseg
	;
	new size_seg_macro,seg_macro
	mov cx,size_seg_macro
	call clearvarseg
	;
	new size_seg_internal,seg_internal
	new size_seg_reloc,seg_reloc
	;
;	 new 32k,seg_tmp
	new size_seg_strucsizes,seg_strucsizes
	new size_seg_eaoverrides,seg_eaoverrides
	new size_seg_macrodata,seg_macrodata
;	 new 16k,seg_macrobuffer
;	 new 2k,seg_macroparms
;	 new 2k,seg_macroparmnames
	new size_seg_namespace,seg_namespace

;	 delete [seg_tmp]
;	 call GarbageCollection
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Initialize 1st pass
	mov dx,SourceName
	call open_file		;Open main source file (Load it to XMS).
	call init_symbols	;Initialise keyword tables.
	mov b[pass],1		;This is our first pass.
	push di
	mov es,w[seg_input]
	xor di,di
	mov cx,maxbuf
	call readsource 	;Read the first block.
	pop di
	call showpass		;Display pass number
	call initpass		; and initalise it.
	;
	; 1st Pass
l1:	call parse		;Assemble a line.
#if _view
;	 call showline		 ;Display line.
#endif
	jc l1			;Assemble the next line if there is one
	call j_pcode		;Switch to CODE segment to flush the
				;generation buffer.
	call arrange_segments	;Arrange segment information if the desired
				; memory model needs it.
	call get_segment_positions ;Get segment positions relative to the
				; first byte generated.
	;
	; Pass 2+
l2:	inc b[pass]		;Next pass.
	call showpass		;Display new pass number.
	call initpass
	; Reset pointer within the virtual data segment
	xor eax,eax
	mov d[segflags+seg_size+seg_ptr],eax
	call clearlocals	;Clear references to local labels.
	mov dx,SourceName
	call open_file
	push di
	mov es,w[seg_input]
	xor di,di
	mov cx,maxbuf
	call readsource
	pop di
l1:	call parse		;Assemble once more
#if _view
	call showline
#endif
	jc l1			;...line by line...
	call j_pcode		;Flush code generation buffer
	call get_segment_positions
	;
	cmp w[total_errors],0	;Exit in case an error occured.
	jne bye
	cmp b[newpass],1	;If adresses have changed assemble again
	je l2			;with the new database.

; Output generation statistics
bye:	mov al," "
	mov cx,79
l1:	call cout
	loop l1
	mov al,13
	call cout
	mov ax,w[total_errors]
	or ax,ax
        je >o1
	;
	mov dx,TXT_errors
	call printword
	cmp b[flag_ed],0
	jne >c1
	call error2prj
c1:	mov si,txt_inserted
	call asciiz
	mov al,-1
	jmp bye3
	;
o1:	mov si,txt_destination	;Display name of destination file
	call asciiz
	mov si,destname
	call asciiz
	mov dx,destname
	cmp b[output_mode],_exe
	jne >n1
	mov si,txt_exe
	call asciiz
	call create_exefile
	jmp >n2

n1:	call create_comfile

n2:	call cret
bye2:	call drline
	mov ax,w[includecnt]
	inc ax
	mov dx,TXT_files
	call printword
	xor ax,ax
	mov al,b[pass]
	mov dx,TXT_passes
	call printword
	mov ax,w[lines]  ;Display number of asembled lines
	mov dx,TXT_lines
	call printword
	mov ax,w[used_labels]
	mov dx,txt_labels
	call printword
	mov ax,w[used_labels_c]
	mov dx,txt_codelabels
	call printword
	mov ax,w[used_labels_l]
	mov dx,txt_locallabels
	call printword
	mov ax,w[used_labels_d]
	mov dx,txt_datalabels
	call printword
	mov si,txt_code_size
	call asciiz
	mov dx,txt_bytes
	mov ax,w[sizeof_executable]
	call printword
	mov es,40h
	mov eax,[es:6ch]
	sub eax,timer_start
	mov ebx,1008h
	xor edx,edx
	div ebx
	mov eax,edx
	mov ebx,444h
	xor edx,edx
	div ebx
	push dx
	call printword_2d
	mov al,":"
	call cout
	cmp dl,10
	jb >a1
	mov al,"0"
	call cout
a1:	pop ax
	shl ax,4
	mov bx,123h
	xor dx,dx
	div bx
	push dx
	call printword_2d
	mov al,":"
	call cout
	pop ax
	mov bx,1000/182
	xor dx,dx
	mul bx
	shr ax,4
	call printword_2d
	mov si,txt_time
	call asciiz
	call drline
	cmp b[flg_removerr],0
	je bye3
	call error2prj
	mov si,txt_removed
	call asciiz
	xor al,al
	;
bye3:	cmp b[flag_ed],1
	je >l1
;	 call Reallocmem
	jmp _exit
bye4:	cmp b[flag_ed],1
	je >l1
	xor al,al
	jmp _exit

l1:	mov si,asm_segs
	mov cx,(end_asm_segs-asm_segs)/2
l1:	delete [si]
	add si,2
	loop l1
	call GarbageCollection
	ret

o1:	jmp doserrorroutine

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#if !_mdltiny
_text	segment para
#else
	.code
#endif
TXT_standard:
	db"TMA macro assembler - build A#65-980717 - http://www.devcon.net/~sven",13,10
	db"This is free software; read COPYING for details.",10,13
	db 0
tc_jmp: dw t_jmpshort,t_jmpnear,t_jmpfar
tc_call:dw 0,t_call,t_callfar
tc_j:	dw t_j,t_jn,1
dsc_jump:dw createconstants,dsc_o,dsc_ow,dsc_os,dsc_s,dsc_xrm,dsc_xpm,dsc_xsm

segments_std:	db "code",0,"data",0,"stack",_end ;Namen der Standardsegmente
;classes_langc: db 5,"_text",0,"const",0,"_bss",0,"data",0,"_stack",_end
;groups_langc:	db 1,"dgroup",_end
;tab_groupc:	db 2,3,4,0
classes_std:	db "data",_end
txt_data:	db"data",0
	
;#if lang_english
txt_exe:	db" (old EXEcutable)",0
TXT_pass:	db"Pass ",0
TXT_source:	db"Primary file: ",0
TXT_external:	db"Other sources: ",10,13,0
TXT_include:	db"    ",0
TXT_destination:db"Assembly successful.",10,13
		db"Destination: ",0
TXT_passes:	db" passes. ",0
TXT_lines:	db" lines. ",0
TXT_files:	db" files. ",0
TXT_code_size:	db"Code size: ",0
TXT_errors:	db" errors found.",10,13,0
txt_inserted:	db"Error messages inserted.",13,10,0
txt_removed:	db"Error messages removed.",13,10,0

#if !_edit
TXT_help:	db"Syntax :",10,13,10
		db" TMA [options] <infile> [<outfile>]",10,13,10
		db"Options:",13,10,10
		db" -e  Generate EXE file. (COM output is default)",13,10
		db 0
#endif

txt_codelabels: db" code labels.",10,13,0
txt_locallabels:db" local labels.",10,13,0
txt_datalabels: db" data"
txt_labels:	db" labels.",10,13,0
txt_bytes:	db" bytes.",10,13,0
txt_time:	db" min.",10,13,0
;#endif

#if _screen
txt_scrram:	db 13 dup 10
		db"Using VGA RAM due to low memory.$"
#endif
		db 1,"Ceterum censeo Cannabis legalisandum esse !",1 ;-?

_struc=2

;Generation mode
_data=80h
_code=0

	code ends

