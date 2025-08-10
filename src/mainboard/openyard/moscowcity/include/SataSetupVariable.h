#ifndef _SATA_SETUP_VARIABLE_H_
#define _SATA_SETUP_VARIABLE_H_

#define SATA_SETUP_VARIABLE_NAME L"SataSetup"

#define SATA_SETUP_VARIABLE_GUID \
  { \
    0xD12BFF13, 0x6225, 0x44C4, {0xBE, 0x3F, 0x29, 0xE8, 0x32, 0xE6, 0x87, 0x41 } \
  }

extern EFI_GUID  gEfiSataSetupVariableGuid;

#pragma pack(1)

typedef struct {
  // SATA
  UINT8   Port1;
  UINT8   Port2;
} SATA_SETUP_CONFIGURATION;

#pragma pack()

#endif
