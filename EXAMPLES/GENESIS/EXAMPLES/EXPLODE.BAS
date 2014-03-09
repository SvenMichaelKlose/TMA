OPEN "explode.ext" FOR OUTPUT AS #1
FOR c = 1 TO 16
FOR a = 1 TO 21
PRINT #1, "BYT"; 255 - INT(RND(1) * 12); ","; 255 - INT(RND(1) * 12); ","; 255 - INT(RND(1) * 12)
NEXT
PRINT #1, "BYT"; INT(RND(1) * 255)
NEXT: CLOSE

