#ifndef ESFS_HPP
#define ESFS_HPP

#include <stdint.h>

#define ESFS_MAX_FILES 16

extern "C" void print(const char* str, unsigned char color);

struct ESFSHeader {
	char magic[4];
	uint32_t version;
	uint32_t file_count;
};

struct FileEntry {
	char name[16];
	uint8_t used;
	uint16_t size;
	uint16_t sectors;
};

void esfs_init();
void esfs_format();

bool esfs_save(const char* name, const char* data, uint16_t size);
bool esfs_load(const char* name, char* buffer);
bool esfs_delete(const char* name);
void esfs_list();
bool esfs_is_formatted();

#endif