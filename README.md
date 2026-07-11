# ZeSystemPlus3

A hobby x86 (32-bit protected mode) operating system written entirely from scratch, with no third-party dependencies (no GRUB, no Limine). It's the latest link in the ZeSystem lineage (Z.eSystem → ZeSystemPlus → ZeSystemPlus2 → **ZeSystemPlus3**).

> Built on a "write it yourself" philosophy: bootloader, filesystem, assembler, and executable format are all hand-written, layer by layer.

## Features

- **Hand-written NASM bootloader** — no GRUB/Limine; disk reads and the switch to protected mode are all done in `boot.asm`
- **IDT-based interrupt handling** — PIC remapping, PIT timer, and an interrupt-driven PS/2 keyboard driver (`idt.asm`)
- **ESFS** — custom filesystem with dynamic sector allocation, supporting files up to ~4GB (`uint32_t` size field)
- **ATA PIO disk driver** (`disk.cpp`)
- **ACPI support** (`acpi.cpp`) and **PCI scanning** (`pci.cpp`)
- **ZASM** — a custom-written assembler (`zasm.cpp`) that produces the custom **LNC** executable format (`lnc.cpp`, magic: `0x434E4C02`)
- **Shell** — comes with `help`, `clear`, `version`, `color`, `shutdown`, `reboot`, `editor`, `cat`, `ls`, `rm`, `zasm`, `runl`, `format`, `install` commands, including a simple built-in text editor (`editor`)
- **Installer** (`installer.cpp`) — copies the bootloader and ESFS sectors from USB to the hard disk

## Project Structure

```
ZeSystemPlus/
├── boot.asm             # Bootloader (16-bit real mode -> 32-bit protected mode)
├── kernel_entry.asm      # Kernel entry point
├── idt.asm               # IDT / PIC / PIT / keyboard interrupt handlers
├── kernel.cpp/.hpp        # Kernel: screen output, shell, command parsing
├── disk.cpp/.hpp          # ATA PIO disk driver
├── esfs.cpp/.hpp          # ESFS custom filesystem
├── acpi.cpp/.hpp          # ACPI support
├── pci.cpp/.hpp           # PCI device scanning
├── zasm.cpp/.hpp          # ZASM assembler
├── lnc.cpp/.hpp           # LNC executable format loader
├── installer.cpp/.hpp     # Hard disk installer
├── linker.ld              # Linker script
├── derleme_komutu.txt     # Build and QEMU run commands
└── os.img                 # Prebuilt bootable disk image (boot sector + kernel)
```

## Requirements

- NASM
- g++ (with 32-bit target support, `-m32`)
- GNU `ld`
- QEMU (`qemu-system-i386`, `qemu-img`)

Development environment: Windows 10 + WSL1 for compilation; QEMU is also run from WSL1 to avoid dual-boot risk while keeping access to the Linux toolchain.

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/<your-username>/ZeSystemPlus3.git
cd ZeSystemPlus3/ZeSystemPlus
```

### 2. Install the toolchain (WSL1 / Ubuntu example)

```bash
sudo apt update
sudo apt install nasm g++-multilib qemu-system-x86 -y
```

### 3. Build from source

All steps below are listed in order in `derleme_komutu.txt`. Clean any previous build artifacts first, then assemble, compile, and link:

```bash
# Clean previous build outputs
rm -f *.o *.bin os.img hdd.img

# Assemble the bootloader and low-level ASM parts
nasm -f bin boot.asm -o boot.bin
nasm -f elf32 kernel_entry.asm -o kernel_entry.o
nasm -f elf32 idt.asm -o idt.o

# Compile the C++ sources (freestanding, 32-bit, no PIE/RTTI/exceptions)
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c kernel.cpp -o kernel.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c disk.cpp -o disk.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c esfs.cpp -o esfs.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c installer.cpp -o installer.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c pci.cpp -o pci.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c lnc.cpp -o lnc.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c zasm.cpp -o zasm.o
g++ -m32 -ffreestanding -fno-pie -fno-rtti -fno-exceptions -c acpi.cpp -o acpi.o

# Link everything into a flat binary kernel image
ld -m elf_i386 -T linker.ld --oformat binary kernel_entry.o idt.o acpi.o kernel.o disk.o esfs.o installer.o pci.o lnc.o zasm.o -o kernel.bin

# Pad the kernel and assemble the final bootable disk image
truncate -s 25600 kernel.bin
cat boot.bin kernel.bin > os.img
truncate -s 10M os.img
```

At this point, `os.img` is the bootable disk image (boot sector + kernel). This is the same file already committed to this repository — you only need to rebuild it if you've changed the source.

### 4. Create a secondary storage disk (for ESFS)

`os.img` is the boot/kernel disk; ESFS data lives on a second, separate disk image that isn't checked into the repo (a fresh one is created locally instead):

```bash
qemu-img create -f raw hdd.img 10M
```

### 5. Run in QEMU

With both disks attached (`hdd.img` as the data disk, `os.img` as the bootable disk):

```bash
qemu-system-i386 -drive file=hdd.img,format=raw,if=none,id=drive-hdd -device ide-hd,bus=ide.0,unit=0,drive=drive-hdd -drive file=os.img,format=raw,if=none,id=drive-os -device ide-hd,bus=ide.0,unit=1,drive=drive-os,bootindex=1
```

Or, to boot from `hdd.img` alone as a single disk:

```bash
qemu-system-i386 -drive file=hdd.img,format=raw,index=0,media=disk
```

> Note: `hdd.img` is a local, regenerable data disk and is intentionally not tracked in this repository. `os.img` is committed since it's the bootable output needed to run the system without a full rebuild.

## License

Not specified yet — a `LICENSE` file (e.g. MPL 2.0, as used in Plus2) can be added if desired.

## Contributing

This is an individual hobby/learning project. Feedback and issues are welcome.

__NOTE:__ Scrolling has been removed in this version. It will return in future versions.
