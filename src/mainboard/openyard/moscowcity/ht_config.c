#include <string.h>
#include <ht_config.h>

#include <drivers/efi/efivars.h>
#include <fmap.h>

int32_t ht_setup(FSPM_UPD *mupd) {
    /* OySocketProcessorCoreConfig */
    const EFI_GUID var_guid = VAR_GUID;
    char var_name[128];
		strcpy(var_name, VAR_NAME);
    struct region_device smmstore;
		struct ht_config ht_config;
    uint32_t ht_config_size = sizeof(struct ht_config);

		if (mupd == NULL)
				return -1;

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -2;
    }

		enum cb_err error = efi_fv_get_option(&smmstore, &var_guid, var_name, &ht_config, &ht_config_size);
    if (error != 0) {
        printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
        ht_config.hyperthreadingdisable = 1;
    } else {
        printk(BIOS_ERR, "Socket processor configuration:\n");
        printk(BIOS_ERR, "hyperthreadingdisable: 0x%01X\n", ht_config.hyperthreadingdisable);
    }
		
		mupd->FspmConfig.ProcessorHyperThreadingDisable = ht_config.hyperthreadingdisable;
    
		if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -3;
    }
    error = efi_fv_set_option(&smmstore, &var_guid, var_name, &ht_config, ht_config_size);
    if (error != 0)
        printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);

    return 0;
}
