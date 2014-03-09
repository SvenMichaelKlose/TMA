SCREEN 1, 0
CONST pi = 3.141592654#
OPEN "slopes.inc" FOR OUTPUT AS #1
PRINT #1, "byt";
FOR a! = 1 TO 720 STEP (720 / 256)
b = a! * (pi / 180)
x = INT(SIN(b) * &H1F) + &H20
REM y = INT(COS(b) * &H40)
PRINT #1, x; ",";
PSET (x, y), 1
NEXT: CLOSE
WHILE INKEY$ = "": WEND

