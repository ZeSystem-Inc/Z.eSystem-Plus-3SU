#include "kernel.hpp"
#include <stdint.h>

extern "C" {
	
	void print(const char *str, unsigned char color);
	void clear();
	void newline();
	void enter(int number);
	uint8_t inb(uint16_t port);
	char get_char();
	int strcmp(const char *s1, const char *s2);
	void outb(uint16_t port, uint8_t veri);
	void outw(uint16_t port, uint16_t veri);
	void text_editor();
	
	int cursor_x = 0;
	int cursor_y = 0;
	unsigned char default_color = 0x0F;
	
	void kernel_main() {
		clear();
		print("Z.eSystem Plus'a hosgeldiniz!\n", default_color);
		print("Yardim icin 'help' yazin.\n", default_color);
		print("> ", default_color);
		
		char komut_buffer[80];
		int komut_index = 0;
		
		while(1) {
			char harf = get_char();
			
			if(harf == '\n') {
				newline();
				komut_buffer[komut_index] = '\0';
				
				if(komut_index > 0) {
					if(strcmp(komut_buffer, "help") == 0) {
						print("Komutlar:\n", default_color);
						print("help - Komut listesini gosterir\n", default_color);
						print("clear - ekrani temizler\n", default_color);
						print("version - Z.eSystem surumunu gosterir\n", default_color);
						print("color 0 - 3 - renk degistir\n", default_color);
						print("reboot - biligsayari yeniden baslatir\n", default_color);
						print("shutdown - biligsayari kapatir\n", default_color);
						print("texteditor - text editoru acar\n", default_color);
					}
					
					else if(strcmp(komut_buffer, "clear") == 0) {
						clear();
					}
					
					else if(strcmp(komut_buffer, "version") == 0) {
						print("Z.eSystem Plus\n", default_color);
						print("ESUI 1.0.2\n", default_color);
						print("ZeSys Kernel 1.0.6\n", default_color);
					}
					
					else if(strcmp(komut_buffer, "color 0") == 0) {
						default_color = 0x0F;
						clear();
					}
					
					else if(strcmp(komut_buffer, "color 1") == 0) {
						default_color = 0x0A;
						clear();
					}
					
					else if(strcmp(komut_buffer, "color 2") == 0) {
						default_color = 0x0E;
						clear();
					}
					
					else if(strcmp(komut_buffer, "color 3") == 0) {
						default_color = 0x0C;
						clear();
					}
					
					else if(strcmp(komut_buffer, "reboot") == 0) {
						print("Sistem yeniden baslatiliyor...\n", default_color);
						for(volatile int i = 0; i < 50000000; i++);
						outb(0x64, 0xFE);
					}
					
					else if(strcmp(komut_buffer, "shutdown") == 0) {
						print("Sistem kapatiliyor...\n", default_color);
						for(volatile int i = 0; i < 50000000; i++);
						outw(0x604, 0x2000);
						outw(0xB004, 0x2000);
					}
					
					else if(strcmp(komut_buffer, "texteditor") == 0) {
						clear();
						text_editor();
					}
					
					else {
						print("Hata: '", default_color);
						print(komut_buffer, default_color);
						print("' diye bir komut yoktur!\n", default_color);
					}
				}
				
				komut_index = 0;
				print("> ", default_color);
			}
			
			else if(harf == '\b') {
				if(komut_index > 0) {
					komut_index--;
					cursor_x--;
					
					volatile char* vga = (volatile char*) 0xB8000;
					int konum = (cursor_y * 80 + cursor_x) * 2;
					vga[konum] = ' ';
					vga[konum + 1] = default_color;
				}
			}
			
			else if(harf != 0) {
				if(komut_index < 79) {
					komut_buffer[komut_index++] = harf;
					char tek_harf_dizisi[2] = {harf, '\0'};
					print(tek_harf_dizisi, default_color);
				}
			}
		}
	}
	
	void print(const char *str, unsigned char color) {
		volatile char* vga = (volatile char*) 0xB8000;
		int i = 0;
		
		while(str[i] != '\0') {
			char c = str[i];
			
			if(c == '\n') {
				newline();
			}
			
			else {
				int konum = (cursor_y * 80 + cursor_x) * 2;
				
				vga[konum] = c;
				vga[konum + 1] = color;
				
				cursor_x++;
				
				if(cursor_x >= 80) {
					newline();
				}
			}
			
			i++;
		}
	}
	
	void clear() {
		volatile char* vga = (volatile char*) 0xB8000;
		
		for(int i = 0; i < 80 * 25; i++) {
			vga[i * 2] = ' ';
			vga[i * 2 + 1] = default_color;
		}
		
		cursor_x = 0;
		cursor_y = 0;
	}
	
	void newline() {
		cursor_x = 0;
		cursor_y++;
		if(cursor_y >= 25) {
			volatile char* vga = (volatile char*) 0xB8000;
			for(int y = 1; y < 25; y++) {
				for(int x = 0; x < 80; x++) {
					int eski_yer = ((y - 1) * 80 + x) * 2;
					int yeni_yer = (y * 80 + x) * 2;
					
					vga[eski_yer] = vga[yeni_yer];
					vga[eski_yer + 1] = vga[yeni_yer + 1];
				}
			}

			for(int x = 0; x < 80; x++) {
				int alt_satir = (24 * 80 + x) * 2;
				vga[alt_satir] = ' ';
				vga[alt_satir + 1] = default_color;
 			}
			
			cursor_y = 24;
		}
	}
	
	void enter(int number) {
		for(int i = 0; i < number; i++) {
			newline();
		}
	}
	
	uint8_t inb(uint16_t port) {
		uint8_t veri;
		asm volatile("inb %1, %0" : "=a"(veri) : "Nd"(port));
		return veri;
	}
	
	char get_char() {
		while((inb(0x64) & 1) == 0) {
			
		}
		
		uint8_t scancode = inb(0x60);
		
		if (scancode & 0x80) {
			return 0;
		}
		
		switch(scancode) {
			case 0x1E: return 'a'; case 0x30: return 'b'; case 0x2E: return 'c'; case 0x20: return 'd';
			case 0x12: return 'e'; case 0x21: return 'f'; case 0x22: return 'g'; case 0x23: return 'h';
			case 0x17: return 'i'; case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
			case 0x32: return 'm'; case 0x31: return 'n'; case 0x18: return 'o'; case 0x19: return 'p';
			case 0x10: return 'q'; case 0x13: return 'r'; case 0x1F: return 's'; case 0x14: return 't';
			case 0x16: return 'u'; case 0x2F: return 'v'; case 0x11: return 'w'; case 0x2D: return 'x';
			case 0x15: return 'y'; case 0x2C: return 'z';
			
			case 0x02: return '1';
			case 0x03: return '2';
			case 0x04: return '3';
			case 0x05: return '4';
			case 0x06: return '5';
			case 0x07: return '6';
			case 0x08: return '7';
			case 0x09: return '8';
			case 0x0A: return '9';
			case 0x0B: return '0';
			
			case 0x39: return ' ';
			case 0x1C: return '\n';
			case 0x0E: return '\b';
			case 0x01: return 27;
			default: return 0;
		}
	}
	
	int strcmp(const char *s1, const char *s2) {
		while(*s1 && (*s1 == *s2)) {
			s1++;
			s2++;
		}
		
		return *(const unsigned char*)s1 - *(const unsigned char*)s2;
	}
	
	void outb(uint16_t port, uint8_t veri) {
		asm volatile("outb %0, %1" :: "a"(veri), "Nd"(port));
	}
	
	void outw(uint16_t port, uint16_t veri) {
		asm volatile("outw %0, %1" :: "a"(veri), "Nd"(port));
	}
	
	void text_editor() {
		print("=====================Text Editor=====================\n", default_color);
		print("Cikmak icin ESC tusuna basin\n", default_color);
		enter(2);
		
		char metin_tamponu[2000];
		int metin_index = 0;
		
		cursor_x = 0;
		cursor_y = 2;
		
		while(1) {
			char t_harf = get_char();
			
			if(t_harf == 27) {
				break;
			}
			
			else if(t_harf == '\n') {
				if(metin_index < 1999) {
					metin_tamponu[metin_index++] = '\n';
					newline();
				}
			}
			
			else if(t_harf == '\b') {
				if(cursor_x > 0 || cursor_y > 2) {
					if(metin_index > 0) {
						metin_index--;
					}
					
					if(cursor_x == 0) {
						cursor_y--;
						cursor_x = 79;
					} else {
						cursor_x--;
					}
					
					volatile char* vga = (volatile char*) 0xB8000;
					int konum = (cursor_y * 80 + cursor_x) * 2;
					vga[konum] = ' ';
					vga[konum + 1] = default_color;
				}
			}
			
			else if(t_harf != 0) {
				if(metin_index < 1999) {
					metin_tamponu[metin_index++] = t_harf;
					char t_tek_harf_dizisi[2] = {t_harf, '\0'};
					print(t_tek_harf_dizisi, default_color);
				}
			}
		}
		
		clear();
		print("Ana ekrana basariyla donuldu!\n", default_color);
	}
}