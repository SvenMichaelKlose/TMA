	.code
set43textlines:
        mov ax,3
        int 10h
        mov ax,1112h
        xor bl,bl
        int 10h
        mov ah,2
        mov dx,43 by 1
        xor bx,bx
        int 10h
        ret

