; Routines for a moveable viewer
; (c)1997 Ba'al shem
        .code


get_viewport:
        ;x=V*n-Z*n
        mov si,viewZ
        mov di,viewN
        call vScalar
        push eax
        mov si,viewV
        mov di,viewN
        call vScalar
        pop ebx
        sub eax,ebx
        mov view_x,eax
        ;
        ;Y(y)*X(x)-Y(x)*X(y)
        movsx eax,w viewZ
        ;Y(z)*X(y)-Y(y)*X(z)
        ;Y(z)*X(x)-Y(x)*Y(z)

vScalar:        
        movsx eax,w x[si]
        movsx ebx,w x[di]
        cdq
        imul ebx
        mov ecx,eax
        movsx eax,w y[si]
        movsx ebx,w y[di]
        cdq
        imul ebx
        add ecx,eax
        movsx eax,w z[si]
        movsx ebx,w z[di]
        cdq
        imul ebx
        add eax,ecx
        ret


        .data
viewZ   dd ?,?          ;X
viewV   dd ?,?
viewN   dd ?,?          ;User's distance vector from screen

