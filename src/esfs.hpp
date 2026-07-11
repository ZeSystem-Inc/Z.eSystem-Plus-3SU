#ifndef ESFS_HPP
#define ESFS_HPP

#include <stdint.h>

#define SECTOR_SIZE             512
#define ESFS_HEADER_SECTOR      100
#define ESFS_TABLE_START_SECTOR 101

extern "C" void print(const char* str, unsigned char color);

struct ESFSHeader {
    char magic[4];
    uint32_t version;
    uint32_t file_count;
    uint32_t max_files;
    uint32_t table_sectors;    
    uint32_t data_base_sector; 
};

struct FileEntry {
    char name[32];
    uint32_t size;
    uint32_t start_sector;
    uint32_t sectors;
    uint8_t used;
    uint8_t reserved[19];
};

void esfs_init();
void esfs_format(uint32_t total_ssd_sectors); 
bool esfs_find_file(const char* name, FileEntry* out_entry, uint32_t* out_disk_sector, uint32_t* out_entry_idx);
bool esfs_save(const char* name, const char* data, uint32_t size);
bool esfs_load(const char* name, char* buffer);
bool esfs_delete(const char* name);
void esfs_list();
bool esfs_is_formatted();

#endif