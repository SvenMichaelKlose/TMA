;Generation patterns for coprocessor instructions so far...
; (c)1996 S.Klose
	.code
	db"fld",0,"-","fst",0,"fstp",0
	db"fild",0,".","fist",0,"fistp",0
;fxch

pcommands5r:
	db r32,"1pxm",0
	db r64,"5pxm",0
	db r80,"35xm",0
	db r80,"37xm",_end
pcommands5i:
	db i16,"7pxm",0
	db i32,"3pxm",0
	db i64,"75xm",0
	db i64,"77xm",_end
pcommands5b:
	db bcd,"74xm",0
	db bcd,"76xm",0
	db _st,"103m",0
	db _st,"113m",0
	db _st,"523m",0
	db _St,"533m",_end

	db"--","fcom",0,"fcomp",0
	db"--","ficom",0,"ficomp",0

pcommands6:
	db r32,"0pxm",0
	db i32,"2pxm",0
	db r64,"4pxm",0
	db i16,"6pxm",0
	db _st,"op3m",_ends

	db"fadd",0,"fmul",0,"fsub",0,"fsubr",0,"fdiv",0,"fdivr",0
	db"fiadd",0,"fimul",0,"fisub",0,"fisubr",0,"fidiv",0,"fidivr",0
	db"faddp",0,"fmulp",0,"fsubp",0,"fsubrp",0,"fdivp",0,"fdivrp",0

pcommands7:
	db r32,"0pxm",0
	db i32,"2pxm",0
	db r64,"4pxm",0
	db i16,"6pxm",0
	db _st,"0p3m",0
	db _st,"4p3m",0
	db _st,"6p3m",_end
 
t_fcompp:
	db 0,"6331",_end
t_ftst: db 0,"1434",_end

FXAM	      examine ST
t_fxam: db 0,"1435",_end
t_fucom:db _st,"543m",_end
t_fucomp:
	db _st,"553m",_end
t_fucompp:
	db 0,"2531",_end
 
t_fld1: db 0,"1530",_end
t_fldl2t:
	db 0,"1531",_end
t_fldl2e:
	db 0,"1532",_end
t_fldpi:db 0,"1533",_end
t_fldlg2:
	db 0,"1534",_end
t_fldln2:
	db 0,"1535",_end
t_fldz: db 0,"1536",_end
t_fchs: db 0,"1430",_end
t_fabs: db 0,"1431",_end
t_f2xm1:db 0,"1630",_end
t_fyl2x:db 0,"1631",_end
t_fptan:db 0,"1632",_end
t_fpatan:
	db 0,"1633",_end
t_fxtract:
	db 0,"1634",_end
t_prem1:db 0,"1635",_end
t_fprem:db 0,"1730",_end
t_fyl2xpi:
	db 0,"1731",_end
t_sqrt: db 0,"1732",_end
t_fsincos:
	db 0,"1733",_end
t_frndint:
	db 0,"1734",_end
t_fscale:
	db 0,"1735",_end
t_fsin: db 0,"1736",_end
t_fcos: db 0,"1737",_end
t_fnop: db 0,"1230",_end
t_Ffldenv:
	db 0,"14xm",_end
t_fldcw:db 0,"15xm",_end
t_fstenv:
	db 0,"16xm",_end
t_fstcw:db e,"17xm",_end
t_fdecstp:
	db 0,"1636",_end
t_fincstp:
	db 0,"1637",_end
t_feni: db 0,"3430",_end
t_fdisi:db 0,"3431",_end
t_fclex:db 0,"3432",_end
t_finit:db 0,"3433",_end
t_fsetpe:
	db 0,"3434",_end
t_ffree:db _st,"503m",_end
t_frstor:
	db e,"54xm",_end
t_fsave:db e,"56xm",_end
t_fstsw:db e,"57xm",_end
t_fstsw:db _ax,"7430",_end

