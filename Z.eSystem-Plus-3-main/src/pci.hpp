#ifndef PCI_HPP
#define PCI_HPP

#include <stdint.h>

struct PCI_Device {
	uint8_t bus;
	uint8_t device;
	uint8_t function;
	
	uint16_t vendor_id;
	uint16_t device_id;
	
	uint8_t class_code;
	uint8_t subclass;
	uint8_t prog_if;
};

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

bool pci_find_storage(PCI_Device* out_device);
void pci_scan();

#endif