#ifndef __SATA_CONFIG__
#define __SATA_CONFIG__

#include <stdint.h>
#include <FspmUpd.h>

struct sata_config {
		uint8_t Port1;
		uint8_t Port2;
};

#define SATA_VAR_GUID {0xD12BFF13, 0x6225, 0x44C4, {0xBE, 0x3F, 0x29, 0xE8, 0x32, 0xE6, 0x87, 0x41} }
#define SATA_VAR_NAME "SataSetup"

int32_t sata_setup(FSPM_UPD *mupd);

#endif // __SATA_CONFIG__
