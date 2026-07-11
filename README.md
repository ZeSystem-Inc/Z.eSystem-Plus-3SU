# Z.eSystem Plus 3SU

This system is a version of the Z.eSystem Plus 3 that has received minor updates.

A operating system written from scratch in **Assembly** and **C++** for the **x86 BIOS** platform.

> **Status:** Active Development

---

## About

Z.eSystem Plus is an educational operating system project developed to explore low-level programming, x86 architecture, bootloaders, kernel development, and hardware interaction.

The project is written without using an existing operating system kernel and runs directly on the hardware through the BIOS boot process.

## Features

* x86 BIOS Bootloader
* 32-bit Protected Mode Kernel
* Assembly + C++ architecture
* VGA Text Mode console
* Keyboard input
* Shell
* ESFS (Experimental File System)
* IDE Disk Driver
* PCI Detection
* ACPI Initialization
* ZASM Module
* LNC Module
* Simple Installer
* Modular source structure

## Project Structure

```text
boot.asm              BIOS bootloader
kernel_entry.asm      Kernel entry point
kernel.cpp            Main kernel
idt.asm               Interrupt Descriptor Table

disk.cpp/.hpp         Disk driver
esfs.cpp/.hpp         ESFS file system
installer.cpp/.hpp    Installer
pci.cpp/.hpp          PCI support
acpi.cpp/.hpp         ACPI support
lnc.cpp/.hpp          LNC module
zasm.cpp/.hpp         ZASM module

linker.ld             Linker script
build.sh              Build script
```

## Requirements

* Linux or WSL
* NASM
* GCC (32-bit support)
* GNU LD
* QEMU
* qemu-img

## Build

The project includes an automatic build script.

```bash
chmod +x build.sh
./build.sh
```

The script automatically:

* Cleans old build files
* Compiles Assembly sources
* Compiles C++ sources
* Links the kernel
* Creates the bootable OS image
* Creates a virtual hard disk
* Starts QEMU

## Architecture

```text
BIOS
 │
 ▼
Bootloader (boot.asm)
 │
 ▼
Protected Mode
 │
 ▼
Kernel Entry
 │
 ▼
Kernel
 ├── VGA Console
 ├── Keyboard
 ├── ESFS
 ├── Disk Driver
 ├── PCI
 ├── ACPI
 ├── Installer
 ├── LNC
 └── ZASM
```

## Goals

The main goals of this project are:

* Learn operating system development
* Understand x86 architecture
* Experiment with hardware programming
* Develop a custom file system
* Keep the kernel simple and modular

## License

See the **LICENSE** file for licensing information.

## Disclaimer

Z.eSystem Plus is a hobby operating system intended for educational and experimental purposes. It is **not** designed for production use.

---

**Made with Assembly, C++ and curiosity.**
