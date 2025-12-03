#include <string.h>
#include <oysetup_config.h>

#include <drivers/efi/efivars.h>
#include <fmap.h>

int32_t oysetup_setup(FSPM_UPD *mupd) {
    const EFI_GUID var_guid = OYSETUP_VAR_GUID;
    char var_name[128];
    strcpy(var_name, OYSETUP_VAR_NAME);
    struct region_device smmstore;
    struct oysetup_config oysetup_config;
    uint32_t oysetup_config_size = sizeof(struct oysetup_config);

    if (mupd == NULL)
        return -1;

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -2;
    }

    enum cb_err error = efi_fv_get_option(&smmstore, &var_guid, var_name, &oysetup_config, &oysetup_config_size);
    if (error != 0) {
        printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
        printk(BIOS_ERR, "Default initialization oysetup_config should be done\n");
        // TODO Default initialization oysetup_config should be done
    } else {
        printk(BIOS_DEBUG, "Socket processor configuration:\n");
        printk(BIOS_DEBUG, "ConfigIOU0[0]: 0x%02X\n", oysetup_config.ConfigIOU0[0]);
        printk(BIOS_DEBUG, "ConfigIOU0[1]: 0x%02X\n", oysetup_config.ConfigIOU0[1]);
        printk(BIOS_DEBUG, "ConfigIOU1[0]: 0x%02X\n", oysetup_config.ConfigIOU1[0]);
        printk(BIOS_DEBUG, "ConfigIOU1[1]: 0x%02X\n", oysetup_config.ConfigIOU1[1]);
        printk(BIOS_DEBUG, "ConfigIOU2[0]: 0x%02X\n", oysetup_config.ConfigIOU2[0]);
        printk(BIOS_DEBUG, "ConfigIOU2[1]: 0x%02X\n", oysetup_config.ConfigIOU2[1]);
        printk(BIOS_DEBUG, "ConfigIOU3[0]: 0x%02X\n", oysetup_config.ConfigIOU3[0]);
        printk(BIOS_DEBUG, "ConfigIOU3[1]: 0x%02X\n", oysetup_config.ConfigIOU3[1]);
        printk(BIOS_DEBUG, "ConfigIOU4[0]: 0x%02X\n", oysetup_config.ConfigIOU4[0]);
        printk(BIOS_DEBUG, "ConfigIOU4[1]: 0x%02X\n", oysetup_config.ConfigIOU4[1]);
        printk(BIOS_DEBUG, "ConfigIOU5[0]: 0x%02X\n", oysetup_config.ConfigIOU5[0]);
        printk(BIOS_DEBUG, "ConfigIOU5[1]: 0x%02X\n", oysetup_config.ConfigIOU5[1]);
        printk(BIOS_DEBUG, "ConfigIOU6[0]: 0x%02X\n", oysetup_config.ConfigIOU6[0]);
        printk(BIOS_DEBUG, "ConfigIOU6[1]: 0x%02X\n", oysetup_config.ConfigIOU6[1]);

        printk(BIOS_ERR, "Hyper Threading configuration:\n");
        printk(BIOS_ERR, "ProcessorHyperThreadingDisable: 0x%01X\n", oysetup_config.ProcessorHyperThreadingDisable);

        printk(BIOS_DEBUG, "SATA configuration:\n");
        printk(BIOS_DEBUG, "SataPortController0[0]: 0x%01X\n", oysetup_config.SataPortController0[0]);
        printk(BIOS_DEBUG, "SataPortController0[7]: 0x%01X\n", oysetup_config.SataPortController0[7]);

        printk(BIOS_DEBUG, "SataPortController1[0]: 0x%01X\n", oysetup_config.SataPortController1[0]);
        printk(BIOS_DEBUG, "SataPortController1[7]: 0x%01X\n", oysetup_config.SataPortController1[7]);

        printk(BIOS_DEBUG, "SataPortController2[0]: 0x%01X\n", oysetup_config.SataPortController2[0]);
        printk(BIOS_DEBUG, "SataPortController2[7]: 0x%01X\n", oysetup_config.SataPortController2[7]);

        printk(BIOS_DEBUG, "USB configuration:\n");

        printk(BIOS_DEBUG, "PchUsbHsPort[0]: 0x%01X\n", oysetup_config.PchUsbHsPort[0]);
        printk(BIOS_DEBUG, "PchUsbHsPort[15]: 0x%01X\n", oysetup_config.PchUsbHsPort[15]);

        printk(BIOS_DEBUG, "PchUsbSsPort[0]: 0x%01X\n", oysetup_config.PchUsbSsPort[0]);
        printk(BIOS_DEBUG, "PchUsbSsPort[15]: 0x%01X\n", oysetup_config.PchUsbSsPort[9]);
    }

    mupd->FspmConfig.ProcessorHyperThreadingDisable = oysetup_config.ProcessorHyperThreadingDisable;
    // TODO Pass all parameters to FSPM

/*    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -3;
    }
    error = efi_fv_set_option(&smmstore, &var_guid, var_name, &oysetup_config, oysetup_config_size);
    if (error != 0)
      printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
*/
    return 0;
}

