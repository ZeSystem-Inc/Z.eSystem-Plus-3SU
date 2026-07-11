#ifndef KERNEL_HPP
#define KERNEL_HPP

#include <stdint.h>

extern "C" {
	void print(const char *str, unsigned char color);
	void clear();
	void newline();
	void enter(int number);
	uint8_t inb(uint16_t port);
	char get_char();
	int strcmp(const char *s1, const char *s2);
	void outw(uint16_t port, uint16_t veri);
	void text_editor(const char* dosya_adi);
	void komut_ayir(const char* kaynak, char* komut, char* arguman);
	void ekrani_yenile();
}

#endif