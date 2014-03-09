;TMA macro assembler
;Error messages
; (c)1996-1998 Sven Klose

	.data
total_errors:	dw ?
errorstack_sp:	dw ?
errorstack_ss:	dw ?
error_list:	db size_errorlist dup ?
flg_removerr:	db ?

	.code

size_errorlist=500

; >>> Error messages and their IDs
em_syntaxerror=1
em_dbloverride=2
em_missingendbracket=3
em_misplacedsymbol=4
em_toomuchregs=5
em_toomuchconstants=6
em_wrongscaling=7
em_noptr=8
em_unexpectedeol=9
em_illegaloperand=10
em_nobyteeareg=11
em_twoeaonly=12
em_modelexpected=13
em_dblgroup=14
em_bytesizedeareg=15
em_easpreg=16
em_nomacroname=17
em_dwea16disp=18
em_eadblreg=19
em_invalid16base=20
em_segmentexpected=21
em_nocommdl=22
;em_jumptoofar16=23
em_notsamesize=24
;em_invopsize=25
;em_missingsize=26
;em_oldercpu=27
em_toomuchops=28
em_wordrequired=29
;em_nodwordscaling=30
em_nodwscaling=31
em_notawordseg=32
em_adrmissing=33
;em_noafstandalone=34
;em_onlyconstant=35
em_constmissing=36
em_dblpublic=37
;wr_alreadylocal=38
em_labnotfound=39
em_operandrequired=40
em_wordordwordrequired=41
em_twoopsrequired=42
em_cr1onlyreadable=43
em_sysrisdword=44
;em_suboverflow=45
em_strnotended=46
em_labnotlocal=47
em_const2big=48
em_dbldef=49
em_labmissing=50
em_alreadydata=51
em_descdamage=52
;em_unexpecteddir=53
em_endsmissing=54
em_nostrucval=55
em_nostruccon=56
em_tmuchstruccon=57
;em_segmentfull=58
em_namemissing=59
em_tmbracket=60
em_bracketop=61
em_stringexpected=62
em_unexpectedendif=63
em_unexpectedelseif=64
em_dblmacro=65
em_endmmissing=66
em_codest=67
em_sysr=68
em_s=69
em_af=70
em_parm2big=71
em_noparm=72
em_shortcall=73
;em_farjcc=74
em_labbuffull=75
em_alreadypublic=76
em_cpu=77
em_noclass=78
em_segment=79
em_ebpbase=80
em_invalid16index=81
em_invalid16single=82
em_undefdkeyword=84
em_full_localclr=85
em_nocsload=86

error_struc struc
	error_nr:	db ?	;Error code
	error_file:	db ?	;File where error was detected.
	error_pos:	db ?	;Position within the input line.
	error_line:	dw ?	;Line #
	error_size:
	ends

sort_errors:
	mov si,error_list
	mov cx,w[total_errors]
	jcxz >n1
	dec cx
	jz >n1
l0:	push cx
l1:	mov di,si
l3:	add di,error_size
	mov ax,w[error_line+si]
	cmp w[error_line+di],ax
	ja >l2
	mov al,error_pos[si]
	mov ah,error_pos[di]
	mov error_pos[si],ah
	mov error_pos[di],al
	mov ax,error_line[si]
	mov bx,error_line[di]
	mov error_line[si],bx
	mov error_line[di],ax
	mov al,error_nr[si]
	mov ah,error_nr[di]
	mov error_nr[si],ah
	mov error_nr[di],al
	mov al,error_file[si]
	mov ah,error_file[di]
	mov error_file[si],ah
	mov error_file[di],al
l2:	loop l3
	pop cx
	add si,error_size
	loop l0
n1:	ret

; >>> Quit TMA
break:	mov ax,fs
	mov ds,ax
	mov es,ax
	mov sp,w[errorstack_sp] ;Abort program.
	pop cx
	push bye3
	mov di,errormsgs
	call show_msg
	call cret
	call drline
	stc
	ret

; >>> Handle found error
;
; In:	AX	error ID
; Doesn't return.
error:
ERRORHANDLER:
	mov cx,si
	sub cx,line
	;
	push cx,ax
	mov bx,w[lines_local]
	mov dh,b[current_filenr]
	mov dl,al
	mov al,cl
	mov cx,w[total_errors]
	cmp cx,size_errorlist/error_size
	je >l4			;Error: Error table full.
	mov di,error_list
	jcxz >l5
l1:	cmp b[di+error_file],dh
	jne >l2
	cmp w[di+error_line],bx
        jne >l2                  ;Error is already saved, continue...
        dec w[total_errors]
	jmp >l5
l2:	add di,error_size
	loop l1
l5:	mov b[di+error_nr],dl
	mov b[di+error_file],dh
	mov b[di+error_pos],al
	mov w[di+error_line],bx
	inc w[total_errors]
	pop ax,cx
	;
	cmp b[flag_ed],1
	je >l4
	push ax,cx,si
	call cret
	mov cx,80
	mov al," "
l1:	call cout
	loop l1
	call showline
	pop si,cx
	push cx
	jcxz >l1
	cmp cx,79
	ja >l1
	mov al," "
l3:	int 29h
	loop l3
l1:	mov al,"^"
	int 29h
	call cret
	pop cx,ax
	mov di,errormsgs
	call show_msg
	call cret
	call drline
	call showpass3	;Always display pass.
	mov al," "
	call cout
	mov al,"-"
	call cout
	mov al," "
	call cout
l4:	;xor ax,ax
	;int 16h
	mov sp,w[errorstack_sp]
	stc
	ret

error2prj:
	call sort_errors
	mov si,filenames
l1:	mov dx,si
	push cx,si
	call error2file
	pop si,cx
l2:	lodsb
	or al,al
	ja l2
	cmp b[si],_end
	jne l1
	ret

; >>> Inserts error messages to a file
; In:	DS:DX	ASCIIZ filename
error2file:
	push ds,es
	push dx
	mov si,dx	;We need the number of the file we want to process
l2:	mov di,offset filenames
	call getstring2
	pop dx
	push ax
	push dx
	call open_file
	pop dx
	call savefile_open
	mov es,fs:[seg_input]
	call readsource
	;
	mov ds,fs:[seg_input]
	mov es,fs:[seg_code]
	pop dx
	mov bx,error_list
	xor bp,bp
	xor di,di
	xor si,si
	call lnb
	mov cx,fs:[total_errors]   ;Seek the first/next error in this file
	jcxz >e1
s1:	fs:cmp error_file[bx],dl
	je >f1
	add bx,error_size
	loop s1
	jmp >e1
	;
f1:	fs:mov b error_file[bx],-1
	push cx
	fs:mov cx,error_line[bx]   ;Jump over CX lines
	sub cx,bp
l1:	push bp,cx
	call lnb
	call isErrorLine
	call copy_line
	call wnb
	pop cx,bp
	inc bp
	loop l1
	push ds,fs,si,di
	mov di,errormsgs
	xor ah,ah
	push es
	mov es,fs
	fs:mov al,error_nr[bx]
	call get_list_entry
	pop es
	mov ds,fs
	mov si,di
	pop di
	push si
	xor cx,cx
	fs:mov cl,error_pos[bx]
	mov al,"-"
	rep stosb
	mov al,"^"
	stosb
	mov ax, 10 by 13
	stosw
	mov si,txt_ierror
	mov cx,txt_ierror2-txt_ierror
	rep movsb
	pop si
l1:	lodsb
	stosb
	or al,al
	jne l1
	dec di
	mov ax, 10 by 13
	stosw
	pop si,fs,ds
	pop cx
	jmp s1
	;
e1:	call wnb
	call isErrorLine
	call copy_line
e2:	call lnb
	jnc e1
	mov cx,di
	xor di,di
	call savefile_write
	call savefile_close
	pop es,ds
	ret

; >>> Checks if a line is an inserted error message
isErrorLine:
	push ax,di,es,si
l1:	lodsb
	cmp al,"-"
	je l1
	cmp al,9	;tab
	je l1
	cmp al,"^"
	jne >n1
	lodsw
	cmp ax,10 by 13
	jne >n1
	mov es,fs
	mov di,txt_ierror
	mov cx,txt_ierror2-txt_ierror
	repe cmpsb
	or cx,cx
	jnz >n1
	call get_eol
	pop ax		;Let si point to the next line
	stc
	jmp >n2
n1:	pop si
	clc
n2:	pop es,di,ax
	ret

#if !_mdltiny
_text	segment para
#else
	.code
#endif

txt_ierror:
	db"#error: "
txt_ierror2:


#if lang_german
errormsgs:
	db"Syntax-Fehler.",0 ;1
	db"Doppelte Segmentoverrides sind nicht mîglich.",0 ;2
	db"Abschliessende Klammer fehlt.",0 ;3
	db"Fremdes Symbol gefunden.",0 ;4
	db"Sie kînnen hier hîchstens 2 Register benutzen.",0 ;5
	db"Sie kînnen hier nur eine Konstante benutzen.",0 ;6
	db"Zur Skalierung sind nur die Werte 2,4 und 8 zulÑssig.",0 ;7
	db"Erwartet PTR-SchlÅsselwort.",0 ;8
	db"Unverwartetes Ende der Zeile.",0 ;9
	db"Illegaler Operand.",0 ;10
	db"Es dÅrfen keine BYTE-Register in effektiven Adressen benutzt werden.",0 ;11
	db"Sie kînnen nur 2 Register innerhalb einer 16-Bit-EA benutzen.",0 ;12
	db"Speichermodell (TINY,SMALL,COMPACT,LARGE oder HUGE) wird erwartet.",0 ;13
	db"Gruppe ist bereits definiert.",0 ;14
	db"Byte-Register kînnen nicht als Speicherzeiger verwendet werden.",0 ;15
	db"Das SP/ESP-Register kann nicht als Speicherzeiger verwendet werden.",0 ;16
	db"Macro benîtigt einen Namen.",0 ;17
	db"Dword-Displacements sind in 16-Bit-EAs nicht mîglich.",0 ;18
	db"Ein Register kann nur einmal innerhalb einer 16bit EA benutzt werden.",0 ;19
	db"UngÅltiges 16 bit Basisregister. Mîglich sind BP und BX.",0 ;20
	db"Segmentname erwartet.",0 ;21
	db"COM-Ausgabe erwartet TINY. Versuchen Sie, eine EXE-Datei zu erstellen.",0 ;22
	db" ",0 ;23
	db"Die Operanden mÅssen dieselbe Grî·e aufweisen.",0 ;24
	db" ",0 ;25
	db" ",0 ;26
	db" ",0 ;27
	db"Es wurden zuviele Operanden angegeben.",0 ;28
	db"Dieser Befehl benîtigt einen Operanden der Grî·e WORD.",0 ;29
	db" ",0 ;30
	db"Skaling ist nur mit 32-Bit-EAs mîglich.",0 ;31
	db"Segmente besitzen immer die Grî·e WORD.",0 ;32
	db"Vermisse Offset nach Segmentangabe.",0 ;33
	db" ",0 ;34
	db" ",0 ;35
	db"Zielkonstante fehlt.",0 ;36
	db"Als 'public' deklarierte Labels dÅrfen nicht doppelt definiert werden.",0 ;37
	db" ",0 ;38
	db"Label ist undefiniert.",0 ;39
	db"Befehl benîtigt einen Operanden.",0 ;40
	db"Dieser Befehl benîtigt einen Operanden der Grî·e WORD/DWORD.",0 ;41
	db"Dieser Befehl benîtigt 2 Operanden.",0 ;42
	db"Das CR1-Register kann nur gelesen werden.",0 ;43
	db"Systemregister besitzen immer die Grî·e DWORD.",0 ;44
	db" ",0 ;45
	db"String ist nicht abgeschlossen.",0 ;46
	db"Richtungszeiger kînnen nur mit lokalen Sprungmarken verwendet werden.",0 ;47
	db"Konstante ist zu gro·.",0 ;48
	db"Es darf nur 1 Label am Anfang einer Zeile stehen.",0 ;49
	db"Befehl benîtigt Label am Anfang der Zeile.",0 ;50
	db"SchlÅsselwort wurde bereits als Variable deklariert.",0 ;51
	db"Interner Fehler: ID beschÑdigt. Bitte benachrichtigen Sie den Sven ;-).",0 ;52
	db" ",0 ;53
	db"Vermisse ENDS-Direktiv zum Abschluss der Struktur.",0 ;54
	db"Vermisse Konstanten fÅr Strukturblock in eckigen Klammern.",0 ;55
	db"Es fehlen weitere Konstanten oder eine abschliessende Klammer.",0 ;56
	db"Anzahl der Konstanten Åberschreitet die Strukturgrîsse.",0 ;57
	db" ",0 ;58
	db"Dieser Befehl benîtigt einen Dateinamen als Parameter.",0 ;59
	db"Zu wenig schliessende Klammern.",0 ;60
	db"Ungeschlossene Klammer.",0 ;61
	db"Es wurde ein STRING erwarted.",0 ;62
	db"Unerwartetes #ENDIF.",0 ;63
	db"Unerwartetes #ELSEIF.",0 ;64
	db"Macro wurde bereits definiert.",0 ;65
	db"Macro wurde nicht mit ENDM oder #EM abgeschlossen.",0 ;66
	db"Eine Konstante kann nicht als Ziel angegeben werden.",0 ;67
	db"Systemregister kînnen nur innerhalb des MOV-Befehls benutzt werden.",0 ;68
	db"Segmentregister kînnen nur innerhalb eines MOV- oder PUSH-Befehls",10,13
	db"benutzt werden.",0 ;69
	db"Absolut-FAR-Adressen kînnen nur bei SprÅngen benutzt werden.",0 ;70
	db"Parameternummer zu gro·.",0 ;71
	db"Dieser Parameter wurde nicht Åbergeben.",0 ;72
	db"CALLs der Grîsse SHORT sind nicht mîglich.",0 ;73
	db" ",0 ;74
	db"Hashtabelle ist voll. Zuwenig Speicher.",0 ;75
	db"Label ist bereits als PUBLIC definiert.",0 ;76
	db" ",0 ;77
	db"Nur Klassenbezeichnung erlaubt.",0 ;78
	db"Illegaler Segment-Operand.",10,13 ;79
	db"<Name> SEGMENT [<Typ>] [<Align>] [<'Klasse'>]",10,13,10
	db" <Typ>    = PUBLIC (standard)/STACK/COMMON/MEMORY/AT",10,13
	db" <Align>  = BYTE/WORD/EVEN/DWORD/ALIGN/PARA/PAGE (standard)",10,13
	db" <Klasse> = Klassenname als String",10,13,0
	db"Sie kînnen EBP nicht als Index benutzen.",0 ;80
	db"UngÅltiges 16-Bit Indexregister. Es sind SI und DI zulÑssig.",0 ;81
	db"Als einzelnes Basisregister sind BP,BX,SI und DI zulÑssig.",0 ;82
	db" ",0 ;83
	db"SchlÅsselwort ist undefiniert.",0 ;84
	db"öberlauf: LOCALCLR",0 ;85
	db"Das CS-Register kann nicht direkt geladen werden.",0 ;86


em_processorver:
	db"Der Befehlsvorrat wurde beschrÑnkt auf die ",0
#endif

#if lang_english
errormsgs:
	db"Syntax error.",0 ;1
	db"*Double segment overrides aren't possible.",0 ;2
	db"Ending bracket missing.",0 ;3
	db"Misplaced built-in symbol detected.",0 ;4
	db"You can only use up to 2 registers here.",0 ;5
	db"You can only use 1 constant here.",0 ;6
	db"Wrong scaling constant used. Values 2,4 and 8 are possible only.",0 ;7
	db"I'm missing the PTR directive. Advice: Use the abbreviations.",0 ;8
	db"Unexpected end of line.",0 ;9
	db"Illegal operand.",0 ;10
	db"You can't use byte sized registers within effective adresses.",0 ;11
	db"You can only use two registers within an 16bit EA.",0 ;12
	db"Memory model (TINY,SMALL,COMPACT,LARGE or HUGE) expected.",0 ;13
	db"Group is already defined.",0 ;14
	db"You can't use sized registers as memory pointers.",0 ;15
	db"You can't use the SP/ESP register as a memory pointer.",0 ;16
	db"Macro needs a name.",0 ;17
	db"No Dword displacement in 16bit EA possible.",0 ;18
	db"You can only use a register once within an 16bit EA.",0 ;19
	db"Invalid 16 bit base register. BP and BX are possible only.",0 ;20
	db"Segment expected.",0 ;21
	db"COM output expects TINY. Try to create an EXE file instead.",0 ;22
	db" ",0 ;23
	db"Operands must have the same size.",0 ;24
	db" ",0 ;25
	db" ",0 ;26
	db" ",0 ;27
	db"Too much operands used.",0 ;28
	db"This instruction requires a word operand.",0 ;29
	db" ",0 ;30
	db"You'll have to use a 32-bit EA for scaling.",0 ;31
	db"Segments are always word size.",0 ;32
	db"Offset missing after segment.",0 ;33
	db" ",0 ;34
	db" ",0 ;35
	db"Destination constant missing.",0 ;36
	db"Public labels may not be defined double. Declare as local if neccessary.",0 ;37
	db" ",0 ;38
	db"Label is undefined.",0 ;39
	db"This instruction requires an operand.",0 ;40
	db"This instruction requires a WORD/DWORD operand.",0 ;41
	db"This instruction requires two operands.",0 ;42
	db"The CR1 register is readable only.",0 ;43
	db"System registers are always dword sized.",0 ;44
	db" ",0 ;45
	db"String is not ended.",0 ;46
	db"Direction pointers may only be used with local labels.",0 ;47
	db"Constant is too big.",0 ;48
	db"There should only be 1 label at the beginning of a line.",0 ;49
	db"Command needs label at the beginning of the line.",0 ;50
	db"Keyword is already declared as a variable.",0 ;51
	db"Internal error: ID damage. Please send a note to sven@devcon.net.",0 ;52
	db" ",0 ;53
	db"Missing ENDS directive to end the STRUCture.",0 ;54
	db"Missing structure constants in angle brackets.",0 ;55
	db"Missing further constants or ending bracket.",0 ;56
	db"Structure is too small for that number of constants.",0 ;57
	db" ",0 ;58
	db"This instruction needs a filename as a parameter.",0 ;59
	db"Too many ending brackets.",0 ;60
	db"Bracket not ended.",0 ;61
	db"STRING expected.",0 ;62
	db"Unexpected #ENDIF.",0 ;63
	db"Unexpected #ELSEIF.",0 ;64
	db"Macro is already defined.",0 ;65
	db"Macro is not ended by ENDM or #EM.",0 ;66
	db"A constant can't be used as a destination.",0 ;67
	db"You can only use a system register within a MOV instruction.",0 ;68
	db"Segment register can only be used within a MOV or PUSH instruction.",0 ;69
	db"Absolute FAR addresses can only be used for jumps.",0 ;70
	db"Parameter constant too big.",0 ;71
	db"This parameter is not defined.",0 ;72
	db"Short CALLs aren't possible.",0 ;73
	db" ",0 ;74
	db"Hash table full. Not enough memory.",0 ;75
	db"Label already defined as PUBLIC.",0 ;76
	db" ",0 ;77
	db"Class name expected.",0 ;78
	db"Illegal SEGMENT operand.",10,13 ;79
	db"<Name> SEGMENT [<type>] [<align>] [<'class'>]",10,13,10
	db" <type>  = PUBLIC (standard)/STACK/COMMON/MEMORY/AT",10,13
	db" <align> = BYTE/WORD/EVEN/DWORD/ALIGN/PARA/PAGE (standard)",10,13
	db" <class> = class name in string format",10,13,0
	db"You can't use EBP as an index.",0 ;80
	db"Invalid 16 bit indexregister. SI and DI are possible only.",0 ;81
	db"You can only use BP,BX,SI and DI as a stand-alone base register.",0 ;82
	db" ",0 ;83
	db"Keyword is undefined.",0 ;84
	db"Overflow: LOCALCLR",0 ;85
	db"The CS register can't be loaded immediately.",0 ;86

em_processorver:
	db"The instruction set is limited to the ",0
#endif

processornames:
	db"8088/8086",0
	db"x186",0
	db"x286",0
	db"x386",0
	db"x486",0
	db"Pentium",0
	db"Pentium Pro",0
	db"NEC V20/v30",0
	db"NEC V25",0
	db"NEC V35",0
cpu:	db" CPU.",10,13,0


