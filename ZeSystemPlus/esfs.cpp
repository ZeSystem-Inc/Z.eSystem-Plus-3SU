#include "disk.hpp"
#include "esfs.hpp"

static ESFSHeader header;

static int strcmp_simple(const char* a, const char* b) {
    int i = 0;
    while(a[i] && b[i]) {
        if(a[i] != b[i]) return 1;
        i++;
    }
    return (a[i] == b[i]) ? 0 : 1;
}

static void strcpy_simple(char* dst, const char* src) {
    int i = 0;
    while(src[i] && i < 31) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void esfs_init() {
    read_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
}

void esfs_format(uint32_t total_ssd_sectors) {
    header.magic[0] = 'E'; header.magic[1] = 'S'; header.magic[2] = 'F'; header.magic[3] = 'S';
    header.version    = 3; 
    header.file_count = 0;

    header.max_files = total_ssd_sectors / 256;
    if(header.max_files < 32) header.max_files = 32;

    header.table_sectors = (header.max_files + 7) / 8;
    header.data_base_sector = ESFS_TABLE_START_SECTOR + header.table_sectors;

    FileEntry empty_sector_table[8];
    for(int i = 0; i < 8; i++) empty_sector_table[i].used = 0;

    for(uint32_t s = 0; s < header.table_sectors; s++) {
        write_sector(ESFS_TABLE_START_SECTOR + s, (uint8_t*)empty_sector_table);
    }

    write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
}

bool esfs_find_file(const char* name, FileEntry* out_entry, uint32_t* out_disk_sector, uint32_t* out_entry_idx) {
    FileEntry chunk[8];

    for(uint32_t s = 0; s < header.table_sectors; s++) {
        uint32_t current_sector = ESFS_TABLE_START_SECTOR + s;
        read_sector(current_sector, (uint8_t*)chunk);

        for(int i = 0; i < 8; i++) {
            if(chunk[i].used && strcmp_simple(chunk[i].name, name) == 0) {
                if(out_entry)       *out_entry = chunk[i];
                if(out_disk_sector) *out_disk_sector = current_sector;
                if(out_entry_idx)   *out_entry_idx = i;
                return true;
            }
        }
    }
    return false;
}

bool esfs_save(const char* name, const char* data, uint32_t size) {
    if(size == 0) return false; // uint32_t doğal olarak max 4GB sınırındadır, taşma riski bitti.

    if(esfs_find_file(name, 0, 0, 0)) return false; 

    FileEntry chunk[8];
    uint32_t sector_count = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;

    uint32_t next_free_sector = header.data_base_sector;
    for(uint32_t s = 0; s < header.table_sectors; s++) {
        read_sector(ESFS_TABLE_START_SECTOR + s, (uint8_t*)chunk);
        for(int i = 0; i < 8; i++) {
            if(chunk[i].used) {
                uint32_t end = chunk[i].start_sector + chunk[i].sectors;
                if(end > next_free_sector) next_free_sector = end;
            }
        }
    }

    for(uint32_t s = 0; s < header.table_sectors; s++) {
        uint32_t current_sector = ESFS_TABLE_START_SECTOR + s;
        read_sector(current_sector, (uint8_t*)chunk);

        for(int i = 0; i < 8; i++) {
            if(!chunk[i].used) {
                chunk[i].used         = 1;
                chunk[i].size         = size; 
                chunk[i].start_sector = next_free_sector;
                chunk[i].sectors      = sector_count;
                strcpy_simple(chunk[i].name, name);

                for(uint32_t vs = 0; vs < sector_count; vs++) {
                    uint8_t sector_buffer[SECTOR_SIZE] = {0};
                    uint32_t offset = vs * SECTOR_SIZE;
                    uint32_t bytes  = size - offset;
                    if(bytes > SECTOR_SIZE) bytes = SECTOR_SIZE;

                    for(uint32_t j = 0; j < bytes; j++) {
                        sector_buffer[j] = data[offset + j];
                    }
                    write_sector(chunk[i].start_sector + vs, sector_buffer);
                }

                write_sector(current_sector, (uint8_t*)chunk);
                header.file_count++;
                write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
                return true;
            }
        }
    }
    return false;
}

bool esfs_load(const char* name, char* buffer) {
    FileEntry file;
    if(!esfs_find_file(name, &file, 0, 0)) return false;

    uint32_t remaining  = file.size; 
    uint32_t buf_offset = 0;

    for(uint32_t s = 0; s < file.sectors; s++) {
        uint8_t sector_buffer[SECTOR_SIZE];
        read_sector(file.start_sector + s, sector_buffer);

        uint32_t bytes = remaining > SECTOR_SIZE ? SECTOR_SIZE : remaining;
        for(uint32_t j = 0; j < bytes; j++) {
            buffer[buf_offset++] = sector_buffer[j];
        }
        remaining -= bytes;
    }
    buffer[file.size] = '\0';
    return true;
}

bool esfs_delete(const char* name) {
    uint32_t target_sector = 0;
    uint32_t target_idx = 0;
    FileEntry chunk[8];

    if(!esfs_find_file(name, 0, &target_sector, &target_idx)) return false;

    read_sector(target_sector, (uint8_t*)chunk);
    chunk[target_idx].used         = 0;
    chunk[target_idx].size         = 0;
    chunk[target_idx].start_sector = 0;
    chunk[target_idx].sectors      = 0;
    chunk[target_idx].name[0]      = '\0';
    write_sector(target_sector, (uint8_t*)chunk);

    if(header.file_count > 0) header.file_count--;
    write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
    return true;
}

void esfs_list() {
    FileEntry chunk[8];
    for(uint32_t s = 0; s < header.table_sectors; s++) {
        read_sector(ESFS_TABLE_START_SECTOR + s, (uint8_t*)chunk);
        for(int i = 0; i < 8; i++) {
            if(chunk[i].used) {
                print(chunk[i].name, 0x0A);
                print("\n", 0x0F);
            }
        }
    }
}

bool esfs_is_formatted() {
    return (header.magic[0] == 'E' && header.magic[1] == 'S');
}