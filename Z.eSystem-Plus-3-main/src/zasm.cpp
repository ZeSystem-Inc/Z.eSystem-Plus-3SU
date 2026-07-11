#include "zasm.hpp"
#include "lnc.hpp"
#include "esfs.hpp"

#define MAX_FONKSIYON 4096
#define MAX_ISIM_LEN  32

struct Fonksiyon {
    char isim[MAX_ISIM_LEN];
    uint32_t adres;
};

struct RegisterInfo {
    char isim[4];
    uint8_t kod;
    uint8_t boyut;
};

Fonksiyon fonksiyonTablosu[MAX_FONKSIYON];
uint32_t toplamFonksiyon = 0;

const RegisterInfo regTablosu[] = {
    {"eax", 0, 32}, {"ecx", 1, 32}, {"edx", 2, 32}, {"ebx", 3, 32}, {"esp", 4, 32},
    {"ax",  0, 16}, {"cx",  1, 16}, {"dx",  2, 16}, {"bx",  3, 16},
    {"al",  0, 8},  {"cl",  1, 8},  {"dl",  2, 8},  {"bl",  3, 8},
    {"ah",  4, 8},  {"ch",  5, 8},  {"dh",  6, 8},  {"bh",  7, 8}
};
const uint32_t toplamRegister = sizeof(regTablosu) / sizeof(RegisterInfo);

extern "C" {

    uint32_t strUzunluk(const char* s) {
        uint32_t len = 0;
        while (s[len] != '\0') len++;
        return len;
    }

    int strEsit(const char* s1, const char* s2, uint32_t n) {
        for (uint32_t i = 0; i < n; i++) {
            if (s1[i] != s2[i]) return 1; 
            if (s1[i] == '\0' || s2[i] == '\0') break;
        }
        return 0; 
    }

    void strKopyala(char* hedef, const char* kaynak, uint32_t maxLen) {
        uint32_t i = 0;
        while (kaynak[i] != '\0' && kaynak[i] != ' ' && kaynak[i] != '\t' && kaynak[i] != '-' && i < maxLen - 1) {
            hedef[i] = kaynak[i];
            i++;
        }
        hedef[i] = '\0';
    }

    uint32_t sayiParcala(const char* str, uint32_t len) {
        uint32_t val = 0;
        if (len > 2 && str[0] == '0' && str[1] == 'x') {
            for (uint32_t i = 2; i < len; i++) {
                char c = str[i];
                val *= 16;
                if (c >= '0' && c <= '9') val += (c - '0');
                else if (c >= 'a' && c <= 'f') val += (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') val += (c - 'A' + 10);
            }
        } else {
            for (uint32_t i = 0; i < len; i++) {
                if (str[i] >= '0' && str[i] <= '9') {
                    val = val * 10 + (str[i] - '0');
                }
            }
        }
        return val;
    }

    const char* sonrakiKelimedenAl(const char* tarayici, char* tampon, uint32_t* kelimeLen) {
        while (*tarayici == ' ' || *tarayici == '\t' || *tarayici == ',' || *tarayici == ';') tarayici++;
        
        uint32_t len = 0;
        while (*tarayici != '\0' && *tarayici != ' ' && *tarayici != '\t' && 
               *tarayici != ','  && *tarayici != '\n' && *tarayici != '\r' && *tarayici != ';') {
            if (len < 31) {
                tampon[len] = *tarayici;
                len++;
            }
            tarayici++;
        }
        tampon[len] = '\0';
        *kelimeLen = len;
        return tarayici;
    }

    bool regBul(const char* isim, RegisterInfo* sonuc) {
        for (uint32_t i = 0; i < toplamRegister; i++) {
            if (strEsit(regTablosu[i].isim, isim, 4) == 0) { 
                *sonuc = regTablosu[i];
                return true;
            }
        }
        return false;
    }

    int32_t fonkAdresBul(const char* isim) {
        for (uint32_t i = 0; i < toplamFonksiyon; i++) {
            if (strEsit(fonksiyonTablosu[i].isim, isim, MAX_ISIM_LEN) == 0) { 
                return fonksiyonTablosu[i].adres;
            }
        }
        return -1;
    }

    uint32_t zasmGecisIslemi(const char* kaynak, uint8_t* cikis, bool passModu) {
        uint32_t i = 0;
        uint32_t akim_adresi = 0;
        
        while (kaynak[i] != '\0') {
            while (kaynak[i] == ' ' || kaynak[i] == '\t') i++;
            
            if (kaynak[i] == '\n' || kaynak[i] == '\r' || kaynak[i] == ';') {
                while (kaynak[i] != '\n' && kaynak[i] != '\r' && kaynak[i] != '\0') i++;
                if (kaynak[i] != '\0') i++;
                continue;
            }

            uint32_t satir_sonu = i;
            while (kaynak[satir_sonu] != '\n' && kaynak[satir_sonu] != '\r' && kaynak[satir_sonu] != '\0') {
                satir_sonu++;
            }

            if (kaynak[i] == '-' && (kaynak[i+1] == '\n' || kaynak[i+1] == '\r' || kaynak[i+1] == '\0' || kaynak[i+1] == ' ' || kaynak[i+1] == '\t')) {
                i = satir_sonu;
                if (kaynak[i] != '\0') i++;
                continue;
            }

            uint32_t son_karakter = satir_sonu - 1;
            while (son_karakter > i && (kaynak[son_karakter] == ' ' || kaynak[son_karakter] == '\t')) son_karakter--;

            if (kaynak[son_karakter] == '-' && son_karakter > i) {
                if (!passModu && toplamFonksiyon < MAX_FONKSIYON) {
                    strKopyala(fonksiyonTablosu[toplamFonksiyon].isim, &kaynak[i], MAX_ISIM_LEN);
                    fonksiyonTablosu[toplamFonksiyon].adres = akim_adresi;
                    toplamFonksiyon++;
                }
                i = satir_sonu;
                if (kaynak[i] != '\0') i++;
                continue;
            }

            char komut[32];
            uint32_t kLen = 0;
            const char* satir_tarayici = sonrakiKelimedenAl(&kaynak[i], komut, &kLen);

            if (strEsit(komut, "ret", 3) == 0) {
                if (passModu) {
                    cikis[akim_adresi] = 0xC3;
                }
                akim_adresi += 1;
            }
            
            else if (strEsit(komut, "mov", 3) == 0) {
                char regIsim[32]; char degIsim[32];
                uint32_t rLen, dLen;
                satir_tarayici = sonrakiKelimedenAl(satir_tarayici, regIsim, &rLen);
                satir_tarayici = sonrakiKelimedenAl(satir_tarayici, degIsim, &dLen);

                RegisterInfo r;
                if (regBul(regIsim, &r)) {
                    uint32_t sayi = sayiParcala(degIsim, dLen);
                    
                    if (r.boyut == 32) {
                        if (passModu) {
                            cikis[akim_adresi] = 0xB8 + r.kod;
                            *(uint32_t*)(&cikis[akim_adresi + 1]) = sayi;
                        }
                        akim_adresi += 5;
                    } 
                    else if (r.boyut == 16) {
                        if (passModu) {
                            cikis[akim_adresi] = 0x66;
                            cikis[akim_adresi + 1] = 0xB8 + r.kod;
                            *(uint16_t*)(&cikis[akim_adresi + 2]) = (uint16_t)sayi;
                        }
                        akim_adresi += 4;
                    } 
                    else if (r.boyut == 8) {
                        if (passModu) {
                            cikis[akim_adresi] = 0xB0 + r.kod;
                            cikis[akim_adresi + 1] = (uint8_t)sayi;
                        }
                        akim_adresi += 2;
                    }
                }
            }

            else if (strEsit(komut, "xor", 3) == 0) {
                char reg1Isim[32]; char reg2Isim[32];
                uint32_t r1Len, r2Len;
                satir_tarayici = sonrakiKelimedenAl(satir_tarayici, reg1Isim, &r1Len);
                satir_tarayici = sonrakiKelimedenAl(satir_tarayici, reg2Isim, &r2Len);

                RegisterInfo r1, r2;
                if (regBul(reg1Isim, &r1) && regBul(reg2Isim, &r2)) {
                    uint8_t modRM = 0xC0 + (r2.kod * 8) + r1.kod;
                    
                    if (r1.boyut == 32) {
                        if (passModu) {
                            cikis[akim_adresi] = 0x31;
                            cikis[akim_adresi + 1] = modRM;
                        }
                        akim_adresi += 2;
                    }
                    else if (r1.boyut == 16) {
                        if (passModu) {
                            cikis[akim_adresi] = 0x66;
                            cikis[akim_adresi + 1] = 0x31;
                            cikis[akim_adresi + 2] = modRM;
                        }
                        akim_adresi += 3;
                    }
                    else if (r1.boyut == 8) {
                        if (passModu) {
                            cikis[akim_adresi] = 0x30;
                            cikis[akim_adresi + 1] = modRM;
                        }
                        akim_adresi += 2;
                    }
                }
            }

            else if (strEsit(komut, "call", 4) == 0) {
                char hedefFonk[32];
                uint32_t fLen;
                satir_tarayici = sonrakiKelimedenAl(satir_tarayici, hedefFonk, &fLen);

                if (passModu) {
                    int32_t hedef_adres = fonkAdresBul(hedefFonk);
                    if (hedef_adres == -1) {
                        return 0; 
                    }
                    cikis[akim_adresi] = 0xE8;
                    int32_t goreli_ofset = hedef_adres - (akim_adresi + 5);
                    *(int32_t*)(&cikis[akim_adresi + 1]) = goreli_ofset;
                }
                akim_adresi += 5; 
            }

            i = satir_sonu;
            if (kaynak[i] != '\0') i++;
        }
        
        return akim_adresi;
    }
}

uint32_t zasmDerle(const char* kaynakKod, uint8_t* cikisBinary) {
    toplamFonksiyon = 0;
    uint32_t kontrol = zasmGecisIslemi(kaynakKod, cikisBinary, false);
    if (kontrol == 0) return 0; 
    return zasmGecisIslemi(kaynakKod, cikisBinary, true);
}

bool zasmDerle(const char* kaynakDosyaAdi, const char* hedefDosyaAdi, const char* format, char* kaynakKodBuffer, uint8_t* cikisBinaryBuffer) {
    FileEntry dosya_bilgisi;
    if (!esfs_find_file(kaynakDosyaAdi, &dosya_bilgisi, 0, 0)) return false;
    
    uint32_t dosyaBoyutu = dosya_bilgisi.size;
    if (dosyaBoyutu == 0) return false; 

    if (!esfs_load(kaynakDosyaAdi, kaynakKodBuffer)) return false; 
    kaynakKodBuffer[dosyaBoyutu] = '\0'; 

    uint32_t baslangicOfseti = 0;

    uint32_t isimLen = strUzunluk(hedefDosyaAdi);
	bool lncUzantisi = (isimLen > 4 && 
							hedefDosyaAdi[isimLen-4] == '.' && 
							(hedefDosyaAdi[isimLen-3] == 'l' || hedefDosyaAdi[isimLen-3] == 'L') && 
							(hedefDosyaAdi[isimLen-2] == 'n' || hedefDosyaAdi[isimLen-2] == 'N') && 
							(hedefDosyaAdi[isimLen-1] == 'c' || hedefDosyaAdi[isimLen-1] == 'C'));

    if (strEsit(format, "lnc", 3) == 0 || lncUzantisi) {
        baslangicOfseti = sizeof(LNC_Header); 
    }

    uint32_t uretilenBoyut = zasmDerle(kaynakKodBuffer, cikisBinaryBuffer + baslangicOfseti);
    if (uretilenBoyut == 0) return false; 

    if (strEsit(format, "lnc", 3) == 0 || lncUzantisi) {
        LNC_Header* header = (LNC_Header*)cikisBinaryBuffer;
        
        header->magic = LNC_MAGIC;
        header->entry_offset = sizeof(LNC_Header);
        header->text_size = uretilenBoyut;
        header->data_size = 0;
        header->heap_required = 4096;
    }

    uint32_t toplamBoyut = uretilenBoyut + baslangicOfseti;
    if (!esfs_save(hedefDosyaAdi, (const char*)cikisBinaryBuffer, toplamBoyut)) return false; 

    return true; 
}