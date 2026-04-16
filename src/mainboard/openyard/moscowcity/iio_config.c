#include <string.h>
#include <iio_config.h>
#include <IioPcieConfigUpd.h>
#include <drivers/efi/efivars.h>
#include <fmap.h>
#include <sprsp_ac_iio.h>

#define _PEXPUNHIDE 0
#define _PEXPHIDE 1

#define _SLOTNOIMP 0
#define _SLOTIMP 1

#define SOCKET0 0
#define SOCKET1 1

#define IOU0 0
#define IOU1 1
#define IOU2 2
#define IOU3 3
#define IOU4 4
#define IOU5 5
#define IOU6 6

static void pe_controller_setup(FSPM_UPD *mupd, uint32_t socket, uint32_t pe_controller_number, UINT8 iio_bifur) {
  printk(BIOS_DEBUG, "pe_controller_setup( socket: %d, pe_controller_number: %d, iio_bifur %d) \n", socket, pe_controller_number, iio_bifur);
  static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  *pe_controller_config;
  switch (iio_bifur) {
    case IIO_BIFURCATE_x4x4xxx8:
      static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  pe_controller_config_4_4_8[8] = {
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 1),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 2),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 3),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 4),
        // x8
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 5),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 6),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 7),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 8)
      };
      printk(BIOS_DEBUG, "Bifurcation: x4x4xxx8\n");
      pe_controller_config = pe_controller_config_4_4_8;
      break;

    case IIO_BIFURCATE_xxx8x4x4:
      static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  pe_controller_config_8_4_4[8] = {
      // x8
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 1),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 2),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 3),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 4),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 5),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 6),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 7),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 8)
      };
      printk(BIOS_DEBUG, "Bifurcation: xxx8x4x4\n");
      pe_controller_config = pe_controller_config_8_4_4;
      break;

    case IIO_BIFURCATE_xxx8xxx8:
      static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  pe_controller_config_8_8[8] = {
        // x8
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 1),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 2),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 3),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 4),
        // x8
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 5),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 6),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 7),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 8)
      };
      printk(BIOS_DEBUG, "Bifurcation: xxx8xxx8\n");
      pe_controller_config = pe_controller_config_8_8;
      break;

    case IIO_BIFURCATE_xxxxxx16:
      static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  pe_controller_config_16[8] = {
        // x16
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 1),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 2),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 3),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 4),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 5),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 6),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 7),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 8)
      };
      printk(BIOS_DEBUG, "Bifurcation: xxxxxx16\n");
      pe_controller_config = pe_controller_config_16;
      break;

    case IIO_BIFURCATE_x4x4x4x4:
    default:
      iio_bifur = IIO_BIFURCATE_x4x4x4x4;
      static UPD_IIO_PCIE_PORT_CONFIG_ENTRY  pe_controller_config_4_4_4_4[8] = {
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 1),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 2),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 3),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 4),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 5),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 6),
        // x4
        CFG_UPD_PCIE_PORT(_PEXPUNHIDE, _SLOTIMP, 7),
        CFG_UPD_PCIE_PORT(_PEXPHIDE, _SLOTNOIMP, 8)
    };
    printk(BIOS_DEBUG, "Bifurcation: x4x4x4x4\n");
    pe_controller_config = pe_controller_config_4_4_4_4;

  };

  UPD_IIO_PCIE_PORT_CONFIG *PciePortConfig = (UPD_IIO_PCIE_PORT_CONFIG *)(UINTN)mupd->FspmConfig.IioPcieConfigTablePtr;

  // 0 -> DMI; 1-8 -> PE0; 9-16 -> PE1
  for (uint32_t pe_controller_port = 0; pe_controller_port < 8; pe_controller_port++) {
    const UPD_IIO_PCIE_PORT_CONFIG_ENTRY *port_cfg = &pe_controller_config[pe_controller_port];
    printk(BIOS_DEBUG, "PE port number: %d, ", pe_controller_number * 8 + 1 + pe_controller_port);
    if (port_cfg->SLOTIMP == 1)
      printk(BIOS_DEBUG, "implemented, ");
    else
      printk(BIOS_DEBUG, "NOT implemented, ");

    if (port_cfg->PEXPHIDE == 1)
      printk(BIOS_DEBUG, "controller hidden\n");
    else
      printk(BIOS_DEBUG, "controller implemented\n");

    PciePortConfig[socket].SLOTIMP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTIMP;
    PciePortConfig[socket].SLOTPSP[pe_controller_number * 8 + 1 + pe_controller_port] = pe_controller_number * 8 + 1 + pe_controller_port;
    PciePortConfig[socket].SLOTHPCAP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTHPCAP;
    PciePortConfig[socket].SLOTHPSUP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTHPSUP;
    PciePortConfig[socket].SLOTSPLS[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTSPLS;
    PciePortConfig[socket].SLOTSPLV[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTSPLV;
    PciePortConfig[socket].VppAddress[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->VppAddress;
    PciePortConfig[socket].SLOTPIP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTPIP;
    PciePortConfig[socket].SLOTAIP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTAIP;
    PciePortConfig[socket].SLOTMRLSP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTMRLSP;
    PciePortConfig[socket].SLOTPCP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTPCP;
    PciePortConfig[socket].SLOTABP[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->SLOTABP;
    PciePortConfig[socket].VppEnabled[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->VppEnabled;
    PciePortConfig[socket].VppPort[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->VppPort;
    PciePortConfig[socket].MuxAddress[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->MuxAddress;
    PciePortConfig[socket].PciePortEnable[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->PciePortEnable;
    PciePortConfig[socket].PEXPHIDE[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->PEXPHIDE;
    PciePortConfig[socket].PcieHotPlugOnPort[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->PcieHotPlugOnPort;
    PciePortConfig[socket].PcieMaxPayload[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->PcieMaxPayload;
    PciePortConfig[socket].PciePortLinkSpeed[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->PciePortLinkSpeed;
    PciePortConfig[socket].DfxDnTxPresetGen3[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->DfxDnTxPresetGen3;
    PciePortConfig[socket].HidePEXPMenu[pe_controller_number * 8 + 1 + pe_controller_port] = port_cfg->HidePEXPMenu;
  }
  PciePortConfig[socket].ConfigIOU[pe_controller_number] = iio_bifur;
}

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

        iio_config.ConfigIOU0[0] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU0[1] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU1[0] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU1[1] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU2[0] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU2[1] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU3[0] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU3[1] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU4[0] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU4[1] = IIO_BIFURCATE_x4x4x4x4;
        iio_config.ConfigIOU5[0] = IIO_BIFURCATE_xxxxxxxx;
        iio_config.ConfigIOU5[1] = IIO_BIFURCATE_xxxxxxxx;
        iio_config.ConfigIOU6[0] = IIO_BIFURCATE_xxxxxxxx;
        iio_config.ConfigIOU6[1] = IIO_BIFURCATE_xxxxxxxx;
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
    // FSPM_UPD and FSPM_CONFIG are define in 3rdparty/fsp/EagleStreamFspBinPkg/Include/FspmUpd.h
    mupd->FspmConfig.IioConfigIOU0[SOCKET0] = iio_config.ConfigIOU0[SOCKET0];
    pe_controller_setup(mupd, SOCKET0, IOU0, iio_config.ConfigIOU0[SOCKET0]);
    mupd->FspmConfig.IioConfigIOU0[SOCKET1] = iio_config.ConfigIOU0[SOCKET1];
    pe_controller_setup(mupd, SOCKET1, IOU0, iio_config.ConfigIOU0[SOCKET1]);

    mupd->FspmConfig.IioConfigIOU1[SOCKET0] = iio_config.ConfigIOU1[SOCKET0];
    pe_controller_setup(mupd, SOCKET0, IOU1, iio_config.ConfigIOU1[SOCKET0]);
    mupd->FspmConfig.IioConfigIOU1[SOCKET1] = iio_config.ConfigIOU1[SOCKET1];
    pe_controller_setup(mupd, SOCKET1, IOU1, iio_config.ConfigIOU1[SOCKET1]);

    mupd->FspmConfig.IioConfigIOU2[SOCKET0] = iio_config.ConfigIOU2[SOCKET0];
    pe_controller_setup(mupd, SOCKET0, IOU2, iio_config.ConfigIOU2[SOCKET0]);
    mupd->FspmConfig.IioConfigIOU2[SOCKET1] = iio_config.ConfigIOU2[SOCKET1];
    pe_controller_setup(mupd, SOCKET1, IOU2, iio_config.ConfigIOU2[SOCKET1]);

    mupd->FspmConfig.IioConfigIOU3[SOCKET0] = iio_config.ConfigIOU3[SOCKET0];
    pe_controller_setup(mupd, SOCKET0, IOU3, iio_config.ConfigIOU3[SOCKET0]);
    mupd->FspmConfig.IioConfigIOU3[SOCKET1] = iio_config.ConfigIOU3[SOCKET1];
    pe_controller_setup(mupd, SOCKET1, IOU3, iio_config.ConfigIOU3[SOCKET1]);

    mupd->FspmConfig.IioConfigIOU4[SOCKET0] = iio_config.ConfigIOU4[SOCKET0];
    pe_controller_setup(mupd, SOCKET0, IOU4, iio_config.ConfigIOU4[SOCKET0]);
    mupd->FspmConfig.IioConfigIOU4[SOCKET1] = iio_config.ConfigIOU4[SOCKET1];
    pe_controller_setup(mupd, SOCKET1, IOU4, iio_config.ConfigIOU4[SOCKET1]);

    if (fmap_locate_area_as_rdev_rw("SMMSTORE", &smmstore) < 0) { // src/lib/fmap.c
        printk(BIOS_ERR, "SMMSTORE region wasn't found \n");
        return -3;
    }
    error = efi_fv_set_option(&smmstore, &var_guid, var_name, &iio_config, iio_config_size);
    if (error != 0)
      printk(BIOS_ERR, "EFI variable %s can't be set up, error: %d\n", var_name, error);
		return 0;
}
