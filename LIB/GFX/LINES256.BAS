OPEN "lines256.inc" FOR OUTPUT AS #1
FOR a = 0 TO 19
PRINT #1, "DW "; a * 3200;
FOR b = 1 TO 9
PRINT #1, ","; (a * 10 + b) * 320;
NEXT: PRINT #1, ""
NEXT: CLOSE


