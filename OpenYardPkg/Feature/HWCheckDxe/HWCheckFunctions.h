/** @file
  TODO: Brief Description of UEFI Driver HWCheckManagement

  TODO: Detailed Description of UEFI Driver HWCheckManagement

  TODO: Copyright for UEFI Driver HWCheckManagement

  TODO: License for UEFI Driver HWCheckManagement

**/

#ifndef __EFI_HW_CHECK_MANAGEMENT_H__
#define __EFI_HW_CHECK_MANAGEMENT_H__

#define HWDATA_VAR_GUID   { 0x363729f9, 0x35fc, 0x40a6, { 0xaf, 0xc8, 0xe8, 0xf5, 0x49, 0x11, 0xf1, 0xd7 } }
#define HWDATA_VAR_NAME   L"HWCheckData"

#define ATA_SERIAL_NUMBER_LENGTH  20


typedef struct _IDENTIFY_DATA {

  UINT16  General_Config_0;
  UINT16  Reserved_1;
  UINT16  Special_Config_2;
  UINT16  Reserved_3;
  UINT16  Reserved_4;
  UINT16  Reserved_5;
  UINT16  Reserved_6;
  UINT16  Reserved_7;
  UINT16  Reserved_8;
  UINT16  Reserved_9;
  UINT8   Serial_Number_10[20];
  UINT16  Reserved_20;
  UINT16  Reserved_21;
  UINT16  Reserved_22;
  UINT8   Firmware_Revision_23[8];
  UINT8   Model_Number_27[40];
  UINT16  Maximum_Sector_Multiple_Command_47;
  UINT16  Trusted_Computing_Support;
  UINT16  Capabilities_49;
  UINT16  Capabilities_50;
  UINT16  PIO_Mode_51;
  UINT16  Reserved_52;
  UINT16  Valid_Bits_53;
  UINT16  Reserved_54_58[5];
  UINT16  Valid_Bits_59;
  UINT32  Addressable_Sector_60;
  UINT16  SingleWord_DMA_62;
  UINT16  MultiWord_DMA_63;
  UINT16  PIO_Mode_64;
  UINT16  Min_Multiword_DMA_timing_65;
  UINT16  Manuf_Multiword_DMA_timing_66;
  UINT16  Min_PIO_Mode_timing_67;
  UINT16  Min_PIO_Mode_timing_68;
  UINT16  Reserved_69_74[6];
  UINT16  Queue_Depth_75;
  UINT16  Reserved_76_79[4];
  UINT16  Major_Revision_80;
  UINT16  Minor_Revision_81;
  UINT16  Command_Set_Supported_82;
  UINT16  Command_Set_Supported_83;
  UINT16  Command_Set_Supported_84;
  UINT16  Command_Set_Enabled_85;
  UINT16  Command_Set_Enabled_86;
  UINT16  Command_Set_Enabled_87;
  UINT16  UDMA_Mode_88;
  UINT16  Time_security_Earse_89;
  UINT16  Time_Esecurity_Earse_90;
  UINT16  Current_Power_Level_91;
  UINT16  Master_Password_Rev_92;
  UINT16  Hard_Reset_Value_93;
  UINT16  Acoustic_Level_94;
  UINT16  Reserved_95_99[5];
  UINT64  LBA_48;
  UINT16  Reserved_104_126[23];
  UINT16  Status_Notification_127;
  UINT16  Security_Status_128;
  UINT16  Reserved_129_159[31];
  UINT16  CFA_Power_Mode_160;
  UINT16  Reserved_161_175[15];
  UINT16  Media_Serial_Number_176_205[30];
  UINT16  Reserved_206_254[49];
  UINT16  Checksum_255;
} IDENTIFY_DATA;


typedef struct {

  UINT16      Ms;                             ///< Bits 15:0      Metadata Size
  UINT8       Lbads;                          ///< Bits 23:16     LBA Data Size
  UINT8       Rp : 2;                         ///< Bits 25:24     Relative Performance
  UINT8       Reserved1 : 6;                   ///< Bits 31:26     Reserved

} NVME_LBA_FORMAT_DATA;


typedef struct {

  UINT64                  Nsize;              ///< Offset 7:0      Namespace Size
  UINT64                  Ncap;               ///< Offset 15:8     Namespace Capacity
  UINT64                  Nuse;               ///< Offset 23:16    Namespace Utilization
  UINT8                   Nsfeat;             ///< Offset 24       Namespace Features
  UINT8                   Nlbaf;              ///< Offset 25       Number of LBA Formats
  UINT8                   Flbas;              ///< Offset 26       Formatted LBA Size
  UINT8                   Mc;                 ///< Offset 27       Metadata Capabilities
  UINT8                   Dpc;                ///< Offset 28       End-to-end Data Protection Capabilities
  UINT8                   Dps;                ///< Offset 29       End-to-end Data Protection Type Settings
  UINT8                   Nmic;               ///< Offset 30       Namespace Multi-path I/O and Namespace Sharing Capabilities
  UINT8                   Rescap;             ///< Offset 31       Reservation Capabilities
  UINT8                   Fpi;                ///< Offset 32       Format Progress Indicator
  UINT8                   Reserved1;          ///< Offset 33       Reserved
  UINT16                  Nawun;              ///< Offset 35:34    Namespace Atomic Write Unit Normal
  UINT16                  Nawupf;             ///< Offset 37:36    Namespace Atomic Write Unit Power Fail
  UINT16                  Nacwu;              ///< Offset 39:38    Namespace Atomic Compare & Write Unit
  UINT16                  Nabsn;              ///< Offset 41:40    Namespace Atomic Boundary Size Normal
  UINT16                  Nabo;               ///< Offset 43:42    Namespace Atomic Boundary Offset
  UINT16                  Nabspf;             ///< Offset 45:44    Namespace Atomic Boundary Size Power Fail
  UINT8                   Reserved2[2];       ///< Offset 46:47    Reserved
  UINT8                   Nvmcap[16];         ///< Offset 63:48    NVM Capacity
  UINT8                   Reserved3[40];      ///< Offset 103:64   Reserved
  UINT8                   Nguid[16];          ///< Offset 119:104  Namespace Globally Unique Identifier
  UINT64                  Eui64;              ///< Offset 127:120  IEEE Extended Unique Identifier
  NVME_LBA_FORMAT_DATA    Lbaf[16];           ///< Offset 191:128  LBA Format 
  UINT8                   Reserved4[192];     ///< Offset 383:192  Reserved
  UINT8                   Vs[3712];           ///< Offset 4095:384 Vendor Specific   
} NVME_IDENTIFY_NAMESPACE_DATA;

typedef struct {
  UINT8 PortNumber;
  UINT8 IdentifyData[20];
} HWCHECK_DISK_INFO;

typedef struct {
  UINT16  PciNumber;
  UINT8   BusNumber;
  UINT8   DeviceNumber;
  UINT8   FunctionNumber;
  UINT16  DeviceId;
  UINT16  VendorId;
} HWCHECK_PCI_INFO;


typedef struct {
  UINT32 CpuInfoCrc32;
  UINT32 DimmInfoCrc32;
  UINT32 DisksInfoCrc32;
  UINT32 PciInfoCrc32;
} HWCHECK_INFO;

typedef struct {
  CHAR8 SocketDesignation[SMBIOS_STRING_MAX_LENGTH];
  PROCESSOR_ID_DATA ProcessorId;
} HWCHECK_CPU_INFO;

typedef struct {
  //CHAR8 DeviceLocator[SMBIOS_STRING_MAX_LENGTH];
  CHAR8 Manufacturer[SMBIOS_STRING_MAX_LENGTH];
  CHAR8 SerialNumber[SMBIOS_STRING_MAX_LENGTH];
  //UINT16 Size;
} HWCHECK_DIMM_INFO;

EFI_STATUS
EFIAPI
HWCheckGetHardwareInfo(
  IN OUT HWCHECK_INFO* HardwareInfo
);

EFI_STATUS
EFIAPI
HWCheckSaveHardwareInfo();

#endif
