; The bootsector is loaded to 0000:7c00. You can assemble a COM file and write
; it to the boot sector

	org 100h,7c00h

boot_sector:
	jmp loader_code
	nop
	;
	;Here would come the header
	;

loader_code:
	mov ax,1234h	;warm reboot
	mov ds,40h
	mov [72h],ax
	jmp 0ffffh:0

