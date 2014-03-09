;TMA macro assembler
;String tables needed by the parser
;';-...' means that the commented directive is not implemented. FIXME
; (c)1996-1998 Sven Klose

#if !_mdltiny
_text	segment para
#else
	.code
#endif
map_maindirs:
	;memory values
	db"db",0,"dw",0,"dd",0,"dq",0,"dt",0
	;miscellaneous directives
	db"title",0	;-
	db"page",0	;-
	db"name",0	;-
	db"org",0	;New position within current segment.
	db"proc",0	;-Procedure start
	db"endp",0	;-Procedure ends.
	db"end",0	;-End of file.
	db"label",0	;-Defines an explicit label type.
	db"enum",0	;Define enumeration.
	db"string",0	;Define string.
	db"array",0	;-
	db"local",0	;-Defines local label.
	db"public",0	;-Defines public label.
	db"extrn",0	;-Make label accessible for other modules.
	db"model",0
	db"segment",0
	db"group",0
	db"assume",0	;-define segment override (incomplete)
	db"struc",0	;defines a structure
	db"ends",0	;ends structure or segment
	db"dosseg",0	;-Segment order (MSDOS convention)

	;conditions
	db"if",0
	db"else",0
	db"ifb",0	;-
	db"ifnb",0	;-
	db"ifdef",0	;if defined
	db"ifndef",0	;if not defined
	db"case",0	;-

	;macro directives
	db"macro",0	;start macro definition
	db"endm/#em",0	;ends macro and rept command
	db"rept",0	;-repeats command block
	db"irp",0	;-
	db"irpc",0	;-
	db"exitm",0	;-exits macro
	db"while",0	;-
	db"wend",0	;-
	db"repeat",0	;-
	db"until",0	;-
	db"purge",0	;-Clear macro

	;include and arrange directives
	db"include",0	;includes a source file
	db"incbin",0	;-includes a binary file
	db"incobj",0	;-includes an object file
	db"append",0	;-appends binary to the assembled EXE file

	;misc
	db"uses",0	;defines segment mode 16/32 bit or used registers
			;"uses 16" or "uses 32"
	db"typedef",0	;-
	db"startup",0	;-
	db"align",0	;align to dword adress
	db"para",0	;jump to next paragraph
	db"even",0	;sets program ptr to next even adress
	db"exit",0	  ;back to DOS

	;Stringbefehle
	db"substr",0	;-Returns substring.
	db"catstr",0	;-Paste strings.
	db"sizestr",0	;-return length of string.
	db"instr",0	;-Find substring and return its position.
	
	db"option",_end ;-

jmp_maindirs:
	dw j_db,j_dw,j_dd,j_dq,j_dt
	dw j_title,j_page,j_name,j_org,j_proc,j_endp,j_end,j_label,j_enum
	_string=($-jmp_maindirs)/2
	dw j_string,j_array,j_local,j_public,j_extrn
	dw j_model,j_segment,j_group,j_assume
	dw j_struc
	_ends=($-jmp_maindirs)/2
	dw j_ends,j_dosseg
	dw j_if,j_else,j_ifb,j_ifnb,j_ifdef,j_ifndef,j_case
	dw j_macro,j_endm,j_rept,j_irp,j_irpc,j_exitm,j_while,j_wend
	dw j_repeat,j_until,j_purge
	dw j_include,j_incbin,j_incobj,j_append
	dw j_uses,j_typedef,j_startup
	dw j_align,j_para,j_even,j_exit
	dw j_substr,j_catstr,j_sizestr,j_instr,j_option

	;Keywords needed by ASSUME
map_assume:
	db"nothing",0
	db"returns",_end

	;Directives starting with a dot.
map_dotdirs:
	;misc
	db"data",0	;Define unnamed data segment of type NEAR.
	db"code",0	;Define unnamed code segment "
	db"stack",0	;Define stack size
	db"data?",0	;-
	db"const",0	;-
	db"type",0	;- ??!?
	db"lib",0	;-set directory for includes
	db"alpha",0	;-
	db"seq",0	;-
	;CPU directives
	db"086",0
	db"186",0
	db"286",0
	db"386",0
	db"486",0
	db"pentium",0
	db"ppro",0
	db"mmx",0	;- no mmx instruction at all...
	db"nec",0	;various NEC Vxx
	db"model",0	;same as 'model'
	;MASM conditional directives
	db"if",0	;-
	db"elseif",0	;-
	db"else",0	;-
	db"repeat",0	;-
	db"until",0	;-
	db"untilcxz",0	;-
	db"while",0	;-
	db"endw",0	;-
	db"break",0	;-
	db"continue",_end ;-

jmp_dotdirs:
	dw j_pdata,j_pcode,j_pstack,j_data2,j_pconst,j_ptype,j_plib
	dw j_alpha,j_seq
	dw j_086,j_186,j_286,j_386,j_486,j_pentium,j_ppro,j_mmx,j_nec
	dw j_model

	;A86 compatible conditional directives starting with a hash-sign
map_gdirs:
	db"if",0
map_gif:db"else",0
	db"elseif",0
	db"endif",_end

gif_elseif=1
gif_end=2

jmp_gdirs:
	dw j_gif,j_gelse,j_gelseif,j_gend

	;Assembler-Options (OPTION-Befehl)
	;Model types
map_models:
	db"tiny",0
	db"small",0
	db"compact",0
	db"large",0
	db"huge",_end

enum _tiny,_small,_compact,_large,_huge

	;-Language types, implement PROC/ENDP first.
map_lang:
	db"c",0
	db"tpascal",_end


	;Data sizes
map_sizes:
	db"byte",0	   ;memory
	_byte=0
	db"word",0
	db"dword",0
	db"qword",0
	db"tbyte",0
	db"sbyte",0
	db"sword",0
	db"sdword",0
	db"fword",0
	db"signed",0
	db"char",_end

map_pointers:
	db"short",0	;force short jump
	db"near",0	;force near jump
	db"far",0	;force far jump
	db"ptr",_end	;abreviation for `pointer'
	_ptr=3

map_constants:
	db"offset/fs",_end

	;SchlÅsselworte fÅr GETCONSTANT
map_operands:
	db"sizeof",0	;-Return size in bytes.
	db"lengthof",0	;-Return # of elements in structure.
	db"low",0
	db"high",_end

	;Keywords needed by GETCONSTANT, starting with '@'
map_operands_at:
	db"cpu",0	;-Returns current destination CPU code
	db"data",0	;-Alias for unnamed data segment.
	db"wordsize",0	;-Returns size of current segment in words.
	db"version",0	;-Returns string of MASM version.
			; "610" would be great.
	db"tmaversion",0 ;-Returns string of TMA version.

map_option:	;-
	db"casemap:",0
	db"dotname",0	;Allow dots starting labels.
	db"nodotname",0
	db"m510",0	;Masm 5.1 mode
	db"nom510",0	;Masm 6.x mode
	db"oldmacro",0	;Masm v5.1 compatible macro handling.
	db"nooldmacro",0
	db"oldstrucs",0
	db"nooldstrucs",0
	db"scoped",0	;Make all labels local within PROCedures.
	db"noscoped",0
	db"epiloque:",0 ;Define procedure epilogue.
	db"lang:",0	;Define calling convention.
	db"prologue:",0 ;Define procedure proloque.
	db"expr16",0
	db"expr32",0
	db"float:",0
	db"ljmp",0
	db"noljmp",0
	db"nokeyword:",0 ;Deactivate keyword.
	db"nosignextend",0
	db"offset:",0
	db"readonly",0
	db"noreadonly",0
	db"@interface",0
	db"proto",0
	db"invoke",0
	db"%noincl",0
	db"masm51",_end

map_casemap:
	db"all",0	      ;Convert into capital letters.
	db"none",0
	db"notprivate",_end ;Only when PUBLIC or EXTRN symbol

map_endm1:db"endm",_end
map_endm2:db"em",_end

map_segment:
	db"byte",0,"word",0,"even",0,"dword",0,"align",0,"para",0,"page",0
	_public=($-map_segment)/2
	db"public",0,"stack",0,"common",0,"memory",0
	_at=($-map_segment)/2
	db"at",_end

sa_byte  = 1
sa_word  = 2
sa_dword = 3
sa_para  = 4
sa_page  = 5
sa_public = 10h
sa_stack  = 20h
sa_common = 40h
sa_at     = 80h ;Kann nicht sein. MEMORY & AT sind verschiedene Dinge...
		;(I'm still confused with this one...)

; Used by SEGMENT in DIRECT.INC.
mask_align	= 7
align_values:	dw 1,2,4,16,256

tab_segattr:
	db sa_byte,sa_word,sa_word,sa_dword,sa_dword,sa_para,sa_page
	db sa_public,sa_stack,sa_common,sa_public,sa_at

map_if: db"carry?",0,"overvflow?",0,"parity?",0,"sign?",0,"zero?",_end

