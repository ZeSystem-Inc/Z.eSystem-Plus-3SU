[bits 32]
section .text.boot
global _start
extern kernel_main

_start:
	cli
	lgdt [gdt_descriptor]
	jmp CODE_SEG:.reolad_cs
.reolad_cs:
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	call kernel_main
	cli
.hang:
	hlt
	jmp .hang
	
align 8
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