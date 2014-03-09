DIM w(256)
sustain = 8
scalelen = 64 - sustain
scale = 102 / scalelen
FOR a = 128 TO 128 + sustain - 1
        w(a) = a
        NEXT
FOR a = 0 TO scalelen - 1
        w(a + 128 + sustain) = INT(a * scale) + 128 + sustain
        leftrange = w(a + 128 + sustain)
        NEXT
scalelen = scalelen + sustain
FOR a = 0 TO 127 - scalelen
        w(a + 128 + scalelen) = INT(a / (128 - scalelen) * (255 - leftrange) + leftrange)
        NEXT

OPEN "boost.inc" FOR OUTPUT AS #1
FOR a = 0 TO 128
        w(a) = 256 - w(255 - a)
        NEXT
FOR a = 0 TO 255
        PRINT #1, "db"; w(a)
        NEXT
CLOSE

