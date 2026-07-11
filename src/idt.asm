global idt_kur
global son_basilan_tus

section .text
idt_kur:
	pusha
	
	mov eax, pit_handler
	mov word [idt_tablosu + 32*8], ax       
	mov word [idt_tablosu + 32*8 + 2], 0x08 
	mov byte [idt_tablosu + 32*8 + 4], 0    
	mov byte [idt_tablosu + 32*8 + 5], 0x8E 
	shr eax, 16
	mov word [idt_tablosu + 32*8 + 6], ax 

	mov eax, klavye_handler
	mov word [idt_tablosu + 33*8], ax
	mov word [idt_tablosu + 33*8 + 2], 0x08
	mov byte [idt_tablosu + 33*8 + 4], 0
	mov byte [idt_tablosu + 33*8 + 5], 0x8E
	shr eax, 16
	mov word [idt_tablosu + 33*8 + 6], ax


	mov al, 0x11
	out 0x20, al
	out 0xA0, al
	mov al, 0x20 
	out 0x21, al
	mov al, 0x28
	out 0xA1, al
	mov al, 0x04
	out 0x21, al
	mov al, 0x02
	out 0xA1, al
	mov al, 0x01
	out 0x21, al
	out 0xA1, al
	
	mov al, 0xFC
	out 0x21, al
	mov al, 0xFF
	out 0xA1, al

	lidt [idt_descriptor]
	sti                           
	
	popa
	ret

align 4
pit_handler:
	pusha
	cld
	
	movzx ebx, byte [animasyon_sayaci]
	inc ebx
	and ebx, 3
	mov [animasyon_sayaci], bl
	
	cmp ebx, 0
	je .cizgi1
	cmp ebx, 1
	je .cizgi2
	cmp ebx, 2
	je .cizgi3
	
	mov al, 0x5C
	jmp .ekrana_bas
	
.cizgi1:
	mov al, '|'
	jmp .ekrana_bas
.cizgi2:
	mov al, '/'
	jmp .ekrana_bas
.cizgi3:
	mov al, '-'
	
.ekrana_bas:
	mov [0xB8000 + 158], al
	mov [0xB8000 + 159], byte 0x1E
	
	mov al, 0x20
	out 0x20, al
	popa
	iretd

align 4
klavye_handler:
	pusha
	
	in al, 0x60 
	
	test al, 0x80
	jnz .bitti
	mov [son_basilan_tus], al
	mov [0xB8000 + 156], byte 'K'
	mov [0xB8000 + 157], byte 0x0E

.bitti:
	mov al, 0x20
	out 0x20, al
	popa
	iretd

idt_descriptor:
	dw idt_bitis - idt_tablosu - 1 
	dd idt_tablosu 

align 16
idt_tablosu:
	times 256 * 8 db 0
idt_bitis:

align 4
animasyon_sayaci db 0

align 4
son_basilan_tus db 0