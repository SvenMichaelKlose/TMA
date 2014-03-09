; DOS-Devicetreiber-Header

        title "DOS-Devicetreiber-Header"

		dd ?	 ;Zeiger auf n„chsten Treiber oder -1
dev_attribut:	dw ?	;Attributwort

	dev_replace_std =1
	chr_nuldev	=bit 2
	chr_clockdev	=bit 3
	dev_ocr 	=bit 11
	blk_fat 	=bit 13
	chr_otb 	=bit 13
	dev_ioctl	=bit 14
	blk_driver	=0
	dev_driver	=bit 15

		dw dev_strategy
		dw dev_interrupt
		db 8 dup " "	;Block: Anzahl der Units
				;Char:	Name des Treibers
request_header:
		db >l1-$	;Gr”sse des Headers
		db ?		;Block: Angesprochene Unit
command_code	db ?		;Befehlscode
status_word	dw ?		;Statuswort

	;Statuswort:
	;
	; Bits 0-7: Fehlercode bei gesetztem dev_errorflag.
	dev_opok	=bit 8
	dev_busy	=bit 9
	dev_errorflag	=bit 15
	;Fehlercodes:
	enum	der_writeprot, \	;Diskette ist schreibgeschtzt
		der_invalid_unit, \	;Ungltige Unit
		der_drv_not_rdy, \	;Laufwerk nicht bereit
		der_inv_cmdcode, \	;Ungltiger Befehlscode
		der_inv_chksum, \	;Ungltige Prfsumme
		der_inv_reqlen, \	;Falsche L„nge des Requestheaders
		der_seek_error, \	;
		der_inv_format, \	;Ungltiges Diskettenformat
		der_inv_sector, \	;Ungltiger Sektor
		der_no_paper, \ 	;Kein Papier im Drucker
		der_write_error, \	;Schreibfehler
		der_read_error, \	;Lesefehler
		der_error, \		;Allgemeiner Fehler
		der_dskchange		;Unerlaubter Diskettenwechsel

		db 8 dup ?	;reserviert
dev_parameters:
l1:
