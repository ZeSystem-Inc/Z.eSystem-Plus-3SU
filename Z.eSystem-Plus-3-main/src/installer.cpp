#include "installer.hpp"
#include <stdint.h>
#include "disk.hpp"
#include "esfs.hpp"
#include "pci.hpp"

extern "C" {
    void print(const char* str, unsigned char color);
    extern unsigned char default_color;
	char get_char();
}

void sys_install() {
    PCI_Device dev;
	
	uint32_t toplam_disk_sektor_i = disk_get_total_sectors();

    if (!pci_find_storage(&dev)) {
        print("Storage controller bulunamadi!\n", 0x0C);
        return;
    }

    if (dev.subclass == 0x06) {
        print("AHCI/SATA bulundu fakat henuz desteklenmiyor!\n", 0x0E);
        return;
    }

    if (dev.subclass == 0x08) {
        print("NVMe bulundu fakat henuz desteklenmiyor!\n", 0x0E);
        return;
    }

    if (dev.subclass != 0x01) {
        print("Bilinmeyen storage controller!\n", 0x0C);
        return;
    }

    print("IDE controller bulundu.\n", 0x0A);

    if (esfs_is_formatted()) {
        print("Sistem zaten kuruldu!\n", default_color);
        return;
    }

    print("Kurulum baslatiliyor... Lutfen bekleyiniz\n", 0x0E);

    uint8_t SOURCE_DRIVE = 0xF0;
    uint8_t TARGET_DRIVE = 0xE0;

    uint8_t zero_buffer[512];

    for (int i = 0; i < 512; i++) {
        zero_buffer[i] = 0x00;
    }

    for (uint32_t s = 100; s < 1000; s++) {
        write_sector(TARGET_DRIVE, s, zero_buffer);

        if ((s % 100) == 0)
            print(".", 0x0A);
    }

    uint8_t sector_buffer[512];

    print("\nKernel ve sistem dosyalari kopyalaniyor...\n", default_color);

    for (uint32_t s = 0; s < 100; s++) {
        read_sector(SOURCE_DRIVE, s, sector_buffer);
        write_sector(TARGET_DRIVE, s, sector_buffer);

        if ((s % 25) == 0)
            print(".", 0x0B);
    }

    print("\nDosya sistemi yapilandiriliyor...\n", default_color);

    for (uint32_t s = 100; s < 500; s++) {
        read_sector(SOURCE_DRIVE, s, sector_buffer);
        write_sector(TARGET_DRIVE, s, sector_buffer);

        if ((s % 50) == 0)
            print(".", 0x0A);
    }

    print("\nKurulum TAMAMLANDI!\n", 0x0A);
	
	print("Sistemin saglikli calismasi icin disk formatlansin mi? ", 0x0E);
	
	while(1) {
		char secim = get_char();
		
		if(secim == 'e') {
			print("e\n", default_color);
			print("Format yapiliyor...\n", default_color);
			esfs_format(toplam_disk_sektor_i);
			print("Disk formatlandi! Dosya sistemi yuklendi.\n", 0x0A);
			break;
		} else if(secim == 'h') {
			print("h\n", default_color);
			print("Disk formatlama atlandi! Dosya sistemi yuklenenemedi.\n", 0x0C);
			break;
		}
	}
	
	
    print("Sistemi kapatabilir ve medyayi cikarabilirsiniz.\n", default_color);
}