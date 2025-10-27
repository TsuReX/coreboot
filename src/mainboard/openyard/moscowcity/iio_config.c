#include <string.h>
#include <iio_config.h>

#include <drivers/efi/efivars.h>
#include <fmap.h>

int32_t iio_setup(FSPM_UPD *mupd) {
    const EFI_GUID var_guid = IIO_VAR_GUID;
		char var_name[128];
    strcpy(var_name, IIO_VAR_NAME);
		struct region_device smmstore;
    struct iio_config iio_config;
    uint32_t iio_config_size = sizeof(struct iio_config);

    if (mupd == NULL)
        return -1;
  
    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -2;
    }
    enum cb_err error = efi_fv_get_option(&smmstore, &var_guid, var_name, &iio_config, &iio_config_size);
    if (error != 0) {
        printk(BIOS_WARNING, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
        iio_config.ConfigIOU0[0] = 0x0;
				iio_config.ConfigIOU0[1] = 0x0;
				iio_config.ConfigIOU1[0] = 0x0;
				iio_config.ConfigIOU1[1] = 0x0;
				iio_config.ConfigIOU2[0] = 0x0;
				iio_config.ConfigIOU2[1] = 0x0;
				iio_config.ConfigIOU3[0] = 0x0;
				iio_config.ConfigIOU3[1] = 0x0;
				iio_config.ConfigIOU4[0] = 0x0;
				iio_config.ConfigIOU4[1] = 0x0;
				iio_config.ConfigIOU5[0] = 0x0;
				iio_config.ConfigIOU5[1] = 0x0;
				iio_config.ConfigIOU6[0] = 0x0;
				iio_config.ConfigIOU6[1] = 0x0;
    } else {
        printk(BIOS_DEBUG, "Socket processor configuration:\n");
        printk(BIOS_DEBUG, "ConfigIOU0[0]: 0x%02X\n", iio_config.ConfigIOU0[0]);
				printk(BIOS_DEBUG, "ConfigIOU0[1]: 0x%02X\n", iio_config.ConfigIOU0[1]);
        printk(BIOS_DEBUG, "ConfigIOU1[0]: 0x%02X\n", iio_config.ConfigIOU1[0]);
        printk(BIOS_DEBUG, "ConfigIOU1[1]: 0x%02X\n", iio_config.ConfigIOU1[1]);
        printk(BIOS_DEBUG, "ConfigIOU2[0]: 0x%02X\n", iio_config.ConfigIOU2[0]);
        printk(BIOS_DEBUG, "ConfigIOU2[1]: 0x%02X\n", iio_config.ConfigIOU2[1]);
        printk(BIOS_DEBUG, "ConfigIOU3[0]: 0x%02X\n", iio_config.ConfigIOU3[0]);
        printk(BIOS_DEBUG, "ConfigIOU3[1]: 0x%02X\n", iio_config.ConfigIOU3[1]);
        printk(BIOS_DEBUG, "ConfigIOU4[0]: 0x%02X\n", iio_config.ConfigIOU4[0]);
        printk(BIOS_DEBUG, "ConfigIOU4[1]: 0x%02X\n", iio_config.ConfigIOU4[1]);
        printk(BIOS_DEBUG, "ConfigIOU5[0]: 0x%02X\n", iio_config.ConfigIOU5[0]);
        printk(BIOS_DEBUG, "ConfigIOU5[1]: 0x%02X\n", iio_config.ConfigIOU5[1]);
        printk(BIOS_DEBUG, "ConfigIOU6[0]: 0x%02X\n", iio_config.ConfigIOU6[0]);
        printk(BIOS_DEBUG, "ConfigIOU6[1]: 0x%02X\n", iio_config.ConfigIOU6[1]);

    }      
  
    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -3;
    }
    error = efi_fv_set_option(&smmstore, &var_guid, var_name, &iio_config, iio_config_size);
    if (error != 0)
      printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
		return 0;
}
