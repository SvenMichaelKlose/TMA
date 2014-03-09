;TMA macro assembler
;Simple editor
; (c)1996-1998 S.Klose

; Sorry, this is not fully translated into English. FIXME.

	.data
ed_datastart:
screen_addr     dd ?            ;Position in Text relative to display start
crsr_line       dd ?            ;Line number relative to first line on screen
displaypos      dw ?            ;Output position in screen
displaycol      db ?            ;Output color
displaymode     db ?            ;Display mode
filesize        dd ?            ;Text size in seg_text
screenheight    dw ?            ;Height of text window in lines.
screenwidth     dw ?            ;Width of text window in columns
stat_pos        dw ?            ;Position of status message line
column          db ?            ;Actual column
coloffset       dw ?            ;Horizontal scroll offset
crsrx           dw ?            ;Cursor position
crsry           dw ?
crsr_pos        dw ?            ;Cursor position within a line
crsr_addr       dd ?            ;Starting position of the line the cursor
				;points to.
actual_line     dw ?
disp_line       dw ?
linepos         dw ?            ;Position in cursor line
filehandle      dw ?
buffer:         db 256 dup ?    ;Buffer for the actual line
bufferlen       dw ?            ;...and its length
maxbufferlen    dw ?            ;Maximum length of line
oldbufferlen    dw ?            ;Old       "
flg_buffer      db ?            ;Is true if line in buffer was modified.
ed_hlp          dd ?            ;Buffer
				;1=don't scan for next line
mnu_map:        dd 16 dup ?     ;Adresses und control codes of menupages
pagepos         dw ?            ;Current position in menu page
pagesize        dw ?            ;Size of menu page
hotkeys         dw ?            ;# of hotkeys
; We use the same line editor for texts and input lines (f.e. in the
; file selector box.
input_mode      dw ?            ;0=Text mode
				;1=Input mode
buffer_origin   dw ?            ;Ursprung des Pufferinhalts
BUF_currentDir:                 ;Aktuelles Directory
		db ?,?,?
BUF_currentDir2:
		db 64 dup ?
dta:            db 256 dup ?    ;DTA
dirstart        dw ?            ;Startzeile des Directories
dirsize         dw ?            ;Hîhe des Directories
dirpos          dw ?            ;Position im Directory
files           dw ?            ;Anzahl der DirectoryeintrÑge
flag_directory  db ?            ;1, wenn ein Directory angewÑhlt wurde
flag_input      db ?            ;1, wenn Zeileinhalt temporÑr ist
flag_nocomp     db ?            ;1, wenn Kompression abgeschaltet ist
flag_change     db ?            ;Zeigt an, ob die geladene Datei verÑndert wurde
flag_priname    db ?            ;Flag fÅr eingetragenes Projekt
marked_start    dd ?            ;Anfang des markierten Blocks
marked_end      dd ?            ;Ende "
oldstate        db ?            ;alter Zustand der Tastatur
buffersize      dd ?
buf_search:     db 128 dup ?    ;Buffer fÅr Sucheingabe
tmpvar          dd ?            ;temporÑre Variable
tmpvar2         dd ?
		;
windowend       dd ?            ;Grîsse der Datei im XMS
windowsize      dd ?            ;Grîsse des zuletzt ausgelesenen Fensters
windowpos       dd ?            ;Position des aktuellen XMS-Fensters
filelist:	db 2k dup ?
textname:       db 64 dup ?
priname:        db 64 dup ?

autosave_timer  dd ?            ;ZÑhler fÅr Autosave
ed_dataend:

	.code
distance  = 8192        ;XMS-Fenstergrîsse/4
;tab       = 9
cr        = 13
lf        = 10
crsr_up   = 72
crsr_down = 80
crsr_left = 75
crsr_right= 77
pg_up     = 73
pg_down   = 81
pos1      = 71
endpos    = 79
backspace = 14
del    = 83
_enter    = 28
enddot	  = 0fah
textmode  = 0
inputmode = 1
_read     = 1
_hid      = 2
_sys      = 4
_dir      = 10h
_arch     = 20h
_shift    = 2

autosave_delay = 1*6*182

fselector macro
	mov si,#1
	call _fsb
	endm

show_stat macro
	mov si,#1
	call _show_stat
	endm

input_line macro
	mov dx,#2 by #1
	mov si,#3
	call _input_line
	endm

edit:   mov ax,cs
	mov ds,ax
	mov es,ax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	xor edi,edi
	mov si,txt_init
	call asciiz
	mov di,ed_datastart     ;Datenbereich lîschen
	mov cx,ed_dataend-ed_datastart
	xor al,al
	rep stosb
        mov b[flag_ed],2
	debug
	new 64k,seg_text
	new 64k,seg_buffer
	;
	call ed_init_screen
	mov b[flag_ed],1
	mov w screenheight,23	;Grîsse des Textfensters vermerken
	mov w screenwidth,80
	mov w maxbufferlen,256
	mov di,buf_search
	mov ax,70
	call init_input_buffer
	;
	mov es,40h              ;Autosave initialisieren
	mov eax,[es:6ch]
	mov autosave_timer,eax
	;
	mov si,81h              ;Dateinamen holen
	mov di,textname
	call getnamex
	jcxz >k1                ;keine Datei angegeben
	mov dx,textname
	call read_file
	jmp >l1
	;
k1:     call create_new_file
l1:     call line2buffer
l100:   show_stat stat_standard
l0:	call display_text
	call draw_menu
	mov ax,fs
	mov ds,ax
	mov es,ax
l3:     call getkey
	mov bx,ax
	mov ah,2                ;PrÅfen, ob die ALT-Taste gedrÅckt wurde
	int 16h
	test al,1000b
	jz >l10
	push bx                 ;Taste retten
	mov di,scn_german
	mov al,bh
	mov cx,26
	rep scasb
	mov ax,di
	sub ax,scn_german+1-"A"
	cmp al,"a"
	if nb sub al,"a"+"A"
	mov cx,hotkeys
	mov bx,mnu_map+1
l6:     cmp [bx],al
	je >l7
	add bx,4
	loop l6
	pop bx
	or bl,bl
	jne >l10
unknown_command:
	show_stat stat_unknown
	jmp l0
	;
l7:     mov si,[bx+1]           ;Ja, Menue aufrufen
	mov bl,[bx-1]           ;Position auf dem Bildschirm
	xor bh,bh
	call menu
	pop bx
	jnc l100
	jmp l0
	;
l10:    xchg ax,bx
	cmp ah,crsr_down
	jne >l4
	mov ax,crsr_addr
	push ax
	call buffer2line	;Puffer in den Text einsetzen
	call cursor_down
	pop ax
p2:     call set_marker
	jmp l1
	;
l4:     cmp ah,crsr_up
	jne >l2
	mov ax,crsr_addr
	push ax
	call buffer2line        ;Puffer in den Text einsetzen
	call cursor_up
	pop ax
	jmp p2
	;
l2:     cmp ah,pg_down
	jne >l2
	mov ax,crsr_addr
	push ax
	call buffer2line
	mov cx,screenheight
d3:     call cursor_down
	loop d3
	pop ax
	call set_marker
	jmp l1
	;
l2:     cmp ah,pg_up
	jne >l2
	mov ax,crsr_addr
	push ax
	call buffer2line
	mov cx,screenheight
d3:     call cursor_up
	loop d3
	pop ax
	call set_marker
	jmp l1
	;
l2:     mov [oldstate],bl
	cmp ah,crsr_left
	jne >l2
	call cursor_left
	jmp l0
	;
l2:     cmp ah,crsr_right
	jne >l2
	call cursor_right
	jmp l0
	;
l2:     cmp ah,pos1
	jne >l2
	call go_pos1            ;An den Anfang der Zeile
	jmp l0

l2:     cmp ah,endpos
	jne >l2
	call go_end
	jmp l0

l2:     cmp ah,del
	jne >l2
	mov b flag_change,1
	call delete_char
	jmp l0

l2:     cmp ah,backspace        ;RÅckschritt ?
	jne >l2
	mov b flag_change,1
	call cursor_left        ;Cursor nach links
	jc >l4                  ;geht nicht...
	call delete_char        ;Zeichen lîschen
	jmp l0
	;
l4:     call buffer2line        ;Zeile in den Text einbauen
	call cursor_up          ;eine Zeile hoch
	jc l1                   ;geht nicht...
	call line2buffer        ;Neue Zeile einlesen
	call go_end             ;an's Ende der Zeile
	call delete_char        ;Zeichen lîschen, nÑchste Zeile wird angehÑngt
	jmp l0

l2:     cmp ah,1
	je ende                 ;Ende bei Escape
	;
	cmp ah,_enter           ;Enter ?
	jne >l2
	mov b flag_change,1
	mov b flag_nocomp,1
	mov ah,cr               ;Ja, Zeilenende einfÅgen...
	call insert_char
	mov ah,lf
	call insert_char
	call buffer2line        ;...und an den Anfang der nÑchsten Zeile
	mov b flag_nocomp,0
	call cursor_down
	call line2buffer
	call go_pos1
	jmp l1

l2:     cmp al,tab
	je >o1
	cmp al," "
	jb unknown_command      ;Keine bekannte Tastenkombination
o1:     mov b flag_change,1
	mov ah,al
	call insert_char
	jmp l0                  ;Weiter, ohne Zeile in den Text einzusetzen

ende:   call buffer2line
	call ask2save
	jc l1
ende2:  delete [seg_text]
	delete [seg_buffer]
	mov ax,3
	int 10h
	mov dx,registered       ;Namen des eingetragenen Benutzers ausgeben,
	mov ah,9
	int 21h
	xor al,al
	jmp _exit

; -----------------------------------------------------------------------------

; >>> DWord an aktueller Bildschirmposition ausgeben <<<
print_dword:
	mov cx,8
l1:     rol eax,4
	push eax
	and al,0fh
	cmp al,9
	if a add al,"A"-"0"-10
	add al,"0"
	call display_char2
	pop eax
	loop l1
	ret

; ==========================================================
; >>> Stellt ein einzelnes Zeichen im Editierbereich dar <<<
; ==========================================================
; Aufruf: AL=ASCII-Zeichen
; AL bleibt beim RÅcksprung erhalten.
; Zeichen wird nur ausgegeben, wenn BP=0 ist.
display_char:
	or bp,bp
	jnz >l1
display_char2:
	push es,bx
	mov es,0b800h
	mov bx,fs:displaypos
	mov ah,fs:displaycol
	mov [es:bx],ax
	add bx,2
	mov [fs:displaypos],bx
	pop bx,es
	ret
	;
l1:     dec bp
	inc cx
	ret

check_line:
	mov ah,1eh              ;Zeilenfarbe setzen.
	mov dx,fs:marked_start  ;Zeile markiert ?
	mov bx,fs:marked_end
	cmp dx,bx
	jbe >r1
	xchg bx,dx
r1:     cmp si,dx
	jb >p5
	cmp si,bx
	jae >p5
	mov ah,0e1h             ;Ja, andere Farbe...
p5:     mov fs:displaycol,ah    ;...eintragen.
	;
	mov ax,[fs:tmpvar2]
	cmp fs:actual_line,ax   ;aktuelle Cursorzeile ?
	jne >l2                 ;Nein, ...
	;
	push ds                 ;Ja, Zeile im Zeilenpuffer anzeigen.
	mov ds,fs
	mov b displaymode,1
	mov ed_hlp,si
	mov si,buffer
	call display_line
	mov bx,ofs ed_hlp       ;Zeiger auf nÑchste Zeile.
	call get_next_line
	jnc >y1
	mov si,fs:filesize
	jmp >y2
y1:     mov si,fs:ed_hlp
y2:     pop ds
	clc                     ;EOF
	ret
l2:     stc                     ;...Zeile freigeben
	ret

display_errmsg:
	pusha
	push ds
	mov ds,fs
	mov cx,[total_errors]
	jcxz >n0
	mov bx,error_list
	mov ax,[tmpvar2]
	dec ax
l5:     cmp error_line[bx],ax
	jne >n1
	inc w tmpvar            ;PrÅfen, ob Zeile im Bildschirm liegt
	mov dx,tmpvar
	cmp screenheight,dx
	jb >n2
	mov b displaycol,4eh
	call geterrorpos
	push cx         
	call finish_line
	mov al,1eh
	call display_char
	pop ax
	mov cx,79
	sub cx,ax
	call finish_line
	inc w tmpvar            ;PrÅfen, ob Zeile im Bildschirm liegt.
	mov dx,tmpvar
	cmp screenheight,dx
	jb >n2
	mov b displaycol,4fh
	call showerrornr        ;Fehlernummer ausgeben
	mov b displaycol,4eh
	mov al,[error_nr+bx]    ;Fehlermeldung dahinter
	xor ah,ah
	mov di,errormsgs
	push cx
	call get_list_entry
	pop cx
	mov si,di
	call print_string
	call finish_line
	mov b displaycol,1eh
	jmp >n2
n1:     add bx,error_size
	dec cx
	jnz l5
n0:     clc
	jmp >n4
n2:     stc
n4:     pop ds
	popa
	ret

geterrorpos:
	mov cl,b[bx+error_pos]  ;Fehlerposition holen
	xor ch,ch
	cmp cl,79
	if a xor cl,cl
	sub cx,coloffset
	if c xor cx,cx
	ret

showerrornr:
	push bx
	mov al," "
	call display_char
	mov ax,bx
	sub ax,error_list
	xor dx,dx
	mov bx,error_size
	div bx
	inc ax
	mov cx,77
	push ax
	call printdecword
	pop ax
	cmp w[total_errors],ax
	jne >d1
	mov b displaycol,4ah
	mov si,txt_LASTERR
	call print_string
d1:     mov b displaycol,4fh
	mov al,":"
	call display_char
	mov al," "
	call display_char
	pop bx
	ret

; =================================
; >>> Stellt einzelne Zeile dar <<<
; =================================
; Aufruf: DS:SI=Zeilenanfang
; RÅckgabe: DS:SI=Zeiger auf nÑchste Zeile
;
; Geht ein Zeiger Åber das Ende des Textes hinaus, so wird die aktuelle
; Zeile einfach gelîscht.
display_line:
	push cx
	mov b fs:column,0
	mov cx,fs:screenwidth
	mov bp,fs:coloffset
	cld
	cmp b fs:displaymode,1
	je >l2
	cmp si,fs:filesize
	jae >l3
l2:     lodsb           ;Zeichen holen
	cmp al,13       ;Ende der Zeile ?
	je >l1          ;Yep, Ende der Zeile suchen
	cmp al,tab
	je >t1
	cmp al,20h      ;Steuerzeichen...
	jb l2           ;...vergessen.
	call display_char       ;Zeichen auf dem Screen ausgeben.
	inc b fs:column
	dec cx          ;Ist die Zeile voll ?
	jnz l2          ;Nein, weiter...
	;
l3:     lodsb           ;Das Ende der aktuellen Zeile...
	cmp al,20h
	jnb l3
	;
l1:     jcxz >l1
	mov al,enddot
	call display_char
	dec cx
	jz >l1
	cmp b fs:displaymode,1
	je >l6
l5:     lodsb           ;...und den Anfang der nÑchsten suchen.
	cmp al,13       ;Leerzeile !
	je >l3
	cmp al,tab
	je >l3
	cmp al,20h
	jb l5
l3:     dec si
	;
l6:     jcxz >l1        ;Die Zeile wurde komplett beschrieben.
	mov al," "      ;Rest der Zeile beschreiben
x2:     call display_char
	loop x2
	;
l1:     pop cx
	ret

t1:     mov al," "              ;TAB ausgeben
t2:     call display_char
	inc b fs:column
	dec cx
	jz l3                   ;Ende der Zeile
	test b fs:column,7
	jnz t2
	jmp l2

; =================================
; >>> Stellt aktuelle Seite dar <<<
; =================================
display_text:
	push ds
	mov b displaycol,1eh
	mov fs,ds
	mov si,crsr_addr
	;
n2:     mov ax,fs:seg_text      ;Aktuelle Textposition holen
	mov ds,ax
	;
	mov ax,fs:actual_line
	mov fs:tmpvar2,ax
	push ax
	mov cx,fs:screenheight  ;Hîhe des Textfensters
	mov al,fs:crsry
	xor ah,ah
	mov fs:tmpvar,ax
	push ax
	sub cl,al
	shl ax,5
	mov bx,ax
	shl ax,2
	add ax,bx
	push ax
	add ax,160
	mov w fs:displaypos,ax  ;An den Anfang des Textscreens
	;
l1:     cmp si,fs:filesize
	jb >l4
	push cx
	mov cx,fs:screenwidth   ;Nix mehr im Speicher, leere Zeile
	call finish_line
	pop cx
	jmp >l3
	;
l4:     call check_line
	if c call display_line       ;Zeile darstellen
l3:     inc w fs:tmpvar
	inc w fs:tmpvar2
	call display_errmsg
	mov b fs:displaymode,0
	mov ax,w[fs:tmpvar]
	cmp ax,fs:screenheight
	jnae l1
	;
	fs:pop w displaypos
	pop ax
	dec ax
	fs:mov tmpvar,ax
	pop ax
	dec ax
	fs:mov tmpvar2,ax
	fs:mov cl,crsry
	xor ch,ch
	jcxz >l2
	fs:mov ax,crsr_addr     ;Adresse der Cursorzeile holen
	fs:mov [tmpvar2+2],ax
l1:     fs:push w tmpvar        ;Bildschirmzeile
	fs:push w tmpvar2
	fs:push w displaypos
	fs:sub w displaypos,160
	fs:inc w tmpvar2
	call display_errmsg
	fs:pop w displaypos
	fs:pop w tmpvar2
	fs:pop w tmpvar
	jnc >f0                 ;Keine Fehlermeldung
	fs:sub w displaypos,320
	fs:sub w tmpvar,2
f0:     mov bx,ofs tmpvar2+2     ;1 Zeile zurÅck
	call get_last_line
	fs:push w displaypos    ;Bildschirmposition retten
	fs:mov si,[tmpvar2+2]   ;Zeile anzeigen
	call check_line
	call display_line
	fs:dec w tmpvar
	fs:dec w tmpvar2
	pop ax
	sub ax,160
	mov fs:displaypos,ax
	fs:cmp w tmpvar,0
	jge l1

l2:     mov ah,2                ;Cursorposition setzen
	xor bh,bh
	mov dl,fs:crsrx
	mov dh,fs:crsry
	inc dh
	int 10h
	pop ds
	ret

; =========================
; >>> read source block <<<
; =========================
read_textblock:
	mov ds,fs
	push bx,cx,dx
l2:     mov bx,fs:filehandle
	mov ah,3fh              ;Block einlesen
	mov cx,0ff00h
	push ds
	mov ds,w fs:seg_text
	xor dx,dx
	int 21h
	pop ds
	
l4:     mov fs:filesize,ax
	pop dx,cx,bx
	ret

draw_menu:
	mov ax,fs
	mov ds,ax
	mov es,ax
	xor ax,ax
	mov displaypos,ax
	mov si,mnu_main
	mov di,mnu_map
	xor bx,bx
	xor dx,dx
	xor bp,bp
l0:     cmp b[si],0
	je >e1
	mov al,displaypos
	stosb
l1:     mov b displaycol,70h
	lodsb
	cmp al,1
	je >l2
	cmp al,"*"
	jne >k1
	mov b displaycol,74h
	lodsb                   ;Controlcode holen
	mov ah,al
	cmp al,"a"       ;in Grossbuchstaben umwandeln
	if nb sub al,"a"-"A"
	stosb                   ;Buchstaben in Liste eintragen
	mov al,ah
k1:     call display_char
	inc dx
	jmp l1
	;
l2:     mov ax,si               ;Aktuelle Adresse in Tabelle schreiben
	stosw
	xchg di,si              ;Nach dem Ende des Untermenues suchen
	mov cx,-1
	xor al,al
	rep scasb
	xchg si,di
	mov al,179              ;Trennungslinie
	call display_char
	inc dx
	inc bx
	jmp l0
e1:     mov hotkeys,bx   ;Anzahl der Menuepunkte vermerken
	mov b displaycol,08fh
	sub w displaypos,2
	mov al," "
	call display_char2
	mov cx,80
	sub cx,dx
	mov si,textname
	call show_line
	ret

; ==================
; >>> Untermenue <<<
; ==================
menu:   mov ax,fs
	mov ds,ax
	mov es,ax
	pusha
	call buffer2line
	show_stat stat_sub
	popa
	mov b pagepos,0
m1:     pusha
	mov di,si
	xor dx,dx
	push bx
	xor bx,bx
l1:     cmp b[di],0
	je >e1
	mov cx,-1
	mov al,1
	rep scasb
	neg cx
	inc bx
	add di,2                ;Adresse der Menueroutine Åberspringen
	cmp dx,cx
	ja l1
	mov dx,cx
	jmp l1
	;
e1:     dec bx
	mov pagesize,bx
	pop bx
	dec dx
	shr bx,1
	mov ax,80
	sub ax,dx
	cmp bx,ax
	jbe >z1
	mov bx,ax
z1:     shl bx,1
	add bx,160
	mov displaypos,bx
	mov di,mnu_map
	xor bl,bl                       ;ZeilenzÑhler
l1:     cmp b[si],0
	je >e1
	push w displaypos
	mov cx,dx
	mov b displaycol,70h
	mov al," "
	call set_pagecol
	call display_char
l2:     mov b displaycol,70h
	lodsb
	cmp al,1
	je >e2
	cmp al,"*"
	jne haeh
	mov b displaycol,74h
	lodsb                   ;Controlcode holen
	call capital
	stosw                   ;Buchstaben in Liste eintragen
	mov al,ah
haeh:   call set_pagecol
	call display_char
	dec cx
	jmp l2
	;
e2:     lodsw                   ;Adresse der Menueroutine in die Liste
	stosw                   ;Åbertragen.
	dec cx
	mov al," "
	call set_pagecol
l5:     call display_char
	loop l5
	or bl,bl
	je >z2
	push es,bx
	mov es,0b800h
	mov bx,displaypos
	and b[es:bx+1],7
	pop bx,es
z2:     pop ax
	add ax,160
	mov displaypos,ax
	inc bl
	jmp l1
e1:     ;
	mov cx,dx               ;Unteren Schatten zeichnen
	push es,bx
	mov es,0b800h
	mov bx,displaypos
	add bx,3
z1:     and b[es:bx],7
	add bx,2
	loop z1
	pop bx,es
	;
l0:     xor ax,ax
	int 16h
	mov bl,pagepos
	;
	cmp ah,crsr_down
	jne >l1
	cmp pagesize,bl
	jne >l3
	mov b pagepos,0
	jmp >l2
l3:     inc b pagepos
	jmp >l2
	;
l1:     cmp ah,crsr_up
	jne >l1
	or bl,bl
	jne >l3
	mov al,pagesize
	mov pagepos,al
	jmp >l2
	;
l3:     dec b pagepos
	jmp >l2
	;
l1:     cmp ah,_enter
	jne >l1
	call display_text
	mov bx,pagepos
	shl bx,2
	call w[bx+mnu_map+2]
	mov ds,fs       ;debug: war's das?
	mov es,fs
	jmp >e1

l1:     call capital
	mov cx,pagesize
	inc cx
	mov bx,mnu_map
l4:     cmp [bx],al
	je >l3
	add bx,4
	loop l4
	jmp >e1
l3:     add bx,2
	push bx
	call display_text
	pop bx
	call w[bx]
	
e1:     pushf
	call line2buffer
	popf
	popa
	ret
	;
l2:     popa
	jmp m1

set_pagecol:
	cmp pagepos,bl
	jne >f1
	push ax
	mov al,displaycol
	mov ah,al
	and ah,15
	cmp ah,4
	jne >l1
	mov al,0ch
	jmp >l2
l1:     mov al,0eh ;ror al,4
l2:     mov displaycol,al
	pop ax
f1:     ret

; ========================================
; >>> Wandelt AL in Grossbuchstaben um <<<
; ========================================
capital:mov ah,al
	cmp al,"a"       ;in Grossbuchstaben umwandeln
	jb >l4
	cmp al,"z"
	ja >l4
	sub al,"a"-"A"
l4:     ret

; ====================================
; >>> Speichert die aktuelle Datei <<<
; ====================================
save:   show_stat stat_save_file
	mov ax,seg_text
	mov es,ax
	xor di,di
	mov ecx,filesize
	mov dx,textname
	call store_file
	mov b flag_change,0
	clc
	ret

; PrÅft, ob die aktuelle Datei bereits gespeichert wurde und fragt den User.
; Out:  CC - Datei wurde gespeichert
;       CS - Operation wurde abgebrochen
ask2save:
	pusha
	test b flag_change,1
	jz >l3                  ;Datei ist unverÑndert, weiter...
	;
	mov bx,5                ;Window aufbauen
	mov dx,5
	mov cx,5 by 60
	mov si,textname
	call fill_window
	mov si,txt_not_saved
	mov b displaycol,70h
	mov w displaypos,7*160+38
	call print_string
	mov w displaypos,8*160+56
	mov si,txt_query
	call print_string
	;
l2:     xor ax,ax               ;Auf Tastendruck warten
	int 16h
	cmp ah,_enter
	je >l1
	cmp al,"y"
	je >l1
	cmp al,"Y"
	je >l1
	cmp al,"n"
	je >l3
	cmp al,"N"
	je >l3
	cmp ah,1
	jne l2
	popa            ;Operation abbrechen
	stc
	ret
l1:     call save       ;Datei speichern
l3:     popa
	clc
	ret


; ==============================
; >>> Menuepunkt: Neue Datei <<<
; ==============================
new_file:
	call ask2save
	jc >l1
	call create_new_file
l1:     clc
	ret

; >>> Erstellt neue Datei <<<
create_new_file:
	mov ax,fs
	mov ds,ax
	mov es,seg_text               ;Textpuffer initialisieren
	mov w[es:0],lf by cr
	mov w filesize,2
	call reset_screen
	xor ebx,ebx
	mov ax,fs
	mov es,ax
	mov si,NME_noname
	mov di,textname
	mov cx,11
	rep movsb
	mov b flag_change,0
	ret

; ==============================
; >>> Menupunkt: Datei laden <<<
; ==============================
load_text:
	call ask2save                   ;PrÅfen, ob alte Datei gespeichert wurde
	jc >l1
	show_stat stat_fsb              ;Hilfe einblenden
	fselector txt_load_text         ;File-Selector-Box starten
	if nc call load
	clc
	ret

load:   show_stat stat_load_text           ;Anzeigen, da· geladen wird
	mov dx,textname                 ;Datei laden
	call read_file
	mov b flag_change,0
	mov w[total_errors],0
l1:     ret

; >>> Menuepunkt: Datei einfÅgen <<<
insert_file:
	show_stat stat_fsb
	fselector txt_insert_file
	if nc show_stat stat_insert_file
	clc
	ret

; >>> Menuepunkt: Datei anhÑngen <<<
appendfile:
	show_stat stat_fsb
	fselector txt_append_file
	if nc show_stat stat_append_file
	clc
	ret

; >>> Meuepunkt: Datei speichern <<<
save_file:
	call save
	clc
	ret

autosave:
	pushad
	push ds,es
	mov ds,fs
	mov es,40h
	mov eax,[es:6ch]
	mov edx,eax
	sub eax,autosave_timer
	jc >d1
	cmp eax,autosave_delay
	ja >d1
n1:     pop es,ds
	popad
	ret
d1:     mov autosave_timer,edx
	cmp b flag_change,0
	je n1
	show_stat stat_autosave
	call buffer2line
	call line2buffer
	mov ax,seg_text
	mov es,ax
	xor di,di
	mov ecx,filesize
	mov dx,nme_autosave
	call store_file
	show_stat stat_autosaved
	pop es,ds
	popad
	ret
	

; >>> Menuepunkt: Speichern als... <<<
save_as:show_stat stat_fsb
	fselector txt_save_file
	if nc call save
	clc
	ret

save_all:
	ret

; >>> Menuepunkt: Datei komprimieren <<<
compress_file:
	show_stat stat_compress_file
	mov w crsr_addr,0
	mov b flag_change,1
l1:     call line2buffer
	call buffer2line
	mov bx,ofs crsr_addr
	call get_next_line
	jnc l1
	call reset_screen
	call line2buffer
	clc
	ret

; >>> Menuepunkt: Datei drucken <<<
print:  ret

sven:   ret

; >>> Menuepunkt: Programm beenden <<<
quit:   call ask2save
	jnc ende2
	ret

; >>> Menuepunkte: Block ausschneiden/lîschen <<<
cut_block:
	call copy_block
	mov cx,filesize
	sub cx,bx
	jcxz >l2
	mov b flag_change,1
	pusha
l4:     cmp dx,crsr_addr
	je >l5
	ja >l3
	call cursor_up
	jmp l4
l3:     call cursor_down
	jmp l4
	;
l5:     popa
	push ds
	mov ax,seg_text
	mov es,ax
	mov ds,ax
	mov si,bx
	mov di,dx
	mov ax,si
	sub ax,di
	sub fs:filesize,ax
	rep movsb
	pop ds
	xor ax,ax
	mov marked_start,ax
	mov marked_end,ax
l2:     clc
	ret

; >>> Menuepunkt: Block kopieren <<<
copy_block:
	push ds
	mov ds,fs:seg_text
	mov es,fs:seg_buffer
	mov dx,fs:marked_start
	mov cx,fs:marked_end
	cmp cx,dx
	ja >l1
	mov b fs:flag_change,1
	xchg cx,dx
l1:     xor di,di
	mov bx,cx
	sub cx,dx
	mov si,dx
	jcxz >l2
	rep movsb
l2:     mov fs:buffersize,di
	pop ds
	clc
	ret

; >>> Menuepunkt: Block einfÅgen <<<
insert_block:
	push ds
	mov ax,seg_text
	mov ds,ax
	mov es,ax
	mov cx,fs:filesize
	sub cx,fs:crsr_addr
	jcxz >l1
	mov b fs:flag_change,1 ;Vermerken, dass der Text verÑndert wurde
	mov si,fs:crsr_addr
	mov di,si
	add di,fs:buffersize
	dec cx
	add si,cx
	add di,cx
	inc cx
	std
	rep movsb
	cld
	mov ds,fs:seg_buffer
	xor si,si
	mov di,fs:crsr_addr
	mov cx,fs:buffersize
	add fs:filesize,cx
	jcxz >l1
	rep movsb
l1:     pop ds
o2:     clc
	ret

; >>> Menuepunkt: Suchen nach... <<<
search: mov bx,5
	mov dx,5
	mov cx,7 by 70
	mov si,txt_search
	call fill_window
	mov b flag_input,1
	call reset_line

l2:     input_line 6,7,buf_search
	jc >l1                  ;Keine BestÑtigung des Users
	;
	show_stat stat_search   ;Status ausgeben
	call reset_screen
	;
s1:     mov cx,[buf_search+2]    ;Eingabe in Grossbuchstaben umwandeln
	sub cx,2
	jcxz o2
	mov si,buf_search+4
	mov di,si
o3:     lodsb
	call capital
	stosb
	loop o3
	;
l3:     call line2buffer        ;Zeile einlesen
	mov cx,bufferlen        ;PrÅfen, wieviele Stellen verglichen werden mÅssen
	add cx,2
	sub cx,[buf_search+2]
	jz >l5                  ;Wort kann nicht in aktueller Zeile
	jc >l5                  ;untergebracht sein
	;
	push cx
	mov si,buffer           ;Pufferinhalt in Grossbuchstaben umwandeln
	mov di,buffer
	mov cx,bufferlen
	sub cx,2
o1:     lodsb
	call capital
	stosb
	loop o1
	pop cx
	;
	mov si,buffer           ;An den Anfang der Zeile springen
	call reset_line
l6:     push si,cx
	mov di,buf_search+4
	mov cx,[buf_search+2]
	sub cx,2
	repe cmpsb              ;Strings vergleichen
	jz >l4                  ;String gefunden
	call cursor_right
	pop cx,si
	inc si
	loop l6                 ;Komplette Zeile absuchen
l5:     call cursor_down
	jnc l3
	show_stat stat_not_found
	call reset_screen
	stc
	ret
	;
l4:     pop cx,si               ;String gefunden
	jmp >l5
	;
l1:     cmp ah,1
	jne l2
l5:     clc
	ret

reset_screen:
	xor ax,ax
	mov crsr_addr,ax
	mov screen_addr,ax
	mov crsry,ax
	inc ax
	mov actual_line,ax
	mov disp_line,ax
reset_line:
	xor ax,ax
	mov crsrx,ax
	mov crsr_pos,ax
	mov coloffset,ax
	ret

init_input_buffer:
	stosw
	mov ax,2
	stosw
	mov ax,lf by cr
	stosw
	ret

search_again:
	show_stat stat_search_again
	jmp s1

exchange:
go_to:
find_error:
jump_to:
	ret

assemble:
	cmp b flag_priname,1    ;PrimÑre Datei angegeben ?
	je >l2
	call set_prifile        ;Nein, fragen.
	jc >l1
	jmp >l3
l2:     call ask2save           ;Ja, fragen, ob Datei gespeichert werden soll.
	jc >l1
	call line2buffer
l3:     mov b[flag_ed],1
	mov ax,3
	int 10h
	mov si,priname          ;Projektnamen Åbergeben
	mov di,sourcename
l2:     lodsb
	stosb
	or al,al
	jne l2
	call cr_com             ;Assembler starten
	call keywait            ;Auf Tastendruck warten
	call ed_init_screen     ;Bidschirm restaurieren
	show_stat stat_assembled
	stc                     ;Statuszeile beibehalten
	ret
l1:     clc
	ret

run_prg:call ask2save
	jc l1
	call assemble
;        jc l1
	cmp w[total_errors],0
	jne l1
	mov ax,3
	int 10h
	mov si,txt_running
	call asciiz
	mov si,destname
	call asciiz
;        call cr
	mov w cs_adr,destname
	mov cs_seg,fs
	xor al,al
	mov si,txt_nul
	call exec
	sti
	call keywait
	call ed_init_screen
	clc
	ret

help:   ret

; ====================
; >>> FÅllt FlÑche <<<
; ====================
; Position BX/DX, Grîsse CL/CH
fill_window:
	mov ax,dx
	shl ax,7
	shl dx,5
	add ax,dx
	shl bx,1
	add ax,bx
	mov displaypos,ax
	;
	mov es,ds
	mov b displaycol,8fh   ;Fenstertitel zeichnen
	push cx,displaypos,cx
	mov cx,-1               ;StringlÑnge messen
	xor al,al
	mov di,si
	repne scasb
	neg cx
	mov dx,cx               ;Ergebnis nach DX
	pop cx
	xor ch,ch
	mov ax,cx
	sub al,dl
	shr al,1
	adc al,0
	sub cx,ax
	push cx
	mov cx,ax
	mov al," "
s1:     call display_char2
	loop s1
	pop cx
	call show_line
	pop ax
	add ax,160
	mov displaypos,ax
	pop cx
	dec ch
	;
	mov b displaycol,70h
l1:     push displaypos
	push cx
	mov al," "
l2:     call display_char2
	dec cl
	jnz l2
	push es,bx
	mov es,0b800h
	mov bx,displaypos
	and b[es:bx+1],7
	pop bx,es,cx,ax
	add ax,160
	mov displaypos,ax
	dec ch
	jnz l1
	push bx,es
	mov es,0b800h
	mov bx,ax
	add bx,3
	xor ch,ch
l1:     and b[es:bx],7
	add bx,2
	loop l1
	pop es,bx
	ret

_show_stat:
	mov ax,stat_pos
	mov w displaypos,ax
	mov b displaycol,08fh
	xor bp,bp
	mov al," "
	call display_char
	mov cx,79
show_line:                      ;Anzeige einer Zeile
	call print_string
	call finish_line
	ret
	;
finish_line:
	jcxz >l4
	mov al," "
l3:     call display_char
	loop l3
l4:     ret

clear_stat:
	mov cx,80
	mov ax,stat_pos
	mov w displaypos,ax
	mov b displaycol,08fh
	mov al," "
l3:     call display_char
	loop l3
	ret

print_string:
l1:     lodsb
	or al,al
	je >l2
	cmp al,cr
	je >l2
	cmp al,lf
	je >l2
	call display_char2
	loop l1
l2:     ret

; ==============================
; >>> Eingabezeile behandlen <<<
; ==============================
; DS:SI=Eingabepuffer DX=Position der Zeile
; Out:  CC=Name wurde Åbergeben
;       CS=Eingabe wurde nicht erkannt.
_input_line:
	mov b input_mode,inputmode
	mov al,8fh
	cmp b flag_input,1
	jne >l1
	mov al,1fh
l1:     mov displaycol,al
	push crsrx,crsry,crsr_pos
	xor ax,ax
	mov w crsrx,ax
	mov w crsr_pos,ax
	push maxbufferlen
	lodsw                   ;Maximale Grîsse des Puffers lesen
	mov maxbufferlen,ax
	mov buffer_origin,si
	lodsw                   ;Aktuelle Grîsse des Textes im Puffer
	mov bufferlen,ax
	mov cx,ax               ;Zeile in den Puffer Åbertragen
	mov di,buffer
	rep movsb
	;
	xor ax,ax               ;Position auf dem Bildschirm berechnen
	mov al,dh
	mov bx,ax
	shl ax,7
	shl bx,5
	add ax,bx
	xor bx,bx
	mov bl,dl
	shl bx,1
	add ax,bx
	mov displaypos,ax
	jmp >l20

m1:     mov cx,maxbufferlen
	sub cx,2                ;cr+lf auslassen
	xor bp,bp
	mov si,buffer
	push displaypos
	mov al,displaycol
	push ax
	call print_string
	mov b displaycol,8fh
	call finish_line
	pop ax
	mov b displaycol,al
	pop displaypos

	push dx                 ;Cursorposition retten
	mov al,crsrx
	add dl,al
	add al,2
	cmp al,maxbufferlen
	jne >h1
	dec dl
h1:     mov ah,2                ;Cursorposition DX setzen
	xor bh,bh
	int 10h
	pop dx
	xor ax,ax
	int 16h
	;
	cmp ah,1
	je >l1
	cmp ah,_enter
	je >l10
	;
	cmp ah,crsr_left
	jne >l2
	push dx
	call cursor_left
	jmp >l3
	;
l2:     cmp ah,crsr_right
	jne >l2
	push dx
	call cursor_right
	jmp >l3
	;
l2:     cmp ah,pos1
	jne >l2
	push dx
	call go_pos1
	jmp >l3
	;
l2:     cmp ah,endpos
	jne >l2
l20:    push dx
	call go_end
	pop dx 
	jmp m1
	;
l2:     cmp ah,backspace
	jne >l2
	push dx
	call cursor_left
l4:     call delete_char
	jmp >l3
	;
l2:     cmp ah,del
	jne >l2
	push dx
	jmp l4
	;
l2:     or al,al
	je >l1 ;m1              ;kein normaler Buchstabe
	cmp al,";"              ;PASSWORD-UnterstÅtzung bei OpenDOS
	je >l60
	cmp b flag_input,1
	jne >l60
	mov w[buffer],lf by cr  ;Eingabezeile lîschen
	mov w bufferlen,2
	mov w crsr_pos,0
	mov w crsrx,0
l60:    mov ah,al
	push dx
	mov b flag_directory,0
	call insert_char
l3:     pop dx
	mov b flag_input,0
	mov b displaycol,8fh
	jmp m1
	;
l1:     stc
	jmp >l11
	;
l10:    mov si,buffer           ;Neuen Pufferinhalt Åbertragen
	mov di,buffer_origin
	mov ax,bufferlen
	stosw
	mov cx,ax
	rep movsb
	clc
l11:    pop maxbufferlen,crsr_pos,crsry,crsrx
	mov b input_mode,textmode
	ret

; ============================
; >>> Speichert eine Datei <<<
; ============================
;DS:DX=Dateiname
;ES:DI=Quellesegment
;ECX  =LÑnge des Datenblocks
store_file:
	push ecx
	mov ah,3ch      ;Datei erstellen
	xor cx,cx
	int 21h
	mov bx,ax
	pop ecx
savetext3:
	push ecx
	cmp ecx,8000h   ;Ist mehr als 32k da ?
	jb savetext2
	mov cx,8000h    ;Ja,32k Block speichern
savetext2:
	mov ah,40h      ;Block in Datei schreiben
	push ds
	mov ds,es
	mov dx,di
	int 21h
	pop ds,ecx
	mov ax,es
	add ax,800h
	mov es,ax
	sub ecx,8000h
	jnc savetext3   ;Weiter wenn noch was da ist

	mov ah,3eh      ;Sonst Datei schliessen.
	int 21h
	ret

; =======================
; >>> Liest Datei ein <<<
; =======================
read_file:
	pusha
	mov ax,3d00h            ;Datei îffnen
	int 21h
	jc >l1
	mov [filehandle],ax
	call read_textblock   ;Datei einlesen
	mov bx,filehandle       ;Datei schliessen
	mov ah,3eh
	int 21h
;        call open_file          ;Datei cachen...
;        mov es,w[seg_text]      ;...und in den Puffer laden
;        call readsource
;        mov ax,w[inputsize]
;        mov w[filesize],ax
	call reset_screen
	mov crsr_line,ax
	mov marked_end,ax
	mov marked_start,ax
l1:     popa
	ret

; =========================================
; >>> Wandelt Zeile in ASCIIZ-String um <<<
; =========================================
cr2asciiz:
	lodsb
	cmp al," "
	jb >l1
	stosb
	jmp cr2asciiz
l1:     xor al,al
	stosb
	ret

; =========================
; >>> File selector box <<<
; =========================
; DS:SI=Name des Titels
; Out:  CC - Dateiname wurde Åbergeben
;       CS _Auswahl wurde abgebrochen.
_fsb:   pusha
	call display_text
	popa
	mov b dirsize,16
	mov bx,10
	mov dx,2
	mov cx,21 by 38
	call fill_window
	mov w displaypos,4*160+62
	mov si,TXT_filename
	mov b displaycol,70h
	call print_string
	mov w displaypos,20*160+22
	mov si,TXT_directory
	call print_string
	;
l5:     mov si,NME_allfiles     ;Aktuelles Directory einlesen.
l6:     call initfilebox
	call show_dir
l0:     mov si,buf_currentdir2  ;Verzeichnis anzeigen
	mov w displaypos,21*160+22
	mov b displaycol,70h
	mov cx,31
	call show_line
	input_line 31,5,blankline       ;Eingabe pollen
	jc >l2                  ;Steuertaste benutzt...
	cmp b flag_directory,1  ;Eingabe wurde bestÑtigt. Directory ?
	jne >l1                 ;Nein...
	mov dx,dirpos           ;Ins angewÑhlte Directory wechseln
	add dx,dirstart
	shl dx,4
	add dx,filelist+1
	mov ah,3bh
	int 21h
	xor ax,ax
	mov w dirstart,ax
	mov w dirpos,ax
	mov b flag_input,1      ;Inhalt als temporÑr markieren
	jmp l5                  ;Neues Directory einlesen
	;
l1:     mov si,blankline+4
	mov di,textname
	call cr2asciiz
	mov si,textname         ;öbergebenen Namen auf Wildcards prÅfen
l7:     lodsb
	or al,al
	je >l8
	cmp al,"*"
	je >l9
	cmp al,"?"
	jne l7
l9:     mov si,textname         ;Directory neu einlesen
	xor ax,ax
	mov w dirstart,ax
	mov w dirpos,ax
	mov b flag_input,1     ;Inhalt als temporÑr markieren
	jmp l6
	;
l8:     clc                     ;Name wurde Åbergeben.
	ret
l3:     stc                     ;Wahl wurde abgebrochen
	ret
	;
l2:     cmp ah,1        ;Wurde ESC gedrÅckt ?
	je l3           ;Ja, abbrechen
	call filebox    ;Dateiauswahlbalken steuern
	jmp l0

; =========================
; >>> File selector box <<<
; =========================
initfilebox:
	push si
	mov es,fs
	;
	mov ah,1ah              ;DTA einrichten
	mov dx,DTA
	int 21h
	;
	mov ah,47h              ;Aktuellen Directorynamen holen.
	xor dl,dl
	mov si,BUF_currentDir2
	int 21h
	;
	pop si
	mov di,filelist         ;Directory auslesen
	call ReadDir
	call file2input         ;Aktuellen Dateinamen in die
				;Eingabezeile Åbertragen.
	ret

; >>> Steuerung des Dateiauswahlbalkens in der Fileselectorbox
filebox:push dx
l0:     cmp ah,crsr_up
	jne >l2
	call fsb_up
	jmp >l3
	;
l2:     cmp ah,crsr_down
	jne >l2
	call fsb_down
	jmp >l3
	;
l2:     cmp ah,pg_up
	jne >l2
	mov cx,dirsize
l1:     call fsb_up
	loop l1
	jmp >l3
	;
l2:     cmp ah,pg_down
	jne >l3
	mov cx,dirsize
l1:     call fsb_down
	loop l1
l3:     call show_dir
	pop dx
	ret
	
fsb_up: push cx
	mov bx,dirpos
	mov dx,dirstart
	or bx,bx
	jne >l3
	or dx,dx
	je >l10
	dec w dirstart
	jmp >l12
l3:     dec w dirpos
	jmp >l12

fsb_down:
	push cx
	mov bx,dirpos
	mov dx,dirstart
	mov ax,bx
	add ax,dx
	inc ax
	cmp ax,files
	je >l10
	inc bx
	cmp bx,dirsize
	je >l3
	inc w dirpos
	jmp >l12
l3:     inc w dirstart
l12:    call file2input
l10:    pop cx
	ret

; =====================================================
; >>> Kopiert Eintrag aus Liste in die Eingabezeile <<<
; =====================================================
file2input:
	mov di,blankline+4
	mov si,dirpos
	add si,dirstart
	shl si,4
	add si,filelist
	lodsb
	xor ah,ah
	test al,10h
	jz >u1
	inc ah
u1:     mov flag_directory,ah
	xor cx,cx
l13:    lodsb
	or al,al
	je >l14
	stosb
	inc cx
	jmp l13
l14:    mov ax,lf by cr
	stosw
	add cx,2
	mov w[blankline+2],cx
	mov b flag_input,1
l2:     ret

; ==========================
; >>> Zeigt Directory an <<<
; ==========================
show_dir:
	mov w displaypos,160*4+22
	mov si,dirstart         ;Startzeile des Directories
	shl si,4
	add si,filelist
	mov cx,dirsize
	xor bx,bx               ;ZeilenzÑhler
l1:     push displaypos,si,cx
	xor ah,ah
	cmp bx,dirpos
	jne >l7
	mov ah,88h
l7:     lodsb
	mov dl,al
	test al,_hid ;Dateien mit Fremdattributen rot hervorheben
	jz >u1
	or ah,4
	jmp >l6
u1:     test al,10h
	jz >l5
	or ah,0eh
	jmp >l6
l5:     or ah,7
l6:     mov displaycol,ah
	mov al," "
	call display_char2
	;
	mov dh,1
	mov cx,13
	mov ax,bx
	add ax,dirstart
	cmp ax,files
	jae >l8
	dec cx
	xor dh,dh
	;
l2:     lodsb
	cmp al,255
	if e mov al,"?"
	or al,al
	je >l3
	call display_char2
	loop l2
l3:     inc cx
l8:     mov al," "
l4:     call display_char2
	loop l4
	;                       ;Attribute anzeigen
	or dh,dh
	jne >o3
	test dl,_dir
	jz >o1
	mov si,txt_dir
	call print_string
	jmp >o2
o1:     mov al,"-"
	test dl,_arch
	jz >o1
	mov al,"a"
o1:     call display_char2
	mov al,"-"
	test dl,_read
	jz >o1
	mov al,"r"
o1:     call display_char2
	mov al,"-"
	test dl,_sys
	jz >o1
	mov al,"s"
o1:     call display_char2
	mov al,"-"
	test dl,_hid
	jz >o1
	mov al,"h"
o1:     call display_char2
	mov al," "
	call display_char2
	jmp >o2
	;
o3:     mov cx,5
	call finish_line
	;
o2:     inc bx
	pop cx,si,displaypos
	mov ax,160
	add w displaypos,ax
	add si,16
	loop >l10
	ret
l10:    jmp l1

; =======================
; >>> Liest Directory <<<
; =======================
ReadDir:mov w files,0
	mov ah,4eh              ;get first dir entry
	mov cx,31
	mov dx,si
	int 21h
	cmp al,12h
	je >l7
l2:     mov si,DTA+1eh
	cmp w[si],0 by "."      ;"." ignorieren
	je >l3
	mov al,[dta+15h]        ;Dateiattribut
	stosb
	mov cx,14
l1:     movsb
	sub cx,2
l5:     lodsb
	cmp al,"A"
	jb >l4
	cmp al,"Z"
	ja >l4
	add al,"a"-"A"
l4:     stosb
	cmp al,"."
	je l1
	loop l5
	xor al,al
	stosb
	inc di
	inc w files            ;Anzahl der Files+1
l3:     mov ah,4fh
	int 21h
	jnc l2
l7:     ret

; ======================
; >>> Gets file name <<<
; ======================
GetNamex:xor cx,cx
	cmp b[si],20h
	jb >l1
l2:     cmp b[si],21h
	jnb >l1
	inc si
	jmp l2
	;
l1:     lodsb
	cmp al,21h
	jb >l2
	stosb
	inc cx
	jmp l1
l2:     mov b[di],0
	ret

ed_init_screen:
	mov ax,3
	int 10h
	mov ax,1003h            ;Blinken aus
	xor bl,bl
	int 10h
	mov w stat_pos,160*24
	ret

keywait:mov si,txt_keywait
	call asciiz
	xor ax,ax
	int 16h
	ret

getkey: call autosave
	mov ah,1                ;PrÅfen, ob Taste anliegt
	int 16h
	jz getkey               ;Nein, nochmal
	xor ax,ax               ;Ja,Zeichen auslesen
	int 16h
	ret

	.code

; ---------------------------------------------------------------------------

	nop
show_clipboard:
single_step:
trace:
go_here:
step_into:
reset_session:
win_registers:
win_watches:
win_dump:
win_srclist:
cfg_edit:
cfg_asm:
cfg_deb:
cfg_opt:
cfg_save:
cfg_pathes:
library:
notes:
list_modules:
nowt:   ret

set_prifile:
	call ask2save
	jc >l4
	show_stat stat_fsb
	fselector txt_primary_file
	jc >l4
	call load
	jc >l4
	mov b flag_priname,1
	mov es,fs
	mov si,textname
	mov di,primary_name
	mov bx,priname
	mov cx,12
l1:     lodsb
	mov [bx],al
	inc bx
	or al,al
	je >l2
	stosb
	loop l1
	mov di,bx
l5:     lodsb           ;Rest des Namens Åbertragen.
	stosb
	or al,al
	jne l5
l2:     jcxz >l3
	mov al,"."
	rep stosb
l3:     clc
l4:     ret

printdecword:
	push bx,dx,bp
	mov bp,cx
	mov bx,10
	xor cx,cx
p2:     xor dx,dx
	div bx
	push dx
	inc cx
	or ax,ax
	jne p2
p3:     pop dx
	or bp,bp
	jz >n1
	mov al,dl
	add al,"0"
	push bp
	xor bp,bp
	call display_char
	pop bp
	dec bp
	loop p3
	mov cx,bp
	pop bp,dx,bx
	ret

	include"ide\editfunc.inc"
	include"..\lib\io\execdos.inc"

#if !_mdltiny
_text   segment para
#else
	.code
#endif

blankline:                      ;Leere Eingabezeile fÅr Fileselectorbox
	dw 18
	dw 2
	db cr,lf,16 dup ?

scn_german:             ;Scancodes der deutschen Tastatur
	db 30,48,46,32,18,33,34,35,23,36,37,38,50,49,24,25,16,19,31,20,22,47,17,45,44,21

item    macro
	db #1
	db 1
	dw #2
	endm
subitem macro
	db #1
	db 1
	endm

; ======================
; >>> Texte & Menues <<<
; ======================
MNU_main:
	subitem " *File "
		item "*New",new_file
		item "*Open...",load_text
		item "*Save",save_file
		item "Save *as...",save_as
;                item "Save a*ll",save_all
;                item "*Insert...",insert_file
;                item "Appen*d...",appendfile
;                item "*Print",print
		item "*Compress",compress_file
;                item "In*fo...",sven
		item "*Quit",quit
		db ?
	subitem " *Edit "
		item "Cu*t block",cut_block
		item "*Copy block",copy_block
		item "*Paste block",insert_block
		item "*Delete block",cut_block
;                item "*Show clipboard ...",show_clipboard
		db ?
	subitem " *Search "
		item "*Search...",search
		item "Search *again",search_again
;                item "*Replace...",exchange
;                item "*Go to line number...",go_to
;                item "*Find program addr...",find_error
;                item "*Jump to [next] error",jump_to
		db ?
	subitem " *Project "
;                item "*Run",run_prg
		item "*Assemble",assemble
;                item "*Notes",notes
;                item "*List",list_modules
		db "*Primary file : ["
primary_name:   db "none........]",1
		dw set_prifile
		db ?
;        subitem " *Debug "
;                item "*Trace",single_step
;                item "*Step",trace
;                item "*Go here",go_here
;                item "Step into *INT",step_into
;                item "*Reset session",reset_session
;                db ?
;        subitem " *Window "
;                item "*Registers",win_registers
;                item "*Watches...",win_watches
;                item "*Dump...",win_dump
;                item "*Source list",win_srclist
;                db ?
;        subitem " *Config "
;                item "*Editor...",cfg_edit
;                item "*Assembler...",cfg_asm
;                item "*Debugger...",cfg_deb
;                item "*Optimization...",cfg_opt
;                item "*Save options...",cfg_save
;                item "*Pathes...",cfg_pathes
;                db ?
;        subitem " *Help "
;                item "*Help on DASM",help
;                item "*Library",library
;                db ?
	db ?

txt_init:       db"TMA macro assembler A#65-980717 IDE - Please wait.",0
; >>> Statuszeilen <<<
stat_standard:  db"TMA A#65-980717 ",179," ALT+[char]=Submenu ",179," ESC=Quit",0
stat_fsb:       db"Cursor keys: ",24,25,"=list / ",17,16,"=line ",179," ENTER=Ok ",179," ESC=Quit",0
stat_sub:       db"Press highlighted key or use cursor keys ",0
stat_load_text: db"Reading file ...",0
stat_save_file: db"Writing file ...",0
stat_insert_file:db"Inserting file ...",0
stat_append_file:db"Appending file ...",0
stat_compress_file:db"Compressing file ...",0
stat_search:    db"Searching ...",0
stat_search_again:db"Searching again ...",0
stat_not_found: db"Nothing found.",0
stat_previous:  db"Didn't quit last session properly - backup loaded.",0
stat_assembled: db"Project compiled.",0 ;" Press SHIFT+TAB to jump to other modules.",0
stat_unknown:   db"Unknown command. Press F1 for more detailed help.",0
stat_autosave:  db'Autosaving file to "AUTOSAVD.ASM" ...',0
stat_autosaved: db'File autosaved to "AUTOSAVD.ASM".',0

txt_lasterr:db" (last one)",0
txt_running:db"Running code ",10,13,0

; Fenstertitel
TXT_load_text:db"Open file",0
TXT_save_file:db"Save file as",0
TXT_insert_file:db"Insert file",0
TXT_append_file:db"Append file",0
TXT_search:db"Search for",0
txt_primary_file:db"Choose primary file",0

; >>> Textfetzen
TXT_not_saved:db"File is not saved. Save now ?",0
TXT_overwrite:db"File already exists. Overwrite ?",0
TXT_query:db"(Y)es/(N)o ?",0
TXT_filename:db"Filename :",0
TXT_directory:db"Directory :",0
TXT_dir:db"<dir>",0
Registered:     db"Next time use DIDE!",10,13
		db"Have a nice DOS! :-)",10,13,"$"
NME_allfiles:db"*.*",0
NME_noname:db"noname.asm",0
NME_backup:db"~session.bak",0
NME_autosave:db"autosavd.asm",0 ;Name for autosaved file
TXT_keywait:db"Press any key ..."
txt_nul:db 0

;Optionen:

;db"Load options"
;db"[x] perform syntax check"
;db"[x] load all files to cache"
;db"[x] pre-assemble project after loading"

;db"Error messages",0
;db"[x] show in source",0
;db"    [x] 1 line",0
;db"    [x] 2 lines",0
;db"[x] show in message window",0
;db"[x] insert into source",0
;db 0
;db"[x] highlight undefined labels",0
;db"[x] highlight unused labels",0

;db"Optimizing"
;db"*#2[x] Align data (CPU dependend)",0
;db"*#2[x] Align procedures on cache boundaries",0
;db 
;db"Tune for",0
;db 0
;db"    [x] 386",0      ;Schneller Befehl an den Anfang einer Schleife
			;Cache=16
;db"    [ ] 486",0      ;cd=1
			;Cache=16
;db"    [ ] 586",0      ;"=32
;db"    [ ] 686",0
;db
;db"    [x] speed"
;db"    [x] size"
;db"    [x] supertight"
;db"[x] unroll loops max. [   ] times",0
;db 0
; Text formatting:
;
; Convert macros without parameters to calls

