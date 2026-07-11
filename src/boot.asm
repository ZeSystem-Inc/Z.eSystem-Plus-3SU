[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000

start_boot:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00
	
	mov [BOOT_DRIVE], dl
	sti
	
	call load_kernel
	
	in al, 0x92
	or al, 00000010b
	and al, 11111101b
	out 0x92, al
	
	cli
	lgdt [gdt_descriptor]
	
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax
	
	jmp CODE_SEG:start_pm
	
[bits 16]
load_kernel:
	mov ah, 0x00
	mov dl, [BOOT_DRIVE]
	int 0x13
	jc disk_err

	mov bx, KERNEL_OFFSET
	mov ah, 0x02
	mov al, 17
	mov ch, 0x00
	mov dh, 0x00
	mov cl, 0x02
	mov dl, [BOOT_DRIVE]
	int 0x13
	jc disk_err
	ret
	
disk_err:
	mov ah, 0x0E
	mov si, disk_err_msg
.yazdir16_loop:
	lodsb
	cmp al, 0
	jz $
	int 0x10
	jmp .yazdir16_loop

disk_err_msg db "Disk read error!",0
BOOT_DRIVE db 0
	
	
gdt_start:
	dd 0x0
	dd 0x0
gdt_code:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0
gdt_data:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start
	
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


[bits 32]
start_pm:
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov esp, 0x90000
	
	jmp CODE_SEG:KERNEL_OFFSET
	
	jmp $

times 510-($-$$) db 0
dw 0xAA55