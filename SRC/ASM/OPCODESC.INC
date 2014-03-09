;TMA macro assembler
;Code generation descriptors
; (c)1996-1998 Sven Klose

#if !_mdltiny
_text   segment para
#else
	.code
#endif

; -----------------------------------------------------------------------------

; =========================
; >>> Description codes <<<
; =========================

;t_<mnemonic>:
;        db <transfer size|# of operands|allowed operands>
;       [db <operand codes>]
;       [db <processor version>]
;        db <type of opcode description>
;        db <opcode descriptor>
;        db 0
;or      db _end ;if there're no more info about this mnemonic

; Operation size codes
enum    _b,\    ;Operation may be of size BYTE,WORD or DWORD sein. The opcode's
	   \    ; w bit indicates if it's a BYTE or WORD operation.
	   \    ; For DWORD operations, the w bit must be true (WORD operation)
	   \    ; and the RAND: prefix must be generated.
	_w,\    ;Operation size may be WORD or DWORD.
	_d,\    ;Operation may only be DWORD.
	wb,\    ;1. operand WORD, 2. operand BYTE
	_dw,\   ;1. operand DWORD, 2. operand WORD
	bw,\    ;1. operand BYTE, 2. operand WORD
	nb,\    ;Only BYTE size.
	_ow,\   ;Only WORD size.
	af      ;Only absolute far 32/48 bit address

_jb=1           ;Operand is a jump - destination +/-128
_jw=2           ;Operand is a jump - destination +/-32678

; 0-2   = Operation size code
; 3-4   = # of operands needed.

; # of operands
ol0=0
ol1=10o
ol2=20o
ol3=30o

olbits=30o              ;Masks for # of operands.
nolbits=347o
transbits=7             ;Masks for operation size codes.
ntransbits=370o

; If ol1, use this operand types:
soj= 00000000b  ;jump
soe= 00100000b  ;EA
sor= 01000000b  ;reg
soc= 01100000b  ;const
so3= 10000000b  ;_3
sos= 10100000b  ;sreg
som= 11000000b  ;EA-Memory-Pointer
soo= 11100000b  ;There is information using the operand size code bits.
soo_fs=340o        ;fs-register
soo_gs=341o        ;gs-register
soo_af=342o        ;absolute adress
soo_cc=343o        ;signed BYTE
opbits=11100000b

; If ol2, use those instead:
;    00000000b  ;direct descriptor
		;ar,co
ero= 00100000b  ;fixed EA,reg order
reo= 01000000b  ;fixed reg,EA order
der= 01100000b  ;EA,reg or reg,EA defined by a d-bit
eco= 10000000b  ;EA,constant; size defined by w-bit
ecco=10100000b  ;EA,cc
rco= 11000000b  ;reg,constant; size defined by w-bit
eao= 11100000b  ;EA,? ; second operand described by old desciptors
		 ;?=_1,_cl,co

; Operand type codes
enum    r,\     ;General register
	e,\     ;Effektive adress
	co,\    ;Constant
	_sr,\   ;Segment register
	sysr,\  ;System register
	_dr,\   ;Debug register
	_tr,\   ;Task register
	_cr,\   ;Control register
	_cr4,\  ;
	_af,,\  ;absolute 32/48 bit far address
	ar,\    ;AX/AL register, indicated by operation size
	_1,\    ;Constant value 1
	_cl,\   ;CL register
	_dx     ;DX register

; If ol3, use this:
; Instruction with three operands are always of size WORD.
; The following codes describe the 3rd operand.
enum    cb3,\   ;BYTE constant
	cw3,\   ;WORD constant
	cc3,\   ;SBYTE constant
	cl3     ;CL register

_fs=6
_gs=7

_SS=13          ;ea segment override sign
		;(this is to avoid assembly of useless overrides for
		;16 bit effective addresses

; Opcode descriptors
enum    ,o,\    ;Opcode is a Byte
	ow,\    ;Opcode is a Word
	os,\    ;Opcode is a Byte plus a second byte that is described by an
	  \     ;octal string.
	s,\     ;octal string follows for opcode
	xrm,\   ;ModRM byte
	xpm,\   ;ModRM mit P-Flag (Gruppe von arithmetic instructions)
	xsm,\   ;ModRM mit Segmentregister
	x0m,x1m,x2m,x3m,x4m,x5m,x6m,x7m,\ ;ModRM with constant r digit
	_086,_186,_286,_386,_486,_586,_ppro,_mmx,_nec,nv25,nv33,\
	  \     ;Processor type
	_end    ;End of descriptor

;processor flags:
processorflags=_186
customcpu=_nec

; ======================
; >>> register names <<<
; ====================== 
srofs=16
Segregisters:
	db"es",0,"cs",0,"ss",0,"ds",0,"fs",0,"gs",_end
segr_fs=4
segr_gs=5
overridetab:
	db 046o,056o,066o,076o,144o,145o
Mainregisters:
	db"al",0,"cl",0,"dl",0,"bl",0,"ah",0,"ch",0,"dh",0,"bh",0
	db"ax",0,"cx",0,"dx",0,"bx",0,"sp",0,"bp",0,"si",0,"di",0
	db"eax",0,"ecx",0,"edx",0,"ebx",0,"esp",0,"ebp",0,"esi",0,"edi",_end

;Register numbers:
; 0-7   = al..bh
; 8-15  = ax..di
; 16-23 = eax..edi
; 24-31 = cr0..cr7
; 32-39 = dr0..dr7
; 40-47 = tr0..tr7

; ==============================================
; >>> Conditional codes for branches and SET <<<
; ==============================================
conditionals:
	db"o",0,"no",0,"c/b/nae",0,"nc/ae/nb",0
	db"e/z",0,"ne/nz",0,"be/na",0,"a/nbe",0
	db"s",0,"ns",0,"p/pe",0,"np/po",0,"l/nge",0,"ge/nl",0
	db"le/ng",0,"g/nle",_end

_bp=15o
ea32regs=_bp    ;registers in 32 effective adress
_BX=13o
_SI=16o
_DI=17o
_cc=1
_cw=2

; -----------------------------------------------------------------------------

; =======================================
; >>> Adressing modes with the P flag <<<
; =======================================
Pcommands1:
	db 0    ;p flag starts at 0
	db _b+ol2+eao,ar,co,s,"0p4",0
	db _b+ol2+der,s,"0p0",xrm,0
	db _w+ol2+ecco,o,203o,xpm,0
	db _b+ol2+eco,o,200o,xpm,_end

Pcommands2:
	db 4    ;p flag starts at 4
	db _b+ol1+soe,o,0f6h,xpm,_end
 
Pcommands3:
	db 0    ;p flag starts at 0
	db _b+ol2+eao,-1,_1,o,0d0h,xpm,0
	db _b+ol2+eao,-1,_cl,o,0d2h,xpm,0
	db _b+ol2+ecco,_186,o,0c0h,xpm,_end
	
pcommands4:
	db 4    ;p flag starts at 4
	db _w+ol2+ero,_386,os,0fh,"2p3",xrm,0
	db wb+ol2+eco,_386,ow,0fh,272o,xpm,_end
 
t_not:  db _b+ol1+soe,o,0f6h,x2m,_end
t_neg:  db _b+ol1+soe,o,0f6h,x3m,_end
t_inc:  db _w+ol1+sor,s,"10r",0
	db _b+ol1+soe,o,0feh,x0m,_end
t_dec:  db _w+ol1+sor,s,"11r",0
	db _b+ol1+soe,o,0feh,x1m,_end
t_test: db _b+ol2+ero,o,204o,xrm,0 ;ALPHA: was 204o
	db _b+ol2+reo,o,204o,xrm,0 ;"
	db _b+ol2+eao,ar,co,o,250o,0 
	db _b+ol2+eco,o,0f6h,x0m,_end
; Extended 386 integer multiplication
t_imul: db _b+ol1+soe,o,0f6h,x5m,0      ;imul Eb
	db _b+ol2+eco,_186,o,0c0h,x1m,0
	db _b+ol2+eao,-1,_1,o,0d0h,x1m,0
	db _b+ol2+eao,-1,_cl,o,0d2h,x1m,0
	db _w+ol1+reo,_386,ow,0fh,257o,xrm,0        ;imul Rw, Ew
	db ol3+reo+cc3,_186,o,153o,xrm,0           ;imul Rw, Ew, Dc
	db ol3+reo+cw3,_186,o,151o,xrm,_end        ;imul Rw, Ew, Dw
t_shld: db ol3+ero+cb3,_386,ow,0fh,244o,xrm,0
	db ol3+ero+cl3,_386,ow,0fh,245o,xrm,_end
t_shrd: db ol3+ero+cb3,_386,ow,0fh,254o,0
	db ol3+ero+cl3,_386,ow,0fh,255o,_end
t_daa:  db 0,o,047o,_end
t_das:  db 0,o,057o,_end
t_aaa:  db 0,o,067o,_end
t_aas:  db 0,o,077o,_end
t_movsx:db wb+ol2+reo,_386,ow,0fh,266o,xrm,0
	db _dw+ol2+reo,_386,ow,0fh,277o,xrm,_end
t_movzx:db wb+ol2+reo,_386,ow,0fh,266o,xrm,0
	db _dw+ol2+reo,_386,ow,0fh,267o,xrm,_end
t_cbw:  db 0,o,230o,_end
t_cwde: db 0,ow,66h,230o,_end
t_cwd:  db 0,o,231o,_end
t_cdq:  db 0,ow,66h,231o,_end
t_bswap:db _w+ol1+sor,_486,os,0fh,"31r",_end
t_xlatb:db 0,o,327o,_end
t_bsf:  db _w+ol2+reo,_386,ow,0fh,274o,xrm,_end
t_bsr:  db _w+ol2+reo,_386,ow,0fh,275o,xrm,_end
t_cmpxchg:
	db _b+ol2+ero,_486,ow,0fh,246o,xrm,_end
t_xadd: db _b+ol2+ero,_486,ow,0fh,0c0h,xrm,_end
t_nop:  db 0,o,220o,_end
t_wait: db 0,o,233o,_end
t_lock: db 0,o,360o,_end
t_hlt:  db 0,o,364o,_end
t_into: db 0,o,316o,_end
t_iret: db 0,o,317o,_end
t_popf: db 0,o,235o,_end
t_popfd:db 0,_386,ow,66h,235o,_end
t_pushf:db 0,o,234o,_end
t_pushfd:
	db 0,_386,ow,66h,234o,_end         ;pushf / (#) pushfd
t_sahf: db 0,o,236o,_end
t_lahf: db 0,o,237o,_end
t_cmc:  db 0,o,365o,_end
t_clc:  db 0,o,370o,_end
t_stc:  db 0,o,371o,_end
t_cli:  db 0,o,372o,_end
t_sti:  db 0,o,373o,_end
t_cld:  db 0,o,374o,_end
t_std:  db 0,o,375o,_end
t_pusha:db 0,_186,o,140o,_end
t_pushad:
	db 0,_386,ow,66h,140o,_end
t_popa: db 0,_186,o,141o,_end
t_popad:db 0,_386,ow,66h,141o,_end
t_xchg: db _w+ol2+eao,-1,ar,s,"22m",0 ;ALPHA: was "22r" but generated NOP
	db _w+ol2+eao,ar,-1,s,"22r",0
	db _b+ol2+reo,o,206o,xrm,0
	db _b+ol2+ero,o,206o,xrm,_end
t_mov:  db 0            ;dummy byte
t_lea:  db _w+ol2+reo,o,215o,xrm,_end        ;lea Rw, En  (x != 3)
t_lss:  db _ow+ol2+reo,_386,ow,0fh,262o,xrm,_end ;lss Rw, Ef  (x != 3)
t_lfs:  db _ow+ol2+reo,_386,ow,0fh,264o,xrm,_end ;lfs Rw, Ef  (x != 3)
t_lgs:  db _ow+ol2+reo,_386,ow,0fh,265o,xrm,_end ;lgs Rw, Ef  (x != 3)
t_les:  db _ow+ol2+reo,o,304o,xrm,_end            ;les Rw, Ef  (x != 3)
t_lds:  db _ow+ol2+reo,o,305o,xrm,_end            ;lds Rw, Ef  (x != 3)
t_es:   db 0,o,046o,_end   ;ES:
t_cs:   db 0,o,056o,_end   ;CS:
t_ss:   db 0,o,066o,_end   ;SS:
t_ds:   db 0,o,076o,_end   ;DS:
t_fs:   db 0,_386,o,144o,_end ;FS:
t_gs:   db 0,_386,o,145o,_end ;GS:
t_insb: db 0,_186,o,154o,_end      ;insb
t_insw: db 0,_186,o,155o,_end      ;insw
t_insd: db 0,_386,ow,66h,155o,_end ;insd
t_outsb:db 0,_186,o,156o,_end      ;outsb
t_outsw:db 0,_186,o,157o,_end      ;outsw
t_outsd:db 0,_386,ow,66h,157o,_end ;outsd
t_movsb:db 0,_186,o,244o,_end      ;mosvb
t_movsw:db 0,_186,o,245o,_end      ;movsw
t_movsd:db 0,_386,ow,66h,245o,_end ;movsd
t_cmpsb:db 0,o,246o,_end           ;cmpsb
t_cmpsw:db 0,o,247o,_end           ;cmpsw
t_cmpsd:db 0,_386,ow,66h,247o,_end ;cmpsd
t_stosb:db 0,o,252o,_end           ;stosb
t_stosw:db 0,o,253o,_end           ;stosw
t_stosd:db 0,_386,ow,66h,253o,_end ;stosd
t_lodsb:db 0,o,254o,_end           ;lodsb
t_lodsw:db 0,o,255o,_end           ;lodsw
t_lodsd:db 0,_386,ow,66h,255o,_end ;lodsd
t_scasb:db 0,o,256o,_end           ;scasb
t_scasw:db 0,o,257o,_end           ;scasw
t_scasd:db 0,_386,ow,66h,257o,_end ;scasd
t_prefixstart:
t_rep:  db o,362o,_end           ;repne/repnz/rep
t_repe: db o,363o,_end           ;repe/repz
t_rand: db _386,o,146o,_end ;RAND:
t_addr: db _386,o,147o,_end ;ADDR:
t_prefixend:
t_set:  db nb+ol1+soe,_386,ow,0fh,220o,x0m,_end

	;
t_1constart:          ;Befehle mit m”glicher 1. Konstante
t_jcxz: db ol1,_jb,o,343o,_end
t_jecxz:db ol1,_jb,ow,66h,343o,_end
t_loopnz:
	db ol1,_jb,o,340o,_end
t_loopz:db ol1,_jb,o,341o,_end
t_loop: db ol1,_jb,o,342o,_end

t_j:    db ol1,_jb,o,160o,0
t_jn:   db ol1,_jw,4,_386,ow,17o,200o,_end

t_jmp:  db ol1,_jb,o,353o,0  ;jmp short Cb
	db ol1,_jw,3,o,351o,0  ;jmp near cw
	db _ow+ol1+soe,o,377o,x4m,0 ; jmp near En
t_jmpfar:
	db _d+ol1+soe,o,377o,x5m,0 ; jmp far Ef
	db ol1+soo_af,o,352o,_end       ;jmp far Af
t_jmpshort:
	db ol1,_jb,o,353o,_end       ;jmp short Cb
t_jmpnear:
	db ol1,_jw,3,o,351o,0          ;jmp near cw
	db _ow+ol1+soe,o,377o,x4m,_end ; jmp near En
t_call: db ol1,_jw,3,o,350o,0          ;call cw
	db _ow+ol1+soe,o,377o,x2m,0    ;call en
t_callfar:
	db ol1+soo_af,o,232o,0      ;call Af
	db _d+ol1+soe,o,377o,x3m,_end ;call far ef
t_retf: db 0,o,313o,0     ;retf/ret far
	db _ow+ol1+soc,o,312o,_end ;retf Dw/ret far dw
t_enter:db wb+ol2,co,co,_186,o,310o,_end
t_bound:db _w+ol2+reo,_186,o,142o,xrm,_end     ;bound Rw, Ed
t_aam:  db 0,ow,324o,012o,0
	db nb+ol1+soc,o,324o,_end     ;AAM db
t_aad:  db 0,ow,325o,012o,0
	db nb+ol1+soc,o,325o,_end     ;AAD db
t_int:  db nb+ol1+so3,o,314o,0
	db nb+ol1+soc,o,315o,_end
t_in:   db nb+ol2+eao,ar,co,o,344o,0            ;in AL, Db
	db wb+ol2+eao,ar,co,o,345o,0            ;in AX,co
	db bw+ol2+eao,ar,_dx,o,354o,0           ;in AL,DX
	db _ow+ol2+eao,ar,_dx,o,355o,_end       ;in AX, DX
t_out:  db nb+ol2+eao,co,ar,o,346o,0            ;out Db, AL
	db bw+ol2+eao,co,ar,o,347o,0            ;out Db, AX
	db wb+ol2+eao,_dx,ar,o,356o,0           ;out DX, AL
	db _ow+ol2+eao,_dx,ar,o,357o,_end       ;out DX, AX
t_push: db ol1+soo_cc,o,152o,0
	db _w+ol1+sor,s,"12r",0
	db _w+ol1+soe,o,377o,x6m,0
t_pushc:db _w+ol1+soc,o,150o,0
t_pushs:db ol1+soo_fs,_386,ow,0fh,240o,0
	db ol1+soo_gs,_386,ow,0fh,250o,0
	db _ow+ol1+sos,s,"0s6",0 ;wieder ans Ende packen
	db _end
t_1conend:
t_ret:  db 0,o,303o,0      ;ret
	db _ow+ol1+soc,o,302o,_end ;ret Dw
	;

t_pop:  db _w+ol1+sor,s,"13r",0
	db _w+ol1+soe,o,217o,x0m,0
t_pops: db ol1+soo_fs,_386,ow,0fh,241o,0
	db ol1+soo_gs,_386,ow,0fh,251o,0
	db _ow+ol1+sos,s,"0s7",0
	db _end
t_leave:db 0,_186,o,311o,_end      ;leave
t_arpl: db _w+ol2+ero,_286,o,143o,xrm,_end      ;arpl Es, Rw
t_sldt: db _ow+ol1+soe,_286,ow,0fh,0,x0m,_end   ;sldt Ew
t_str:  db _ow+ol1+soe,_286,ow,0fh,0,x1m,_end   ;str Ew
t_lldt: db _ow+ol1+soe,_286,ow,0fh,0,x2m,_end   ;lldt Ew
t_ltr:  db _ow+ol1+soe,_286,ow,0fh,0,x3m,_end   ;ltr Ew
t_verr: db _ow+ol1+soe,_286,ow,0fh,0,x4m,_end   ;verr Ew
t_verw: db _ow+ol1+soe,_286,ow,0fh,0,x5m,_end   ;verw Ew
t_lar:  db _ow+ol1+reo,_286,ow,0fh,2,xrm,_end   ;lar Rw, Ew
t_lsl:  db _ow+ol1+reo,_286,ow,0fh,3,xrm,_end   ;lsl Rw, Ew
t_sgdt: db _ow+ol1+som,_386,ow,0fh,1,x0m,_end   ;sgdt Ep
t_sidt: db _ow+ol1+som,_386,ow,0fh,1,x1m,_end   ;sidt Ep
t_lgdt: db _ow+ol1+som,_386,ow,0fh,1,x2m,_end   ;lgdt Ep
t_lidt: db _ow+ol1+som,_386,ow,0fh,1,x3m,_end   ;lidt Ep
t_smsw: db _ow+ol1+soe,_386,ow,0fh,1,x4m,_end   ;smsw Ew
t_lmsw: db _ow+ol1+soe,_386,ow,0fh,1,x6m,_end   ;lmsw Ew
t_clts: db 0,_386,ow,0fh,6,_end        ;clts

	;x486 instructions
t_invd: db 0,_486,ow,0fh,010o,_end        ;invd
t_wbinvd:
	db 0,_486,ow,0fh,011o,_end        ;wbinvd
t_invlpd:
	db _ow+ol1+som,_486,ow,0fh,020o,x7m,_end ;invlpg Ea

	;Intel Pentium instructions
t_wrmsr:db 0,_586,ow,0Fh,30h,_end ;WRMSR Write EDXEAX to Model Specific Reg #ECX
t_cmpx8:db _ow+ol1+soe,_586,ow,0Fh,0C7h,x1m ;CMPX8 ew 5 If EDXEAX=mq then mq:=ECXEBX, else EAXEDX:=mq
t_cpuid:db 0,_586,ow,0Fh,0A2h,_end ;CPUID If EAX=1 set EDXEAX to CPU identification values
t_rdmsr:db 0,_586,ow,0Fh,32h,_end ;RDMSR Read Model Specific Reg #ECX to EDXEAX
t_rdtsc:db 0,_586,ow,0Fh,31h,_end ;RDTSC Read Time Stamp Counter to EDXEAX
t_rsm:  db 0,_586,ow,0Fh,0AAh,_end ;RSM Resume from System Management mode
t_cmov: db _w+ol2,reo,_ppro,ow,0fh,40h,xrm,_end

	;Intel Pentium Pro instructions
t_rdpmc:db 0,_ppro,ow,0fh,33h,_end      ;Read performance monitoring counters
t_ud2:  db 0,_ppro,ow,0fh,0bh,_end      ;Undefined instruction exception
t_emmx: db 0,_ppro,ow,0fh,77h,_end      ;Empty MMX state

	; NEC V20/V30 instructions (not supported by Intel)
t_add4s:db 0,_nec,ow,0Fh,20h,_end ;ADD4S Add CL nibbles BCD, DS:SI into ES:DI (CL even,NZ)
t_call80:
	db nb+ol1+soc,_nec,ow,0Fh,0FFh,_end ;CALL80 ib Call 8080-emulation code at INT number ib
t_clrbit:
	db nb+ol2+eao,-1,_cl,_nec,ow,0Fh,12h,x0m,0 ;CLRBIT eb,CL Clear bit CL of eb
	db wb+ol2+eao,-1,_cl,_nec,ow,0Fh,13h,x0m,0 ;CLRBIT ew,CL Clear bit CL of ew
	db nb+ol2+eao,-1,co,_nec,ow,0Fh,1Ah,x0m,0 ;CLRBIT eb,ib Clear bit ib of eb
	db wb+ol2+eao,-1,co,_nec,ow,0Fh,1Bh,x0m,_end ;CLRBIT ew,ib Clear bit ib of ew
t_cmp4s:db 0,_nec,ow,0Fh,26h,_end ;CMP4S Compare CL nibbles BCD, DS:SI - ES:DI (CL even,NZ)
t_lodbits:
	db nb+ol2+ero,_nec,ow,0Fh,33h,xrm,0 ;LODBITS rb,rb Load AX with DS:SI,bit rb (incr. SI,rb), rb+1 bits
	db nb+ol2+rco,_nec,ow,0Fh,3Bh,x0m,_end ;LODBITS rb,ib Load AX with DS:SI,bit rb (incr. SI,rb), ib+1 bits
t_notbit:
	db nb+ol2+eao,-1,_cl,_nec,ow,0Fh,16h,x0m,0 ;NOTBIT eb,CL Complement bit CL of eb
	db wb+ol2+eao,-1,_cl,_nec,ow,0Fh,17h,x0m,0 ;NOTBIT ew,CL Complement bit CL of ew
	db nb+ol2+eao,-1,co,_nec,ow,0Fh,1Eh,x0m,0 ;NOTBIT eb,ib Complement bit ib of eb
	db wb+ol2+eao,-1,co,_nec,ow,0Fh,1Fh,x0m,_end ;NOTBIT ew,ib Complement bit ib of ew
t_repc: db 0,_nec,o,65h,_end                  ;REPC Repeat following CMPS or SCAS CX times or until CF=0
t_repnc:db 0,_nec,o,64h,_end                  ;REPNC Repeat following CMPS or SCAS CX times or until CF=1
t_rol4: db nb+ol1+soe,_nec,ow,0Fh,28h,x0m,_end    ;ROL4 eb Rotate nibbles: Heb=Leb   HAL,Leb=LAL  LAL=Heb
t_ror4: db nb+ol1+soe,_nec,ow,0Fh,2Ah,x0m,_end    ;ROR4 eb N Rotate nibbles: Leb=Heb   Heb=LAL  AL=eb
t_setbit:
	db nb+ol2+eao,-1,_cl,_nec,ow,0Fh,14h,x0m,0   ;SETBIT eb,CL Set bit CL of eb
	db wb+ol2+eao,-1,_cl,_nec,ow,0Fh,15h,x0m,0   ;SETBIT ew,CL  N Set bit CL of ew
	db nb+ol2+eao,-1,co,_nec,ow,0Fh,1Ch,x0m,0    ;SETBIT eb,ib  N Set bit ib of eb
	db wb+ol2+eao,-1,co,_nec,ow,0Fh,1Dh,x0m,_end ;SETBIT ew,ib  N Set bit ib of ew
t_stobits:
	db nb+ol2+ero,_nec,ow,0Fh,31h,xrm,0    ;STOBITS rb,rb Store AX to ES:DI,bit rb (incr. DI,rb), rb+1 bits
	db nb+ol2+rco,_nec,ow,0Fh,39h,x0m,_end ;STOBITS rb,ib N Store AX to ES:DI,bit rb (incr. DI,rb), ib+1 bits
t_sub4s:db 0,_nec,ow,0Fh,22h,_end             ;SUB4S Sub CL nibbles BCD, DS:SI - ES:DI (CL even,NZ)
t_testbit:
	db nb+ol2+eao,-1,_cl,_nec,ow,0Fh,10h,x0m,0   ;TESTBIT eb,CL Test bit CL of eb, set Z flag
	db wb+ol2+eao,-1,_cl,_nec,ow,0Fh,11h,x0m,0   ;TESTBIT ev,CL N Test bit CL of ew, set Z flag
	db nb+ol2+eao,-1,co,_nec,ow,0Fh,18h,x0m,0    ;TESTBIT eb,ib N Test bit ib of eb, set Z flag
	db wb+ol2+eao,-1,co,_nec,ow,0Fh,19h,x0m,_end ;TESTBIT ew,ib N Test bit ib of ew, set Z flag
t_brkem:db nb+ol1+soc,nv25,ow,0fh,0ffh,_end ;Break for emulation

	;NEC/Sony V25 instructions
t_movspa:                               ;Move stack pointer after bank switch
	db 0,nv25,ow,0fh,25h,_end
t_movspb:                               ;Move stack pointer before bank switch
	db _ow+ol1+sor,nv25,os,0fh,"27r",_end
t_retrbi:
	db 0,nv25,ow,0fh,91h,_end       ;Return from register bank context switch interrupt
t_stop: db 0,nv25,ow,0fh,0beh,_end      ;Stop CPU
t_tsksw:db _ow+ol1+soc,nv25,ow,0fh,94h,s,"27r",_end ;Task switch
t_brkcs:db _ow+ol1+sor,nv25,ow,0fh,2dh,s,"27r",_end ;Break with context switch
t_brkn: db nb+ol1+soc,nv25,o,63h,_end   ;Break to native mode
t_brks: db nb+ol1+soc,nv25,o,0f1h,_end  ;Break to security mode
	;Assembled manually :
	;t_btclr:db

	;NEC/Sony V33 instructions
t_brkxa:db nb+ol1+soc,nv33,ow,0fh,0e0h,_end ;Break to expansion address
t_retxa:db nb+ol1+soc,nv33,ow,0fh,0f0h,_end ;Return from expansion address

t_debug:db ?

	;MMX instructions
t_emms:
	db _end

t_umov: db _b+ol2,_386,der,ow,0fh,10h,xrm,_end
t_salc: db 0,o,0d6h,_end
t_icebp:db 0,_386,o,0f1h,_end

_q=1
mmxe=1
mmxp=1
mmxpi=1
mmxpe=1
mmxep=1
t_punpcklbw:db _d+ol2,mmxe,ow,0fh,60h,_end
t_punpcklwd:db _d+ol2,mmxe,ow,0fh,61h,_end
t_punpckldq:db _d+ol2,mmxe,ow,0fh,62h,_end
t_packsswb:db _q+ol2,mmxe,ow,0fh,63h,_end
t_pcmpgtb:db _q+ol2,mmxe,ow,0fh,64h,_end
t_pcmpgtw:db _q+ol2,mmxe,ow,0fh,65h,_end
t_pcmpgtd:db _q+ol2,mmxe,ow,0fh,66h,_end
t_packuswb:db _q+ol2,mmxe,ow,0fh,67h,_end
t_pshimw:db 0,_mmx,ow,0fh,71h,_end
t_pshimd:db 0,_mmx,ow,0fh,72h,_end
t_pshimq:db 0,_mmx,ow,0fh,73h,_end
t_pcmpeqb:db _q+ol2,mmxe,ow,0fh,74h,_end
t_pcmpeqw:db _q+ol2,mmxe,ow,0fh,75h,_end
t_pcmpeqd:db _q+ol2,mmxe,ow,0fh,76h,_end
t_psrlw:db _q+ol2,mmxe,ow,0fh,0d1h,_end
t_psrld:db _q+ol2,mmxe,ow,0fh,0d2h,_end
t_psrlq:db _q+ol2,mmxe,ow,0fh,0d3h,_end
t_pmullw:db _q+ol2,mmxe,ow,0fh,0d5h,_end
t_psraw:db _q+ol2,mmxe,ow,0fh,0e1h,_end
t_psrad:db _q+ol2,mmxe,ow,0fh,0e2h,_end
t_pmulhw:db _q+ol2,mmxe,ow,0fh,0e5h,_end    
t_psllw:db _q+ol2,mmxe,ow,0fh,0f1h,_end
t_pslld:db _q+ol2,mmxe,ow,0fh,0f2h,_end
t_psllq:db _q+ol2,mmxe,ow,0fh,0f3h,_end
t_pmaddwd:db _q+ol2,mmxe,ow,0fh,0f5h,_end
t_punpckhbw:db _q+ol2,mmxe,ow,0fh,68h,_end
t_punpckhwd:db _q+ol2,mmxe,ow,0fh,69h,_end
t_punpckhdq:db _q+ol2,mmxe,ow,0fh,6ah,_end
t_packssdw:db _q+ol2,mmxe,ow,0fh,6bh,_end
t_movd:db _d+ol2,mmxpe,ow,0fh,6eh,0
       db _q+ol2,mmxep,ow,0fh,7eh,_end
t_movq:db _q+ol2,mmxe,ow,0fh,6fh,0
       db _q+ol2,mmxe,ow,0fh,7fh,_end
t_psubusb:db _q+ol2,mmxe,ow,0fh,0d8h,_end
t_psubusw:db _q+ol2,mmxe,ow,0fh,0d9h,_end
t_pand:db _q+ol2,mmxe,ow,0fh,0dbh,_end
t_paddusb:db _q+ol2,mmxe,ow,0fh,0dch,_end
t_paddusw:db _q+ol2,mmxe,ow,0fh,0ddh,_end
t_pandn:db _q+ol2,mmxe,ow,0fh,0dfh,_end
t_psubsb:db _q+ol2,mmxe,ow,0fh,0e8h,_end
t_psubsw:db _d+ol2,mmxe,ow,0fh,0e9h,_end
t_por:db _q+ol2,mmxe,ow,0fh,0ebh,_end
t_paddsb:db _q+ol2,mmxe,ow,0fh,0ech,_end
t_paddsw:db _q+ol2,mmxe,ow,0fh,0edh,_end
t_pxor:db _q+ol2,mmxe,ow,0fh,0efh,_end
t_psubb:db _q+ol2,mmxe,ow,0fh,0f8h,_end
t_psubw:db _q+ol2,mmxe,ow,0fh,0f9h,_end
t_psubd:db _q+ol2,mmxe,ow,0fh,0fah,_end
t_paddb:db _q+ol2,mmxe,ow,0fh,0fch,_end
t_paddw:db _q+ol2,mmxe,ow,0fh,0fdh,_end
t_paddd:db _q+ol2,mmxe,ow,0fh,0feh,_end
t_psrl:db _q+ol2,mmxpi,ow,0fh,0h,_end
t_psra:db _q+ol2,mmxpi,ow,0fh,0h,_end
t_psll:db _q+ol2,mmxe,ow,0fh,0h,_end

Mnemonics:
; First of all the commands with a P-Flag. This'll make things easier for
; comparison in the parse engine
	;p1=0-7
	db "add",0,"or",0,"adc",0,"sbb/bc",0,"and",0,"sub",0,"xor",0,"cmp",0
	;p2=4-7
	db "mul",0,"-",0,"div",0,"idiv",0
	;          imul entfernt
	;p3=0-5/7
	db "rol",0,"ror",0,"rcl",0,"rcr",0,"shl/al",0,"shr",0,"-",0,"sar",0
	;p4=4-7
	db "bt",0,"bts",0,"btr",0,"btc",0
	; Usual commands
	db"not",0,"neg",0,"inc",0,"dec",0,"test",0
	db"imul",0,"shld",0,"shrd",0
	db"daa",0,"das",0,"aaa",0,"aas",0,"aam",0,"aad",0 
	db"movsx",0,"movzx",0,"cbw",0,"cwde",0,"cwd",0,"cdq",0 
	db"bswap",0,"xlat/latb",0 
	db"bsf",0,"bsr",0,"cmpxchg",0,"xadd",0 
	db"nop",0,"wait",0,"lock",0,"hlt",0
	db"int",0,"into",0,"iret",0
	db"popf",0,"popfd",0,"pushf",0,"pushfd",0
	db"sahf",0,"lahf",0
	db"cmc",0,"clc",0,"stc",0,"cli",0,"sti",0,"cld",0,"std",0
	db"push",0,"pusha",0,"pushad",0 
	db"pop",0,"popa",0,"popad",0
	db"xchg",0,"mov",0
	db"lea",0,"lss",0,"lfs",0,"lgs",0,"les",0,"lds",0
	db"addr:",0,"rand:",0
	db"es:",0,"cs:",0,"ss:",0,"ds:",0,"fs:",0,"gs:",0
	db"in",0,"out",0 
	db"insb",0,"insw",0,"insd",0
	db"outsb",0,"outsw",0,"outsd",0
	db"movsb",0,"movsw",0,"movsd",0
	db"cmpsb",0,"cmpsw",0,"cmpsd",0
	db"stosb",0,"stosw",0,"stosd",0
	db"lodsb",0,"lodsw",0,"lodsd",0
	db"scasb",0,"scasw",0,"scasd",0
	db"rep/epne/epnz",0
	db"repe/epz",0
	db"jcxz",0,"jecxz",0,"loop",0,"loopz/oope",0,"loopnz/oopne",0
	db"jmp short",0,"jmp near",0,"jmp far/mpf",0,"jmp",0
	db"call",0,"call far/allf",0
	db"ret",0,"retf/et far",0
	db"enter",0,"leave",0
	db"bound",0,"arpl",0
	db"sldt",0,"str",0,"lldt",0,"ltr",0,"verr",0,"verw",0 
	db"lar",0,"lsl",0
	db"sgdt",0,"sidt",0,"lgdt",0,"lidt",0 
	db"smsw",0,"lmsw",0,"clts",0 
	db"invd",0,"wbinvd",0,"invlpd",0
	;Intel Pentium commands
	db"wrmsr",0,"cmpx8/mpxchg8b",0,"cpuid",0,"rdmsr",0,"rdtsc",0,"rsm",0
	;Intel Pentium Pro instructions
	db"rdpmc",0,"ud2",0,"emmx",0
	;NEC V20/V30 commands
	db"add4s",0,"calln/all80",0,"clrbit",0,"cmp4s",0
	db"lodbits",0,"notbit",0,"repc",0,"repnc",0
	db"rol4",0,"ror4",0,"setbit",0,"stobits",0,"sub4s",0,"testbit",0
	db"brkem",0
	;NEC/Sony V25 instructions
	db"movspa",0,"movspb",0,"retrbi",0,"stop",0,"tsksw",0,"brkcs",0
	db"brkn",0,"brks",0
	;NEC/Sony V33 instructions
	db"brkxa",0,"retxa",0

	db"debug",0

	;MMX instructions
	db"movd",0,"movq",0             ;Move MMX reg
	db"packus/packuswb",0           ;pack MMX reg with unsigned saturation
	db"packsswb",0,"packssdw",0     ;pack MMX reg with signed saturation
	db"punpckhbw",0,"punpckhwd",0,"punpckhdq",0 ;unpack high order
	db"punpcklbw",0,"punpcklwd",0,"punpckldq",0 ;unpack low order
	db"paddb",0,"paddw",0,"paddd",0 ;add MMX reg with wrap-around
	db"paddsb",0,"paddsw",0         ;" with signed saturation
	db"paddusb",0,"paddusw",0       ;" with unsigned saturation
	db"psubb",0,"psubw",0,"psubd",0 ;subtract MMX reg
	db"psubsb",0,"psubsw",0
	db"psubusb",0,"psubusw",0
	db"pmulh/pmulhw",0
	db"pmull/pmullw",0
	db"pmadd/pmaddwd",0
	db"psllw",0,"pslld",0,"psllq",0
	db"psrlw",0,"psrld",0,"psrlq",0
	db"psraw",0,"psrad",0
	db"pcmpeqb",0,"pcmpeqw",0,"pcmpeqd",0
	db"pcmpgtb",0,"pcmpgtw",0,"pcmpgtd",0
	db"pand",0
	db"pandn",0
	db"por",0
	db"pxor",0
	db"emms",_end

TAB_mnemonics:
	dw t_not,t_neg,t_inc,t_dec,t_test
	dw t_imul,t_shld,t_shrd
	dw t_daa,t_das,t_aaa,t_aas,t_aam,t_aad
	dw t_movsx,t_movzx,t_cbw,t_cwde,t_cwd,t_cdq
	dw t_bswap,t_xlatb
	dw t_bsf,t_bsr,t_cmpxchg,t_xadd
	dw t_nop,t_wait,t_lock,t_hlt
	dw t_int,t_into,t_iret
	dw t_popf,t_popfd,t_pushf,t_pushfd
	dw t_sahf,t_lahf
	dw t_cmc,t_clc,t_stc,t_cli,t_sti,t_cld,t_std
	dw t_push,t_pusha,t_pushad
	dw t_pop,t_popa,t_popad
	dw t_xchg,t_mov
	dw t_lea,t_lss,t_lfs,t_lgs,t_les,t_lds
	dw t_addr,t_rand
	dw t_es,t_cs,t_ss,t_ds,t_fs,t_gs
	dw t_in,t_out
	dw t_insb,t_insw,t_insd
	dw t_outsb,t_outsw,t_outsd
	dw t_movsb,t_movsw,t_movsd
	dw t_cmpsb,t_cmpsw,t_cmpsd
	dw t_stosb,t_stosw,t_stosd
	dw t_lodsb,t_lodsw,t_lodsd
	dw t_scasb,t_scasw,t_scasd
	dw t_rep
	dw t_repe
	dw t_jcxz,t_jecxz,t_loop,t_loopz,t_loopnz
	dw t_jmpshort,t_jmpnear,t_jmpfar,t_jmp
	dw t_call,t_callfar
	dw t_ret,t_retf
	dw t_enter,t_leave
	dw t_bound,t_arpl
	dw t_sldt,t_str,t_lldt,t_ltr,t_verr,t_verw
	dw t_lar,t_lsl
	dw t_sgdt,t_sidt,t_lgdt,t_lidt
	dw t_smsw,t_lmsw,t_clts
	dw t_invd,t_wbinvd,t_invlpd

	;Intel Pentium instructions
	dw t_wrmsr,t_cmpx8,t_cpuid,t_rdmsr,t_rdtsc,t_rsm

	;Intel Pentium Pro instructions
	dw t_rdpmc,t_ud2,t_emmx

	;NEC/Sony V20/V30 instructions
	dw t_add4s,t_call80,t_clrbit,t_cmp4s
	dw t_lodbits,t_notbit,t_repc,t_repnc
	dw t_rol4,t_ror4,t_setbit,t_stobits,t_sub4s,t_testbit
	dw t_brkem

	;NEC/Sony V25 instructions
	dw t_movspa,t_movspb,t_retrbi,t_stop,t_tsksw,t_brkcs
	dw t_brkn,t_brks

	;NEC/Sony V33 instructions
	dw t_brkxa,t_retxa

	dw t_debug

	;MMX instructions
#if billgatesisdead ;lang_german
	dw t_movd,t_movq                ;Move MMX reg
	dw t_packus                     ;pack MMX reg with unsigned saturation
	dw t_packsswb,t_packssdw        ;pack MMX reg with signed saturation
	dw t_punpckhbw,t_punpckhwd,t_punpckhdq ;unpack high order
	dw t_punpcklbw,t_punpcklwd,t_punpckldq ;unpack low order
	dw t_paddb,t_paddw,t_paddd      ;add MMX reg with wrap-around
	dw t_paddsb,t_paddsw            ;" with signed saturation
	dw t_paddusb,t_paddusw          ;" with unsigned saturation
	dw t_psubb,t_psubw,t_psubd      ;subtract MMX reg
	dw t_psubsb,t_psubsw
	dw t_psubusb,t_psubusw
	dw t_pmulh
	dw t_pmull
	dw t_pmadd
	dw t_psllw,t_pslld,t_psllq
	dw t_psrlw,t_psrld,t_psrlq
	dw t_psraw,t_psrad
	dw t_pcmpeqb,t_pcmpeqw,t_pcmpeqd
	dw t_pcmpgtb,t_pcmpgtw,t_pcmpgtd
	dw t_pand
	dw t_pandn
	dw t_por
	dw t_pxor
#endif
	dw t_emms


