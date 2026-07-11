[org 0x7c00]
[bits 16]

KERNEL_SECTORS equ 50
KERNEL_OFFSET equ 0x1000

_start:
    mov [BOOT_DRIVE], dl

    xor ax, ax
    mov bp, 0x7c00
    mov sp, bp

    call disk_oku
    call korumali_moda_gec

    jmp $

disk_oku:
    mov ax, 0
    mov es, ax
    mov bx, KERNEL_OFFSET

    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    mov dl, [BOOT_DRIVE]

    int 0x13
    jc disk_hatasi
    ret

disk_hatasi:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

korumali_moda_gec:
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000

    jmp CODE_SEG:KERNEL_OFFSET 

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

times 510-($-$$) db 0
dw 0xAA55