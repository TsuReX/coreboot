#ifndef __OYSETUP_CONFIG__
#define __OYSETUP_CONFIG__

#include <stdint.h>
#include <FspmUpd.h>

#define OYSETUP_VAR_GUID { 0xD13BFF13, 0x6225, 0x44C4, {0xBE, 0x3F, 0x29, 0xE8, 0x32, 0xE6, 0x87, 0x4A } }
#define OYSETUP_VAR_NAME "OYSetup"

#define SETUP_MAX_USB2_PORTS                16
#define SETUP_MAX_USB3_PORTS                10

#define PCH_MAX_SATA_CONTROLLERS 3
#define PCH_MAX_SATA_PORTS  8
#undef MAX_SOCKET
#define MAX_SOCKET 2

#define IIO_BIFURCATE_x4x4x4x4          0x0
#define IIO_BIFURCATE_x4x4xxx8          0x1
#define IIO_BIFURCATE_xxx8x4x4          0x2
#define IIO_BIFURCATE_xxx8xxx8          0x3
#define IIO_BIFURCATE_xxxxxx16          0x4
#define IIO_BIFURCATE_AUTO              0xFF

struct __attribute__((packed)) oysetup_config {
  // Chipset - USB
  uint8_t   PchUsbFilter;
  uint8_t   PchUsbReadOnly;

  uint8_t   PchUsbHsPort[SETUP_MAX_USB2_PORTS];
  uint8_t   PchUsbSsPort[SETUP_MAX_USB3_PORTS];

  uint8_t   SataInterfaceMode[PCH_MAX_SATA_CONTROLLERS];

  uint8_t   SataPortController0[PCH_MAX_SATA_PORTS];
  uint8_t   SataPortController1[PCH_MAX_SATA_PORTS];
  uint8_t   SataPortController2[PCH_MAX_SATA_PORTS];

  uint8_t   StateAfterG3;

  // Socket
  uint8_t   ProcessorHyperThreadingDisable;

  // General PCIE Configuration
  uint8_t   ConfigIOU0[MAX_SOCKET];
  uint8_t   ConfigIOU1[MAX_SOCKET];
  uint8_t   ConfigIOU2[MAX_SOCKET];
  uint8_t   ConfigIOU3[MAX_SOCKET];
  uint8_t   ConfigIOU4[MAX_SOCKET];
  uint8_t   ConfigIOU5[MAX_SOCKET];
  uint8_t   ConfigIOU6[MAX_SOCKET];
  uint8_t   ConfigIOU7[MAX_SOCKET];
  
  // Server Management
  uint8_t         RefInterval;
  // IPv4
  uint8_t         BmcLan1;
  uint16_t        StationIp1[15];
  uint16_t        Subnet1[15];
  uint16_t        RouterIp1[15];
  uint16_t        RouterMac1[17];
  // IPv6
  uint8_t        Ip6Support1;
  uint8_t        PrefixLength1;
  uint8_t        Ip6BmcLan1;
  uint8_t        Ip6RouterLan1;
  uint8_t        RouterPrefixLength1;
  uint16_t       StaticIp6Lan1Router1Ip[40];
  uint16_t       RouterPrefixValue1[40];
  uint16_t       StaticIp6Lan1[40];
};

int32_t oysetup_setup(FSPM_UPD *mupd);

#endif // __OYSETUP_CONFIG__

