OPEN "spdtst.a65" FOR OUTPUT AS #1
PRINT #1, "           sys start"
PRINT #1, "start      ;Hier f„ngt das Programm an"
FOR a = 1 TO 20000
IF INT(RND(1) * 10) = 1 THEN FOR b = 1 TO 4: PRINT #1, CHR$(RND(1) * 24 + 65); : NEXT: PRINT #1, " ";
PRINT #1, "sta $1234,x";
IF INT(RND(1) * 4) = 1 THEN PRINT #1, ";Kommentar" ELSE PRINT #1, ""
NEXT: CLOSE

