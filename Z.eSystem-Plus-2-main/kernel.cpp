#include "kernel.hpp"
#include "esfs.hpp"
#include "disk.hpp"
#include "installer.hpp"
#include "pci.hpp"
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
	
	int cursor_x = 0;
	int cursor_y = 0;
	unsigned char default_color = 0x0F;
	
	#define MAX_SANAL_SATIR 500
	char sanal_ekran[MAX_SANAL_SATIR][80];
	unsigned char sanal_renk[MAX_SANAL_SATIR][80];
	int aktif_toplam_satir = 25;
	int kaydirma_ofseti = 0;   
	
	void kernel_main() {
		clear();
		
		pci_scan();
		
		esfs_init();
		
		char buffer[512];
		
		if(esfs_save("test.txt", "Merhaba ESFS", 12)) {
			print("Kaydedilme basarili!\n", default_color);
		} else {
			print("Kaydedilme basarisiz oldu!", default_color);
		}
		
		if(esfs_load("test.txt", buffer)) {
			print("Icerik: ", default_color);
			print(buffer, 0x0B);
			print("\n", default_color);
			print("Yukleme basarili!\n", default_color);
		} else {
			print("test.txt yuklenemedi!\n", default_color);
		}
		
		if(esfs_delete("test.txt")) {
			print("Silme basarili oldu!\n", default_color);
		} else {
			print("Silme basarisiz oldu!\n", default_color);
		}
		
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
					char cmd[40] = {0};
					char arg[40] = {0};
					komut_ayir(komut_buffer, cmd, arg);
					
					if(strcmp(cmd, "help") == 0) {
						print("Komutlar:\n"
						      "help - Komut listesini gosterir\n"
						      "clear - ekrani temizler\n"
						      "version - Z.eSystem surumunu gosterir\n"
						      "color <0 - 3> - renk degistir\n"
						      "shutdown - biligsayari kapatir\n"
						      "editor <isim> - text editoru acar\n"
						      "ls - Dosyalari listeler\n"
						      "cat <isim> - Dosyayi acar\n"
						      "rm <isim> - Dosyayi siler\n"
						      "format - Bilgisayari format atar\n"
						      "install - Sistemi sabit diske kurar\n"
							  "Asagi veya yukari tuslariyla asagi veya yukari kaydirabilirsiniz.\n", default_color);
					}
					
					else if(strcmp(cmd, "clear") == 0) {
						clear();
					}
					
					else if(strcmp(cmd, "version") == 0) {
						print("Z.eSystem Plus 2\n", default_color);
						print("ESUI 1.0.9\n", default_color);
						print("ZeSys Kernel 1.2.2\n", default_color);
					}
					
					else if(strcmp(cmd, "color") == 0) {
						if(strcmp(arg, "0") == 0) default_color = 0x0F;
						else if(strcmp(arg, "1") == 0) default_color = 0x0A;
						else if(strcmp(arg, "2") == 0) default_color = 0x0E;
						else if(strcmp(arg, "3") == 0) default_color = 0x0C;
						clear();
					}
					
					else if(strcmp(cmd, "shutdown") == 0) {
						print("Sistem kapatiliyor...\n", default_color);
						outw(0x604, 0x2000);
						outw(0xB004, 0x2000);
					}
					
					else if(strcmp(cmd, "editor") == 0) {
						if(arg[0] == '\0') {
							print("Hata: Lutfen dosya adi giriniz! (Ornek: editor note.txt)\n", default_color);
						} else {
							clear();
							text_editor(arg);
						}
					}
					
					else if(strcmp(cmd, "cat") == 0) {
						if(arg[0] == '\0') {
							print("Lutfen dosya adi giriniz!(Ornek: cat note.txt)\n", default_color);
						} else {
							char cat_buffer[4096];
							if(esfs_load(arg, cat_buffer)) {
								print(arg, 0x0B);
								print(" icerigi:\n", default_color);
								print(cat_buffer, default_color);
								print("\n", default_color);
							} else {
								print("Dosya bulunamadi veya okunamadi!\n", default_color);
							}
						}
					}
					
					else if(strcmp(cmd, "ls") == 0) {
						esfs_list();
					}
					
					else if(strcmp(cmd, "rm") == 0) {
						if(arg[0] == '\0') {
							print("Hata: Lutfen dosya adi yazin! (Ornek: rm note.txt)\n", default_color);
						} else {
							if(esfs_delete(arg)) {
								print("Dosya basariyla silindi!\n", default_color);
							} else {
								print("Dosya bulunamadi veya silinemedi!\n", default_color);
							}
						}
					}
					
					else if(strcmp(cmd, "format") == 0) {
						print("Format yapiliyor...\n", default_color);
						esfs_format();
						print("Format yapildi!\n", default_color);
					}
					
					else if(strcmp(cmd, "install") == 0) {
						sys_install();
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
					
					sanal_ekran[cursor_y][cursor_x] = ' ';
					sanal_renk[cursor_y][cursor_x] = default_color;
					ekrani_yenile();
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
	
	void ekrani_yenile() {
		volatile char* vga = (volatile char*) 0xB8000;
		
		int baslangic_satiri = (aktif_toplam_satir - 25) - kaydirma_ofseti;
		if (baslangic_satiri < 0) baslangic_satiri = 0;

		for (int y = 0; y < 25; y++) {
			int sanal_y = baslangic_satiri + y;
			for (int x = 0; x < 80; x++) {
				int konum = (y * 80 + x) * 2;
				vga[konum] = sanal_ekran[sanal_y][x];
				vga[konum + 1] = sanal_renk[sanal_y][x];
			}
		}
	}
	
	void print(const char* str, unsigned char color) {
		for (int i = 0; str[i] != '\0'; i++) {
			char harf = str[i];
			
			if (harf == '\n') {
				newline();
			} 
			else {
				sanal_ekran[cursor_y][cursor_x] = harf;
				sanal_renk[cursor_y][cursor_x] = color;
				cursor_x++;
				
				if (cursor_x >= 80) {
					newline();
				}
			}
		}
		
		if (kaydirma_ofseti == 0) {
			ekrani_yenile();
		}
	}
	
	void clear() {
		for(int y = 0; y < MAX_SANAL_SATIR; y++) {
			for(int x = 0; x < 80; x++) {
				sanal_ekran[y][x] = ' ';
				sanal_renk[y][x] = default_color;
			}
		}
		
		cursor_x = 0;
		cursor_y = 0;
		aktif_toplam_satir = 25;
		kaydirma_ofseti = 0;
		ekrani_yenile();
	}
	
	void newline() {
		cursor_x = 0;
		cursor_y++;
		
		if (cursor_y >= aktif_toplam_satir) {
			if (aktif_toplam_satir < MAX_SANAL_SATIR) {
				aktif_toplam_satir++;
			} else {
				for (int y = 1; y < MAX_SANAL_SATIR; y++) {
					for (int x = 0; x < 80; x++) {
						sanal_ekran[y - 1][x] = sanal_ekran[y][x];
						sanal_renk[y - 1][x] = sanal_renk[y][x];
					}
				}
				for (int x = 0; x < 80; x++) {
					sanal_ekran[MAX_SANAL_SATIR - 1][x] = ' ';
					sanal_renk[MAX_SANAL_SATIR - 1][x] = default_color;
				}
				cursor_y = MAX_SANAL_SATIR - 1;
			}
		}
		
		if (kaydirma_ofseti == 0) {
			ekrani_yenile();
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
		if((inb(0x64) & 1) == 0) {
			return 0;
		}

		uint8_t scancode = inb(0x60);

		if(scancode & 0x80) {
			return 0;
		}

		if (scancode == 0xE0) {
			while (!((inb(0x64) & 1) != 0)); 
			uint8_t asil_kod = inb(0x60);
			
			if (asil_kod == 0x48) {
				if (kaydirma_ofseti < (aktif_toplam_satir - 25)) {
					kaydirma_ofseti++;
					ekrani_yenile();
				}
			}
			else if (asil_kod == 0x50) {
				if (kaydirma_ofseti > 0) {
					kaydirma_ofseti--;
					ekrani_yenile();
				}
			}
			return 0;
		}
		
		if (scancode == 0x48) { 
			if (kaydirma_ofseti < (aktif_toplam_satir - 25)) {
				kaydirma_ofseti++;
				ekrani_yenile();
			}
			return 0;
		}
		if (scancode == 0x50) { 
			if (kaydirma_ofseti > 0) {
				kaydirma_ofseti--;
				ekrani_yenile();
			}
			return 0;
		}
		
		if (kaydirma_ofseti > 0) {
			kaydirma_ofseti = 0;
			ekrani_yenile();
		}
		
		switch(scancode) {
			case 0x1E: return 'a'; case 0x30: return 'b'; case 0x2E: return 'c'; case 0x20: return 'd';
			case 0x12: return 'e'; case 0x21: return 'f'; case 0x22: return 'g'; case 0x23: return 'h';
			case 0x17: return 'i'; case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
			case 0x32: return 'm'; case 0x31: return 'n'; case 0x18: return 'o'; case 0x19: return 'p';
			case 0x10: return 'q'; case 0x13: return 'r'; case 0x1F: return 's'; case 0x14: return 't';
			case 0x16: return 'u'; case 0x2F: return 'v'; case 0x11: return 'w'; case 0x2D: return 'x';
			case 0x15: return 'y'; case 0x2C: return 'z';
			
			case 0x02: return '1'; case 0x03: return '2'; case 0x04: return '3'; case 0x05: return '4';
			case 0x06: return '5'; case 0x07: return '6'; case 0x08: return '7'; case 0x09: return '8';
			case 0x0A: return '9'; case 0x0B: return '0';
			
			case 0x34: return '.'; case 0x35: return '/';
			
			case 0x39: return ' '; case 0x1C: return '\n'; case 0x0E: return '\b'; case 0x01: return 27;
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
	
	void outw(uint16_t port, uint16_t veri) {
		asm volatile("outw %0, %1" :: "a"(veri), "Nd"(port));
	}
	
	void text_editor(const char* dosya_adi) {
		print("=====================Text Editor=====================\n", default_color);
		print("Duzenlenen dosya: ", default_color);
		print(dosya_adi, 0x0B);
		print("\nCikmak ve kaydetmek icin ESC tusuna basin\n", default_color);
		enter(2);
		
		char metin_tamponu[4000];
		int metin_index = 0;
		
		cursor_x = 0;
		cursor_y = 4;
		
		while(1) {
			char t_harf = get_char();
			
			if(t_harf == 27) {
				break;
			}
			
			else if(t_harf == '\n') {
				if(metin_index < 3999) {
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
					
					sanal_ekran[cursor_y][cursor_x] = ' ';
					sanal_renk[cursor_y][cursor_x] = default_color;
					ekrani_yenile();
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
		
		if(esfs_save(dosya_adi, metin_tamponu, metin_index)) {
			print("Dosya basariyla kaydedildi!\n", default_color);
		} else {
			print("Dosya kaydedilemedi!\n", default_color);
		}
	}
	
	void komut_ayir(const char* kaynak, char* komut, char* arguman) {
		int i = 0;
		int k = 0;
		int a = 0;
		
		while(kaynak[i] == ' ') {
			i++;
		}
		
		while(kaynak[i] != ' ' && kaynak[i] != '\0') {
			komut[k++] = kaynak[i++];
		}
		komut[k] = '\0';
		
		while(kaynak[i] == ' ') {
			i++;
		}
		
		while(kaynak[i] != '\0') {
			arguman[a++] = kaynak[i++];
		}
		arguman[a] = '\0';
	}
}