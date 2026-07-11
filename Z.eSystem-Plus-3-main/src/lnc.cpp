#include "lnc.hpp"
#include "esfs.hpp"
#include "kernel.hpp"
#include "disk.hpp"

#define LNC_EXECUTATION_ZONE 0x00500000

extern "C" {
    typedef uint32_t (*lnc_entry_t)();

    bool lnc_launch(const char* filename) {
        char* load_buffer = (char*)LNC_EXECUTATION_ZONE;
        
        FileEntry dosya_bilgisi;
        uint32_t entry_sector = 0;
        uint32_t entry_idx = 0;

        if (!esfs_find_file(filename, &dosya_bilgisi, &entry_sector, &entry_idx)) {
            print("LNC Hata: Program bulunamadi!\n", 0x0C);
            return false;
        }
        
        uint32_t start_sector = dosya_bilgisi.start_sector; 
        uint32_t total_sectors = (dosya_bilgisi.size + 511) / 512; 
        
        for (uint32_t i = 0; i < total_sectors; i++) {
            read_sector(start_sector + i, (uint8_t*)(load_buffer + (i * 512)));
        }
        
        LNC_Header* header = (LNC_Header*)load_buffer;
        
        if(header->magic != LNC_MAGIC) {
            print("LNC Hata: Gecersiz LNC sihirli numarasi!\n", 0x0C);
            return false;
        }
        
        uint32_t absolute_entry = (uint32_t)load_buffer + header->entry_offset;
        
        lnc_entry_t launch_target = (lnc_entry_t)absolute_entry;
        uint32_t donen_harf = launch_target();
        
        if (donen_harf != 0) {
            char c[2] = { (char)donen_harf, '\0' };
            print(c, 0x0F); 
            print("\n", 0x0F); 
        }
        
        print("Z.eSystem sorunsuz bir sekilde kernel'a dondu!\n", 0x0A); 
        return true; 
    }
}