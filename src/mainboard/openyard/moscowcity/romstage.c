/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <drivers/vpd/vpd.h>
#include <drivers/ocp/ewl/ocp_ewl.h>
#include <drivers/ocp/include/vpd.h>
#include <soc/romstage.h>
#include <defs_cxl.h>
#include <defs_iio.h>
#include <sprsp_ac_iio.h>
#include <OySocketIioVariable.h>
#include <OySocketProcessorCoreVariable.h>
#include <SataSetupVariable.h>

#include <drivers/efi/efivars.h>
#include <types.h>
#include <fmap.h>

#define VAR_SIZE 0x10


static void setup_gpio(void) {
#define PID_GPIOCOM0   0x6E
#define PID_GPIOCOM1   0x6D
#define PID_GPIOCOM3   0x6B
#define PID_GPIOCOM4   0x6A
#define PID_GPIOCOM5   0x69

#define PAD_OWN_GPPC_A_1 0xA4
#define PAD_CFG_DW0_GPPC_A_10 0x7A0
#define PAD_CFG_DW1_GPPC_A_10 0x7A4
#define PAD_CFG_DW2_GPPC_A_10 0x7A8
#define PAD_CFG_DW3_GPPC_A_10 0x7AC

#define PAD_OWN_GPP_H_1 0xA4
#define PAD_CFG_DW0_GPPC_H_15 0x7F0
#define PAD_CFG_DW1_GPPC_H_15 0x7F4
#define PAD_CFG_DW2_GPPC_H_15 0x7F8
#define PAD_CFG_DW3_GPPC_H_15 0x7FC
// Definition for PCR base address (defined in PchReservedResources.h)
#define PCH_PCR_BASE_ADDRESS		0xFD000000
#define PCH_PCR_MMIO_SIZE		0x01000000
#define PCH_PCR_ADDRESS(Pid, Offset)	(PCH_PCR_BASE_ADDRESS | ((unsigned char) (Pid) << 16) | (unsigned short) (Offset))

    printk(BIOS_DEBUG, "%s():%s:%d\n\r", __func__, __FILE__, __LINE__);
    printk(BIOS_DEBUG, "Set active high %s\n\r", "TP_GPPC_A_10 - TP604");
    // Set GPPC_A_10_SRCCLKREQ_N_0 - BD17 - TP_GPPC_A_10 - TP604 active (high voltage, 1v8)
    uint32_t tmp = *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_OWN_GPPC_A_1));
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_OWN_GPPC_A_1)) = tmp & (~(0xF << 8) & 0xFFFFFFFF);
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_CFG_DW0_GPPC_A_10)) = (0x1 << 9) | 0x1;
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_CFG_DW1_GPPC_A_10)) = 0x0;
    // Select voltage 1v8
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_CFG_DW2_GPPC_A_10)) = (0x1 << 8);
    // Select voltage 3v3
    //*(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_CFG_DW2_GPPC_A_10)) = (0x0 << 8);
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM0, PAD_CFG_DW3_GPPC_A_10)) = 0x0;

    printk(BIOS_DEBUG,"Set active high %s\n\r", "BIOS_READY_PFR_N_R");
    // Set GPPC_H_15_FLEX_CLK_OUT_0 - J2 - BIOS_READY_PFR_N_R - active (high voltage, 1v8)
    tmp = *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_OWN_GPP_H_1));
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_OWN_GPP_H_1)) = tmp & (~(0xF << 28) & 0xFFFFFFFF);
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_CFG_DW0_GPPC_H_15)) = (0x1 << 9) | 0x1;
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_CFG_DW1_GPPC_H_15)) = 0x0;
    // Select voltage 1v8
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_CFG_DW2_GPPC_H_15)) = (0x1 << 8);
    // Select voltage 3v3
    //*(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_CFG_DW2_GPPC_H_15)) = (0x0 << 8);
    *(uint32_t *)(PCH_PCR_ADDRESS(PID_GPIOCOM4, PAD_CFG_DW3_GPPC_H_15)) = 0x0;

}

void mainboard_ewl_check(void)
{
	if (CONFIG(OCP_EWL))
		get_ewl();
}

static void mainboard_config_iio(FSPM_UPD *mupd)
{
	/* If CONFIG(OCP_VPD) is not enabled or CXL is explicitly disabled, don't enable CXL */
	if (!CONFIG(OCP_VPD) || get_cxl_mode_from_vpd() == CXL_DISABLED) {
		printk(BIOS_DEBUG, "Don't enable CXL via VPD %s\n", CXL_MODE);
	} else {
		/* Set socket 0 IIO PCIe PE1 to CXL mode */
		/* Set socket 1 IIO PCIe PE0 to CXL mode */
		/* eg. Protocl Auto Negotiation */
		mupd->FspmConfig.IioPcieSubSystemMode1[0] = IIO_MODE_CXL;
		mupd->FspmConfig.IioPcieSubSystemMode0[1] = IIO_MODE_CXL;

		mupd->FspmConfig.DfxCxlHeaderBypass = 0;
		mupd->FspmConfig.DfxCxlSecLvl = CXL_SECURITY_FULLY_TRUSTED;

		mupd->FspmConfig.DelayAfterPCIeLinkTraining = 2000; /* ms */
	}
}


static void read_variable (void) {
    char var_name[1024] = "Timeout";
    uint32_t var_size = VAR_SIZE;
    uint8_t var_data[VAR_SIZE];
    struct region_device smmstore;

    const EFI_GUID EfiVariableGuid = { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} };

    const EFI_GUID * pguid;

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
				printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
				return;
    }
		/* Timeout */
    enum cb_err error = efi_fv_get_option(&smmstore, &EfiVariableGuid, var_name, &var_data, &var_size);
    if (error != 0) {
				printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
		  	var_data[0] = 0x05;
    		var_data[1] = 0x00;


    } else {
			printk(BIOS_ERR, "Timeout: 0x%02X, size: %d\n", *(uint16_t*)&var_data, var_size);
//	    var_data[0]++;
//	    var_data[1]++;
    }


    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
				printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
				return;
    }
		error = efi_fv_set_option(&smmstore, &EfiVariableGuid, var_name, &var_data, var_size);

		if (error != 0)
    	printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);

		/* OySocketProcessorCoreConfig */    
		const EFI_GUID gEfiOySocketProcessorCoreVarGuid = { 0x07013588, 0xC789, 0x4E12, { 0xA7, 0xC3, 0x88, 0xFA, 0xFA, 0xE7, 0x9F, 0xC7 } };
		strcpy(var_name, "OySocketProcessorCoreConfig");
    pguid = &gEfiOySocketProcessorCoreVarGuid;
    OY_SOCKET_PROCESSORCORE_CONFIGURATION soc_proc_config;
    uint32_t soc_proc_config_size = sizeof(OY_SOCKET_PROCESSORCORE_CONFIGURATION);

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
				printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
				return;
    }
    error = efi_fv_get_option(&smmstore, pguid, var_name, &soc_proc_config, &soc_proc_config_size);
    if (error != 0) {
				printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
				soc_proc_config.ProcessorHyperThreadingDisable = 1;
    } else {
        printk(BIOS_ERR, "Socket processor configuration:\n");
				printk(BIOS_ERR, "ProcessorHyperThreadingDisable: 0x%01X\n", soc_proc_config.ProcessorHyperThreadingDisable);
    }

   	if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
				printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
				return;
		}
		error = efi_fv_set_option(&smmstore, pguid, var_name, &soc_proc_config, soc_proc_config_size);
		if (error != 0)
			printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);

    /* OySocketIioeConfig */
    const EFI_GUID gEfiOySocketIioVariableGuid = { 0xdd84017e, 0x7f52, 0x48f9, { 0xb1, 0x6e, 0x50, 0xed, 0x9e, 0x0d, 0xbe, 0x72 }  };
    strcpy(var_name, "OySocketIioConfig");
    pguid = &gEfiOySocketIioVariableGuid;
    OY_SOCKET_IIO_CONFIGURATION soc_iio_config;
    uint32_t soc_iio_config_size = sizeof(OY_SOCKET_IIO_CONFIGURATION);
      
    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return;
    }
    error = efi_fv_get_option(&smmstore, pguid, var_name, &soc_iio_config, &soc_iio_config_size);
    if (error != 0) {
        printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
        soc_iio_config.ConfigIOU0[0] = 0x0;
				soc_iio_config.ConfigIOU0[1] = 0x0;
				soc_iio_config.ConfigIOU1[0] = 0x0;
				soc_iio_config.ConfigIOU1[1] = 0x0;
				soc_iio_config.ConfigIOU2[0] = 0x0;
				soc_iio_config.ConfigIOU2[1] = 0x0;
				soc_iio_config.ConfigIOU3[0] = 0x0;
				soc_iio_config.ConfigIOU3[1] = 0x0;
				soc_iio_config.ConfigIOU4[0] = 0x0;
				soc_iio_config.ConfigIOU4[1] = 0x0;
				soc_iio_config.ConfigIOU5[0] = 0x0;
				soc_iio_config.ConfigIOU5[1] = 0x0;
				soc_iio_config.ConfigIOU6[0] = 0x0;
				soc_iio_config.ConfigIOU6[1] = 0x0;
				soc_iio_config.ConfigIOU7[0] = 0x0;
				soc_iio_config.ConfigIOU7[1] = 0x0;
    } else {
        printk(BIOS_ERR, "Socket processor configuration:\n");
        printk(BIOS_ERR, "ConfigIOU0: 0x%02X\n", *(UINT16 *)&soc_iio_config.ConfigIOU0);
    }      
  
    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return;
    }
    error = efi_fv_set_option(&smmstore, pguid, var_name, &soc_iio_config, soc_iio_config_size);
    if (error != 0)
      printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
	

		/* SataSetupConfig */    
		const EFI_GUID gEfiOySataSetupVariableGuid = {0xD12BFF13, 0x6225, 0x44C4, {0xBE, 0x3F, 0x29, 0xE8, 0x32, 0xE6, 0x87, 0x41}/*0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}*/ };
		strcpy(var_name, "SataSetup");
    pguid = &gEfiOySataSetupVariableGuid;
    SATA_SETUP_CONFIGURATION sata_config;
    uint32_t sata_config_size = sizeof(SATA_SETUP_CONFIGURATION);
    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return;
    }
    error = efi_fv_get_option(&smmstore, pguid, var_name, &sata_config, &sata_config_size);
    if (error != 0) {
				printk(BIOS_ERR, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
				sata_config.Port1 = 0x1;
				sata_config.Port2 = 0x1;
    } else {
        printk(BIOS_ERR, "SATA configuration:\n");
				printk(BIOS_ERR, "Port1: 0x%02X\n", sata_config.Port1);
    }

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
		   	printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
				return;
    }
		error = efi_fv_set_option(&smmstore, pguid, var_name, &sata_config, sata_config_size);
		if (error != 0)
			printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
}

void mainboard_memory_init_params(FSPM_UPD *mupd)
{
	read_variable();
	/* Setup FSP log */
	if (CONFIG(OCP_VPD)) {
		mupd->FspmConfig.SerialIoUartDebugEnable = get_bool_from_vpd(FSP_LOG,
			FSP_LOG_DEFAULT);
		if (mupd->FspmConfig.SerialIoUartDebugEnable) {
			mupd->FspmConfig.serialDebugMsgLvl = get_int_from_vpd_range(
				FSP_MEM_LOG_LEVEL, FSP_MEM_LOG_LEVEL_DEFAULT, 0, 4);
			/* If serialDebugMsgLvl less than 1, disable FSP memory train results */
			if (mupd->FspmConfig.serialDebugMsgLvl <= 1) {
				printk(BIOS_DEBUG, "Setting serialDebugMsgLvlTrainResults to 0\n");
				mupd->FspmConfig.serialDebugMsgLvlTrainResults = 0x0;
			}
		}

		/* FSP Dfx PMIC Secure mode */
		mupd->FspmConfig.DfxPmicSecureMode = get_int_from_vpd_range(
			FSP_PMIC_SECURE_MODE, FSP_PMIC_SECURE_MODE_DEFAULT, 0, 2);
	}

	/* Set Rank Margin Tool to disable. */
	mupd->FspmConfig.EnableRMT = 0x0;
	/* Enable - Portions of memory reference code will be skipped
	 * when possible to increase boot speed on warm boots.
	 * Disable - Disables this feature.
	 * Auto - Sets it to the MRC default setting.
	 */
	mupd->FspmConfig.AttemptFastBoot = 0x1;
	mupd->FspmConfig.AttemptFastBootCold = 0x1;

	/* Set Adv MemTest Option to 0. */
	mupd->FspmConfig.AdvMemTestOptions = 0x0;
	/* Set MRC Promote Warnings to disable.
	   Determines if MRC warnings are promoted to system level. */
	mupd->FspmConfig.promoteMrcWarnings = 0x0;
	/* Set Promote Warnings to disable.
	   Determines if warnings are promoted to system level. */
	mupd->FspmConfig.promoteWarnings = 0x0;
	soc_config_iio(mupd, ac_iio_pci_port, ac_iio_bifur);
	mainboard_config_iio(mupd);
	setup_gpio();
}
