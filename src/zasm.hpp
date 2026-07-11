#ifndef ZASM_HPP
#define ZASM_HPP

#include <stdint.h>

uint32_t zasmDerle(const char* kaynakKod, uint8_t cikisBinary);
bool zasmDerle(const char* kaynakDosyaAdi, const char* hedefDosyaAdi, const char* format, char* kaynakKodBuffer, uint8_t* cikisBinaryBuffer);

#endif