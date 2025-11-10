#ifndef __IIO_CONFIG__
#define __IIO_CONFIG__

#include <stdint.h>
#include <FspmUpd.h>

struct iio_config {
/*
	IIO ConfigIOUx
  ConfigIOU[MAX_SOCKET][x]: MAX_SOCKET=8, 0x00:x4x4x4x4, 0x01:x4x4xxx8, 0x02:xxx8x4x4,
  0x03:xxx8xxx8, 0x04:xxxxxx16, 0xFF:AUTO(Default)
*/
  int8_t ConfigIOU0[8];
	int8_t ConfigIOU1[8];
	int8_t ConfigIOU2[8];
	int8_t ConfigIOU3[8];
	int8_t ConfigIOU4[8];
	int8_t ConfigIOU5[8];
	int8_t ConfigIOU6[8];
};

#define IIO_VAR_GUID { 0xdd84017e, 0x7f52, 0x48f9, { 0xb1, 0x6e, 0x50, 0xed, 0x9e, 0x0d, 0xbe, 0x72 } }
#define IIO_VAR_NAME "IioConfig"

int32_t iio_setup(FSPM_UPD *mupd);

#endif // __IIO_CONFIG__                     
