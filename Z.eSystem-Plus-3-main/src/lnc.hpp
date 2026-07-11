#ifndef LNC_HPP
#define LNC_HPP

#include <stdint.h>

#define LNC_MAGIC 0x434E4C02

typedef struct {
	uint32_t magic;
	uint32_t entry_offset;
	uint32_t text_size;
	uint32_t data_size;
	uint32_t heap_required;
} __attribute__((packed)) LNC_Header;

extern "C" bool lnc_launch(const char* filename);

#endif