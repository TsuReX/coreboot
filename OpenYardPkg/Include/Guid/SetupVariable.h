#ifndef _OY_SETUP_VARIABLE_H_
#define _OY_SETUP_VARIABLE_H_

#define OY_SETUP_VARIABLE_NAME L"OYSetup"

#define OY_SETUP_VARIABLE_GUID \
  { \
    0xD13BFF13, 0x6225, 0x44C4, {0xBE, 0x3F, 0x29, 0xE8, 0x32, 0xE6, 0x87, 0x4A } \
  }

extern EFI_GUID  gEfiOYSetupVariableGuid;

#pragma pack(1)

#define SETUP_MAX_USB2_PORTS                16
#define SETUP_MAX_USB3_PORTS                10

#define PCH_MAX_SATA_CONTROLLERS 3
#define PCH_MAX_SATA_PORTS  8

#define MAX_SOCKET 2

#define IIO_BIFURCATE_x4x4x4x4          0x0
#define IIO_BIFURCATE_x4x4xxx8          0x1
#define IIO_BIFURCATE_xxx8x4x4          0x2
#define IIO_BIFURCATE_xxx8xxx8          0x3
#define IIO_BIFURCATE_xxxxxx16          0x4
#define IIO_BIFURCATE_AUTO              0xFF


typedef struct {
  // Chipset - USB
  UINT8   PchUsbFilter;
  UINT8   PchUsbReadOnly;

  UINT8   PchUsbHsPort[SETUP_MAX_USB2_PORTS];
  UINT8   PchUsbSsPort[SETUP_MAX_USB3_PORTS];

  UINT8   SataInterfaceMode[PCH_MAX_SATA_CONTROLLERS];

  UINT8   SataPortController0[PCH_MAX_SATA_PORTS];
  UINT8   SataPortController1[PCH_MAX_SATA_PORTS];
  UINT8   SataPortController2[PCH_MAX_SATA_PORTS];

  UINT8   StateAfterG3;

  // Socket
  UINT8   ProcessorHyperThreadingDisable;

  // General PCIE Configuration
  UINT8   ConfigIOU0[MAX_SOCKET];
  UINT8   ConfigIOU1[MAX_SOCKET];
  UINT8   ConfigIOU2[MAX_SOCKET];
  UINT8   ConfigIOU3[MAX_SOCKET];
  UINT8   ConfigIOU4[MAX_SOCKET];
  UINT8   ConfigIOU5[MAX_SOCKET];
  UINT8   ConfigIOU6[MAX_SOCKET];
  UINT8   ConfigIOU7[MAX_SOCKET];
  
  // Server Management
  UINT8         RefInterval;
  // IPv4
  UINT8         BmcLan1;
  CHAR16        StationIp1[15];
  CHAR16        Subnet1[15];
  CHAR16        RouterIp1[15];
  CHAR16        RouterMac1[17];
  // IPv6
  UINT8        Ip6Support1;
  UINT8        PrefixLength1;
  UINT8        Ip6BmcLan1;
  UINT8        Ip6RouterLan1;
  UINT8        RouterPrefixLength1;
  CHAR16       StaticIp6Lan1Router1Ip[40];
  CHAR16       RouterPrefixValue1[40];
  CHAR16       StaticIp6Lan1[40];
} OY_SETUP_CONFIGURATION;

#pragma pack()

#endif
