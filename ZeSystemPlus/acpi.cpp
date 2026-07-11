#include "acpi.hpp"

inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static bool kcompare_signature(const char* str1, const char* str2, int len) {
    for (int i = 0; i < len; i++) {
        if (str1[i] != str2[i]) return false;
    }
    return true;
}

static bool kverify_checksum(uint8_t* address, int len) {
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += address[i];
    }
    return (sum == 0);
}

static FADT* g_fadt = nullptr;
static uint16_t g_slp_typa = 0;
static uint16_t g_slp_typb = 0;
static bool g_acpi_valid = false;

void init_acpi() {
    uint8_t* rsdp_ptr = (uint8_t*)0x000E0000;
    RSDPDescriptor* rsdp = nullptr;

    while ((uint32_t)rsdp_ptr < 0x000FFFFF) {
        if (kcompare_signature((const char*)rsdp_ptr, "RSD PTR ", 8)) {
            if (kverify_checksum(rsdp_ptr, sizeof(RSDPDescriptor))) {
                rsdp = (RSDPDescriptor*)rsdp_ptr;
                break;
            }
        }
        rsdp_ptr += 16;
    }

    if (!rsdp) return;

    ACPISDTHeader* rsdt = (ACPISDTHeader*)rsdp->RsdtAddress;
    if (!kverify_checksum((uint8_t*)rsdt, rsdt->Length)) return;

    int total_tables = (rsdt->Length - sizeof(ACPISDTHeader)) / 4;
    uint32_t* table_array = (uint32_t*)((uint32_t)rsdt + sizeof(ACPISDTHeader));

    for (int i = 0; i < total_tables; i++) {
        ACPISDTHeader* current_header = (ACPISDTHeader*)table_array[i];
        if (kcompare_signature(current_header->Signature, "FACP", 4)) {
            if (kverify_checksum((uint8_t*)current_header, current_header->Length)) {
                g_fadt = (FADT*)current_header;
                break;
            }
        }
    }

    if (!g_fadt) return;

    ACPISDTHeader* dsdt = (ACPISDTHeader*)g_fadt->Dsdt;
    if (!kverify_checksum((uint8_t*)dsdt, dsdt->Length)) return;

    char* aml_code = (char*)dsdt + sizeof(ACPISDTHeader);
    int aml_size = dsdt->Length - sizeof(ACPISDTHeader);

    for (int i = 0; i < aml_size - 4; i++) {
        if (kcompare_signature(&aml_code[i], "_S5_", 4)) {
            char* ptr = &aml_code[i + 4];

            if (*ptr == 0x12) { // PackageOp
                ptr++;
                if ((*ptr & 0xC0) == 0) ptr += 1;
                else if ((*ptr & 0xC0) == 0x40) ptr += 2;
                else if ((*ptr & 0xC0) == 0x80) ptr += 3;
                else ptr += 4;

                ptr++;

                if (*ptr == 0x0A) { g_slp_typa = *(ptr + 1); ptr += 2; }
                else { g_slp_typa = *ptr; ptr++; }

                if (*ptr == 0x0A) { g_slp_typb = *(ptr + 1); }
                else { g_slp_typb = *ptr; }

                g_acpi_valid = true;
                break;
            }
        }
    }
}

void sys_shutdown() {
    if (!g_acpi_valid) init_acpi();

    if (g_acpi_valid && g_fadt != nullptr) {
        if ((inw(g_fadt->PM1a_CNT_BLK) & 1) == 0) {
            if (g_fadt->SMI_CMD != 0 && g_fadt->ACPI_ENABLE != 0) {
                outb(g_fadt->SMI_CMD, g_fadt->ACPI_ENABLE);
                int wait_loop = 0;
                while (((inw(g_fadt->PM1a_CNT_BLK) & 1) == 0) && wait_loop < 1500) {
                    wait_loop++;
                }
            }
        }

        uint16_t pm1a_shutdown_signal = g_slp_typa | (1 << 13);
        uint16_t pm1b_shutdown_signal = g_slp_typb | (1 << 13);

        outw(g_fadt->PM1a_CNT_BLK, pm1a_shutdown_signal);
        if (g_fadt->PM1b_CNT_BLK != 0) {
            outw(g_fadt->PM1b_CNT_BLK, pm1b_shutdown_signal);
        }
    }

    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);

    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}

void sys_reboot() {
    uint8_t buffer = 0x02;
    int safety_timeout = 0;
    while ((buffer & 0x02) && safety_timeout < 1000) {
        buffer = inb(0x64);
        safety_timeout++;
    }
    outb(0x64, 0xFE);

    outb(0xCF9, 0x06);

    asm volatile("cli");
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) null_idt = {0, 0};
    asm volatile("lidt %0" : : "m"(null_idt));
    asm volatile("int $3");
}