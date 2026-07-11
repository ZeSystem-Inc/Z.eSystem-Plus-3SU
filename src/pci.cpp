#include "pci.hpp"

extern "C" void print(const char* str, unsigned char color);

#define PCI_CONFIG_ADRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static inline void outl(uint16_t port, uint32_t value) {
	 asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t adress =
		(1u << 31) |
		((uint32_t)bus << 16) |
		((uint32_t)slot << 11) |
		((uint32_t)func << 8) |
		(offset & 0xFC);
		
		outl(PCI_CONFIG_ADRESS, adress);
		return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t data = pci_read_dword(bus, slot, func, offset);

    if (offset & 2) {
        return (data >> 16) & 0xFFFF;
	} else {
        return data & 0xFFFF;
	}
}

bool pci_find_storage(PCI_Device* out_device) {
	for(uint16_t bus = 0; bus < 256; bus++) {
		for(uint8_t slot = 0; slot < 32; slot++) {
			for(uint8_t func = 0; func < 8; func++) {
				uint16_t vendor = pci_read_word(bus, slot, func, 0x00);
				if(vendor == 0xFFFF) continue;
				
				uint16_t device = pci_read_word(bus, slot, func, 0x02);
				uint32_t class_reg = pci_read_dword(bus, slot, func, 0x08);
				
				uint8_t class_code = (class_reg >> 24) & 0xFF;
				uint8_t subclass = (class_reg >> 16) & 0xFF;
				uint8_t prog_if = (class_reg >> 8) & 0xFF;
				
				if(class_code == 0x01) {
					out_device->bus = bus;
					out_device->device = slot;
					out_device->function = func;
					out_device->vendor_id = vendor;
					out_device->device_id = device;
					out_device->class_code = class_code;
					out_device->subclass = subclass;
					out_device->prog_if = prog_if;
					return true;
				}
			}
		}
	}
	
	return false;
}

void pci_scan() {
	PCI_Device dev;
	uint16_t vendor = pci_read_word(0, 0, 0, 0x00);
	
	if(vendor == 0xFFFF) {
		print("PCI FAIL!\n", 0x0C);
	} else {
		print("PCI OK!\n", 0x0A);
	}
	
	if(pci_find_storage(&dev)) {
		print("PCI controller OK!\n", 0x0A);
	} else {
		print("PCI controller not found!\n", 0x0C);
	}
}