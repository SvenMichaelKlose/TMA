OPEN "durch4.inc" FOR OUTPUT AS #1
FOR a = 0 TO 255
PRINT #1, "byt "; INT(a / 4)
NEXT
CLOSE

