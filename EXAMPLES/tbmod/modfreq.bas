DIM n(14)
PRINT
FOR b = 1 TO 14
READ a: n(b) = INT(a / 2): IF INT(a / 2) <> a / 2 THEN n(b) = n(b) + 1
NEXT
d = 1
OPEN "modfreq.inc" FOR OUTPUT AS #1
FOR a = 1 TO 5
FOR b = 1 TO 14
PRINT #1, INT(n(b) / d); ",";
NEXT b: d = d * 2: NEXT: CLOSE

DATA &h358,&h2fb,&h2a7,&h281,&h23b,&h1fd,&h1c5
DATA &h328,&h2d0,0,&h25d,&h21b,&h1e0,0

