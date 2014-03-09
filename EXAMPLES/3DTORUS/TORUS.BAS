CONST pi = 3.141592654# / 180
r1 = 600
r2 = 130
x = 0
y = 0
s1 = 20
s2 = 20

OPEN "torus.vec" FOR OUTPUT AS #1
PRINT #1, "pointlist:"
PRINT #1, "dw "; 360 / s1 * 360 / s2
p = 0
FOR a = 0 TO 359 STEP s1
        FOR b = 0 TO 359 STEP s2
        PRINT #1, "dw ";
        PRINT #1, INT(SIN(a * pi) * (r1 + (SIN(b * pi) * r2)) + x); ",";
        PRINT #1, INT(COS(a * pi) * (r1 + (SIN(b * pi) * r2)) + y); ",";
        PRINT #1, INT(COS(b * pi) * r2)
        p = p + 1
        NEXT
NEXT

PRINT #1, "planelist:"
PRINT #1, "dw "; p * 2
FOR a = 0 TO p - 1

PRINT #1, "dw "; a; ",";

b = a
IF b >= (p - (360 / s2)) THEN b = b - p
PRINT #1, b + (360 / s2); ",";

b = a + 1
IF b >= p THEN b = b - p
PRINT #1, b; ",";

PRINT #1, a MOD 16; ",0"

PRINT #1, "dw ";
b = a + (360 / s2)
IF b >= p THEN b = b - p
PRINT #1, b; ",";

b = a + (360 / s2) + 1
IF b >= p THEN b = b - p
PRINT #1, b; ",";

b = a + 1
IF b >= p THEN b = b - p
PRINT #1, b; ",";

PRINT #1, a MOD 16; ",0"

NEXT

CLOSE




