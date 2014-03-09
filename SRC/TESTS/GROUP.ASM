dataseg group _text,data

dataseg group _text,data
	.code
	nop

_text	segment
	db"Test des GROUP-Directivs",0

	.data
testvar dw ?
