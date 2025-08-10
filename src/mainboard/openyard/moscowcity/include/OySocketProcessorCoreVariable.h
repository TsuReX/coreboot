/** @file
  Data format for Universal Data Structure

  @copyright
  Copyright 1999 - 2025 OpenYard Corporation. <BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef   __OY_SOCKET_PROCESSORCORE_CONFIGURATION_DATA_H__
#define   __OY_SOCKET_PROCESSORCORE_CONFIGURATION_DATA_H__
//#include <Uefi/UefiBaseType.h>
//extern EFI_GUID gEfiOySocketProcessorCoreVarGuid;// = { 0x07013588, 0xC789, 0x4E12, { 0xA7, 0xC3, 0x88, 0xFA, 0xFA, 0xE7, 0x9F, 0xC7 } };
#define OY_SOCKET_PROCESSORCORE_CONFIGURATION_NAME L"OySocketProcessorCoreConfig"

#pragma pack(1)

typedef struct {
	UINT8   ProcessorHyperThreadingDisable;   // Hyper Threading [ALL]
} OY_SOCKET_PROCESSORCORE_CONFIGURATION;
#pragma pack()

#endif
