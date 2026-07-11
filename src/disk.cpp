#include "disk.hpp"


static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

void read_sector(uint32_t lba, uint8_t* buffer) {
    read_sector(0xE0, lba, buffer);
}

void write_sector(uint32_t lba, const uint8_t* buffer) {
    write_sector(0xE0, lba, buffer);
}

void read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer) {
    outb(0x1F6, drive | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); 
	
    while ((inb(0x1F7) & 0x80));
    while (!(inb(0x1F7) & 0x08));

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }
}

void write_sector(uint8_t drive, uint32_t lba, const uint8_t* buffer) {
    outb(0x1F6, drive | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30); 

    while ((inb(0x1F7) & 0x80));
    while (!(inb(0x1F7) & 0x08));

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, ptr[i]);
    }
    
    outb(0x1F7, 0xE7);
    while ((inb(0x1F7) & 0x80));
}