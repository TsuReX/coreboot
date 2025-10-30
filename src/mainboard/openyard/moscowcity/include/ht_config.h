#ifndef __HT_CONIG__
#define __HT_CONIG__

#include <stdint.h>
#include <FspmUpd.h>

struct ht_config {
		int8_t ProcessorHyperThreadingDisable;
};

#define HT_VAR_GUID { 0x07013588, 0xC789, 0x4E12, { 0xA7, 0xC3, 0x88, 0xFA, 0xFA, 0xE7, 0x9F, 0xC7 } }
#define HT_VAR_NAME "OySocketProcessorCoreConfig"

int32_t ht_setup(FSPM_UPD *mupd);

#endif // __HT_CONFIG__
