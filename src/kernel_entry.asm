[bits 32]
section .text.boot
global _start
extern kernel_main

_start:
	call kernel_main
	cli
.hang:
	hlt
	jmp .hang