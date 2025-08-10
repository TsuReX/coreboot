/** @file
  Data format for Universal Data Structure

  @copyright
  Copyright 1999 - 2025 OpenYard Corporation. <BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _OY_SOCKET_IIO_VARIABLE_H_
#define _OY_SOCKET_IIO_VARIABLE_H_


//extern EFI_GUID gEfiOySocketIioVariableGuid;// = { 0xdd84017e, 0x7f52, 0x48f9, { 0xb1, 0x6e, 0x50, 0xed, 0x9e, 0x0d, 0xbe, 0x72 } };
#define SOCKET_IIO_CONFIGURATION_NAME L"OySocketIioConfig"

#pragma pack(1)

typedef struct {

    // General PCIE Configuration
    UINT8   ConfigIOU0[2];
    UINT8   ConfigIOU1[2];
    UINT8   ConfigIOU2[2];
    UINT8   ConfigIOU3[2];
    UINT8   ConfigIOU4[2];
    UINT8   ConfigIOU5[2];
    UINT8   ConfigIOU6[2];
    UINT8   ConfigIOU7[2];
    
} OY_SOCKET_IIO_CONFIGURATION;
#pragma pack()

#endif // _OY_SOCKET_IIO_VARIABLE_H_

