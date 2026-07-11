# Z.eSystem Plus 2

## What's New:
1. File system implemented
2. Scrolling support added
3. File saving, loading, and deletion features implemented
4. The system can now be installed onto a persistent disk

## Requirements:
* RAM: 120MB or higher (Recommended)
* Disk: IDE
* Storage: 10MB or higher (Recommended)

### How to Install:

#### Part 1: Creating the Disk Image

First, create the disk image using the following command:

```bash
qemu-img create -f raw hdd.img 10M
```

#### Part 2: Installing the System

Next, boot the installation media:

```bash
qemu-system-i386 -drive file=hdd.img,format=raw,if=none,id=drive-hdd -device ide-hd,bus=ide.0,unit=0,drive=drive-hdd -drive file=os.img,format=raw,if=none,id=drive-os -device ide-hd,bus=ide.0,unit=1,drive=drive-os,bootindex=1
```

A QEMU window will appear showing the following output:

```text
PCI OK!
PCI Controller OK!
Kaydedilme basarili!
Icerik: Merhaba ESFS
Yukleme basarili!
Silme basarili oldu!
Z.eSystem Plus'a hosgeldiniz!
Yardim almak icin 'help' yazin.
```

Once Z.eSystem boots up, enter the following command:
```text
> install
```

After the installation is complete, power off the system:
```text
> shutdown
```

#### Part 3: Booting the Installed System

After shutting down, run the system from the hard disk:
```bash
qemu-system-i386 -drive file=hdd.img,format=raw,index=0,media=disk
```

You are now ready to use Z.eSystem!
