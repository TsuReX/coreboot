# StarBook Mk V

## Specs

- CPU (full processor specs available at <https://ark.intel.com>)
    - Intel 165H (Meteor Lake)
- EC
    - ITE IT5570E
    - Backlit keyboard, with standard PS/2 keycodes and SCI hotkeys
    - Battery
    - USB-C PD Charger
    - Suspend / resume
- GPU
    - Intel® Arc® Graphics
    - GOP driver is recommended, VBT is provided
    - eDP 14-inch 3840x2160 LCD
    - HDMI video
    - USB-C DisplayPort video
- Memory
    - 2 x DDR5 SODIMM
- Networking
    - AX210 2230 WiFi / Bluetooth
- Sound
    - Realtek ALC269-VB6
    - Internal speakers
    - Internal microphone
    - Combined headphone / microphone 3.5-mm jack
    - HDMI audio
    - USB-C DisplayPort audio
- Storage
    - M.2 PCIe SSD
    - RTS5129 MicroSD card reader
- USB
    - 1920x1080 CCD camera
    - USB 3.1 Gen 2 (left)
    - USB 3.1 Gen 2 Type-A (left)
    - USB 3.1 Gen 1 Type-A (right)
    - USB 2.0 Type-A (right)

## Building coreboot

Please follow the [Star Labs build instructions](common/building.md) to build coreboot, using `config.starlabs_starbook_adl` as config file.

### Preliminaries

Prior to building coreboot the following files are required:
* Intel Flash Descriptor file (descriptor.bin)
* Intel Management Engine firmware (me.bin)
* ITE Embedded Controller firmware (ec.bin)

The files listed below are optional:
- Splash screen image in Windows 3.1 BMP format (Logo.bmp)

These files exist in the correct location in the StarLabsLtd/blobs repo on GitHub which is used in place of the standard 3rdparty/blobs repo.

### Build

The following commands will build a working image:

```bash
make distclean
make defconfig KBUILD_DEFCONFIG=configs/config.starlabs_starbook_mtl
make
```

## Flashing coreboot

```{eval-rst}
+---------------------+------------+
| Type                | Value      |
+=====================+============+
| Socketed flash      | no         |
+---------------------+------------+
| Vendor              | Winbond    |
+---------------------+------------+
| Model               | W25Q256JW  |
+---------------------+------------+
| Size                | 32 MiB     |
+---------------------+------------+
| Package             | SOIC-8     |
+---------------------+------------+
| Internal flashing   | yes        |
+---------------------+------------+
| External flashing   | yes        |
+---------------------+------------+
```

Please see [here](common/flashing.md) for instructions on how to flash with fwupd.
