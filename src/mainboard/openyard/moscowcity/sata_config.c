#include <string.h>
#include <sata_config.h>

#include <drivers/efi/efivars.h>
#include <fmap.h>

#define __SIMPLE_DEVICE__

#include "device/pci_type.h"
#include "device/pci_ops.h"

static void print_debug_information (void) {
		/* PCI_DEV(SEGBUS, DEV, FN) */
		pci_devfn_t dev = PCI_DEV(0x0, 0x17, 0x0);
/*
		Emmitsburg Platform Controller Hub
		External Design Specification
		Reference Number: 606161
		23.8.1 SATA Configuration Registers Summary
*/
		uint32_t ret_val = pci_s_read_config32(dev, 0x94);
		printk(BIOS_DEBUG, "Sata controller 0 Port Control and Status: 0x%08X\n", ret_val);
		pci_write_config32(dev, 0x94, 0xFF);

		ret_val = pci_s_read_config32(dev, 0x94);
    printk(BIOS_DEBUG, "Sata controller 0 Port Control and Status: 0x%08X\n", ret_val);

		ret_val = pci_s_read_config32(dev, 0x0);
    printk(BIOS_DEBUG, "Sata controller 0 Identifiers: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x10);
    printk(BIOS_DEBUG, "Sata controller 0 BAR0: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x14);
    printk(BIOS_DEBUG, "Sata controller 0 BAR1: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x18);
    printk(BIOS_DEBUG, "Sata controller 0 BAR2: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x1C);
    printk(BIOS_DEBUG, "Sata controller 0 BAR3: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x20);
    printk(BIOS_DEBUG, "Sata controller 0 BAR4: 0x%08X\n", ret_val);

    ret_val = pci_s_read_config32(dev, 0x24);
    // See more information here https://wiki.osdev.org/AHCI#AHCI_Registers_and_Memory_Structures
		printk(BIOS_DEBUG, "Sata controller 0 BAR5(AHCI Base Memory Register): 0x%08X\n", ret_val);

		dev = PCI_DEV(0x0, 0x18, 0x0);
		ret_val = pci_s_read_config32(dev, 0x94);
    printk(BIOS_DEBUG, "Sata controller 1 Port Control and Status: 0x%08X\n", ret_val);
		pci_write_config32(dev, 0x94, 0xFF);

    ret_val = pci_s_read_config32(dev, 0x0);
    printk(BIOS_DEBUG, "Sata controller 1 Identifiers: 0x%08X\n", ret_val);

		dev = PCI_DEV(0x0, 0x19, 0x0);
		ret_val = pci_s_read_config32(dev, 0x94);
    printk(BIOS_DEBUG, "Sata controller 2 Port Control and Status: 0x%08X\n", ret_val);
		pci_write_config32(dev, 0x94, 0xFF);

    ret_val = pci_s_read_config32(dev, 0x0);
    printk(BIOS_DEBUG, "Sata controller 2 Identifiers: 0x%08X\n", ret_val);

}


int32_t sata_setup(FSPM_UPD *mupd) {
    const EFI_GUID var_guid = SATA_VAR_GUID;
    char var_name[128];
		strcpy(var_name, SATA_VAR_NAME);
    struct region_device smmstore;
		struct sata_config sata_config;
    uint32_t sata_config_size = sizeof(struct sata_config);

		if (mupd == NULL)
				return -1;

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -2;
    }

		enum cb_err error = efi_fv_get_option(&smmstore, &var_guid, var_name, &sata_config, &sata_config_size);
    if (error != 0) {
        printk(BIOS_WARNING, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
        sata_config.Port1 = 0;
				sata_config.Port2 = 0;
    } else {
        printk(BIOS_DEBUG, "SATA configuration:\n");
        printk(BIOS_DEBUG, "Port 1: 0x%01X\n", sata_config.Port1);
				printk(BIOS_DEBUG, "Port 2: 0x%01X\n", sata_config.Port2);
    }
		
		// TODO Hand off SATA configuration to ????
 
		if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -3;
    }
    error = efi_fv_set_option(&smmstore, &var_guid, var_name, &sata_config, sata_config_size);
    if (error != 0)
        printk(BIOS_WARNING, "EFI variable %s can't be set up, error: %d\n", var_name, error);
		print_debug_information();
    return 0;
}
