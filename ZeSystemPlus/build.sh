#!/bin/bash

set -e

echo "=== Eski dosyalar temizleniyor ==="
rm -f *.o *.bin os.img hdd.img *.o* *.bin* os.img* hdd.img*

echo "=== Assembly dosyaları derleniyor ==="
nasm -f bin boot.asm -o boot.bin
nasm -f elf32 kernel_entry.asm -o kernel_entry.o
nasm -f elf32 idt.asm -o idt.o

echo "=== C++ kaynak dosyaları derleniyor ==="
GPP_FLAGS="-m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c"
g++ $GPP_FLAGS kernel.cpp -o kernel.o
g++ $GPP_FLAGS disk.cpp -o disk.o
g++ $GPP_FLAGS esfs.cpp -o esfs.o
g++ $GPP_FLAGS installer.cpp -o installer.o
g++ $GPP_FLAGS pci.cpp -o pci.o
g++ $GPP_FLAGS lnc.cpp -o lnc.o
g++ $GPP_FLAGS zasm.cpp -o zasm.o
g++ $GPP_FLAGS acpi.cpp -o acpi.o

echo "=== Linker (Bağlayıcı) çalıştırılıyor ==="
ld -m elf_i386 -T linker.ld --oformat binary kernel_entry.o idt.o acpi.o kernel.o disk.o esfs.o installer.o pci.o lnc.o zasm.o -o kernel.bin

echo "=== İmaj dosyaları hazırlanıyor ==="
truncate -s 25600 kernel.bin
cat boot.bin kernel.bin > os.img
truncate -s 10M os.img

qemu-img create -f raw hdd.img 10M

echo "=== OS Başlatılıyor... ==="
qemu-system-i386 -drive file=hdd.img,format=raw,if=none,id=drive-hdd -device ide-hd,bus=ide.0,unit=0,drive=drive-hdd -drive file=os.img,format=raw,if=none,id=drive-os -device ide-hd,bus=ide.0,unit=1,drive=drive-os,bootindex=1