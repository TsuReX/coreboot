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

#include <ht_config.h>
#include <iio_config.h>
#include <sata_config.h>
#include <oysetup_config.h>

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


static int32_t timeout_setup (void) {
	char var_name[1024] = "Timeout";
	const EFI_GUID var_guid = { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} };
	uint16_t timeout;
	uint32_t timeout_size = sizeof(timeout);
	struct region_device smmstore;

	if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
		printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
		return -1;
	}

	enum cb_err error = efi_fv_get_option(&smmstore, &var_guid, var_name, &timeout, &timeout_size);
	if (error != 0) {
		printk(BIOS_WARNING, "EFI variable %s can't be gotten, error: %d\n", var_name, error);
		*(uint16_t*)&timeout = 0x0A;
	} else {
		printk(BIOS_DEBUG, "Timeout: 0x%04X, size: %d\n", *(uint16_t*)&timeout, timeout_size);
	}

	if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
		printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
		return -2;
	}

	error = efi_fv_set_option(&smmstore, &var_guid, var_name, &timeout, timeout_size);

	if (error != 0)
		printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
	return 0;
}

void mainboard_memory_init_params(FSPM_UPD *mupd)
{
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
	ht_setup(mupd);
	iio_setup(mupd);
	timeout_setup();
	sata_setup(mupd);
  oysetup_setup(mupd);
// Setup MMCFG Base Address - 0: 1G, 1: 1.5G, 2: 1.75G, 3: 2G, 4: 2.25G, 5: 3G, 6: Auto(Default)
	printk(BIOS_DEBUG, "mupd->FspmConfig.mmCfgBase: 0x%02X\n", mupd->FspmConfig.mmCfgBase);
// Select MMCFG Size - 0: 64M, 1: 128M, 2: 256M, 3: 512M, 4: 1G, 5: 2G, 6: Auto(Default).
	printk(BIOS_DEBUG, "mupd->FspmConfig.mmCfgSize: 0x%02X\n", mupd->FspmConfig.mmCfgSize);

}
