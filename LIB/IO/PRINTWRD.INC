	.code
PrintWord:
        push dx
        mov bx,10
        xor cx,cx
PrintWord2:
        xor dx,dx
        div bx
        push dx
        inc cx
        or ax,ax
        jne PrintWord2
PrintWord3:
        pop dx
        mov al,dl
        add al,"0"
        int 29h
        loop PrintWord3
        pop dx
        mov ah,9
        int 21h
        ret

