#include "disk.hpp"
#include "esfs.hpp"

#define ESFS_HEADER_SECTOR 100
#define ESFS_TABLE_SECTOR  101
#define ESFS_DATA_SECTOR   102

#define ESFS_SECTORS_PER_FILE 8

static ESFSHeader header;
static FileEntry table[ESFS_MAX_FILES];

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
    while(src[i] && i < 15) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void esfs_init() {
    read_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
    read_sector(ESFS_TABLE_SECTOR,  (uint8_t*)table);
}

void esfs_format() {
    header.magic[0] = 'E';
    header.magic[1] = 'S';
    header.magic[2] = 'F';
    header.magic[3] = 'S';
    header.version    = 1;
    header.file_count = 0;

    for(int i = 0; i < ESFS_MAX_FILES; i++) {
        table[i].used    = 0;
        table[i].size    = 0;
        table[i].sectors = 0;
        table[i].name[0] = '\0';
    }

    write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
    write_sector(ESFS_TABLE_SECTOR,  (uint8_t*)table);
}

bool esfs_save(const char* name, const char* data, uint16_t size) {
    uint16_t max_size = ESFS_SECTORS_PER_FILE * SECTOR_SIZE;
    if(size > max_size) return false;

    for(int i = 0; i < ESFS_MAX_FILES; i++) {
        if(!table[i].used) {
            uint8_t sector_count = (uint8_t)((size + SECTOR_SIZE - 1) / SECTOR_SIZE);
            if(sector_count == 0) sector_count = 1;

            table[i].used    = 1;
            table[i].size    = size;
            table[i].sectors = sector_count;
            strcpy_simple(table[i].name, name);

            uint32_t base = ESFS_DATA_SECTOR + (uint32_t)i * ESFS_SECTORS_PER_FILE;

            for(int s = 0; s < sector_count; s++) {
                uint8_t sector_buffer[SECTOR_SIZE];

                for(int j = 0; j < SECTOR_SIZE; j++) {
                    sector_buffer[j] = 0;
                }

                uint16_t offset = (uint16_t)s * SECTOR_SIZE;
                uint16_t bytes  = size - offset;
                if(bytes > SECTOR_SIZE) bytes = SECTOR_SIZE;

                for(int j = 0; j < bytes; j++) {
                    sector_buffer[j] = data[offset + j];
                }

                write_sector(base + (uint32_t)s, sector_buffer);
            }

            header.file_count++;
            write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
            write_sector(ESFS_TABLE_SECTOR,  (uint8_t*)table);

            return true;
        }
    }
    return false;
}

bool esfs_load(const char* name, char* buffer) {
    for(int i = 0; i < ESFS_MAX_FILES; i++) {
        if(table[i].used && strcmp_simple(table[i].name, name) == 0) {
            uint32_t base         = ESFS_DATA_SECTOR + (uint32_t)i * ESFS_SECTORS_PER_FILE;
            uint16_t remaining    = table[i].size;
            uint16_t buf_offset   = 0;

            for(int s = 0; s < table[i].sectors; s++) {
                uint8_t sector_buffer[SECTOR_SIZE];
                read_sector(base + (uint32_t)s, sector_buffer);

                uint16_t bytes = remaining > SECTOR_SIZE ? SECTOR_SIZE : remaining;

                for(int j = 0; j < bytes; j++) {
                    buffer[buf_offset++] = sector_buffer[j];
                }

                remaining -= bytes;
            }

            buffer[table[i].size] = '\0';
            return true;
        }
    }
    return false;
}

bool esfs_delete(const char* name) {
    for(int i = 0; i < ESFS_MAX_FILES; i++) {
        if(table[i].used && strcmp_simple(table[i].name, name) == 0) {
            table[i].used    = 0;
            table[i].size    = 0;
            table[i].sectors = 0;
            table[i].name[0] = '\0';

            if(header.file_count > 0) header.file_count--;

            write_sector(ESFS_HEADER_SECTOR, (uint8_t*)&header);
            write_sector(ESFS_TABLE_SECTOR,  (uint8_t*)table);

            return true;
        }
    }
    return false;
}

void esfs_list() {
    for(int i = 0; i < ESFS_MAX_FILES; i++) {
        if(table[i].used) {
            print(table[i].name, 0x0A);
            print("\n", 0x0F);
        }
    }
}

bool esfs_is_formatted() {
    if (header.magic[0] == 'E' && header.magic[1] == 'S') {
        return true; 
    }
    return false;    
}