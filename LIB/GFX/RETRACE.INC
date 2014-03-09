  .code
; Waits for Retrace
Retrace:
  push ax,dx
  mov dx,3dah
Retrace2:
  in al,dx
  test al,1000b
  jz Retrace2
Retrace3:
  in al,dx
  test al,1000b
  jnz Retrace3
  pop dx,ax
  ret
