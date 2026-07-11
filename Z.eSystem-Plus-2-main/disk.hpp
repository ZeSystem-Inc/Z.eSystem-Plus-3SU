#ifndef DISK_HPP
#define DISK_HPP

#include <stdint.h>

#define SECTOR_SIZE 512
#define TOTAL_SECTORS 256

void read_sector(uint32_t lba, uint8_t* buffer);
void write_sector(uint32_t lba, const uint8_t* buffer);

void read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer);
void write_sector(uint8_t drive, uint32_t lba, const uint8_t* buffer);

#endif