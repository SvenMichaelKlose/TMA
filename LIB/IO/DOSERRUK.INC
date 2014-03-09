;Needs DOSERRMN.INC
#if !_mdltiny
_text   segment para
#else
	.code
#endif

TXT_doserrors:
	db"$Invalid function number !"
	db 10,13,"$"
	db"File not found.$"
	db"Directory not found.$"
	db"Too many open files.$"
	db"Access to file denied.$"
	db"Invalid file handle.$"
	db"Memory control blocks are destroyed.$"
	db"Not enough memory to start program.",10,13
	db"See the doc file(s) to solve this problem.$"
	db"Can't allocate memory block. Invalid adress or segment used.$"
	db"Invalid user environment.$"
	db"Unknown file format.$"
	db"Invalid file access code.$"
	db"Invalid data for access on device.$"
	db"DOS: Internal error. Description not possible.",10,13
	db"You must be using MS-DOS (tm), don't you (y/y)?$"
	db"Invalid drive specification.$"
	db"Can't delete actual directory. Get back to the parent one and try again.$"
	db"You will have to specify the same drive for this operation.$"
	db"Can't find file.$"
	db"Disk is write protected.$"
	db"Error at access on disk drive. Bad disk unit.$"
	db"Drive is not ready.$"
	db"Invalid disk command at access on drive.$"
	db"CRC at reading from drive.$"
	db"Disk access with invalid request structure length.$"
	db"Couldn't find track on disk. Seek error.$"
	db"Not a DOS disk present in accessed drive.$"
	db"Couldn't find sector on accessed drive.$"
	db"Apply some paper to your printer.$"
	db"Error writing data block to disk.$"
	db"Error reading data block from disk.$"
	db"Device is not present.$"
	db"File sharing voilation occured.$"
	db"File sharing lock violation occured.$"
	db"Wrong disk. Can't access.$"
	db"File control block is unavailable.$"
	db"File sharing buffer overflow.$"
	db"DOS: Internal error. Description not possible.$"
	db"Couldn't finsish file access.$"
	db"A network error occured.$"

