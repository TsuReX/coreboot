/** @file
  Header file for NV data structure definition.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _VARSTORE_INFO_HII_H_
#define _VARSTORE_INFO_HII_H_

#include <Guid/HiiPlatformSetupFormset.h>

#define VARSTORE_INFO_FORM_SET_GUID \
  { \
    0x2a46715f, 0x3581, 0x4a55, {0x8e, 0x73, 0x2b, 0x76, 0x9a, 0xaa, 0x30, 0xc7} \
  }


#define MAIN_FORM_ID                        0x1000

#define KEY_UPDATE                          0x100


typedef struct {
  UINT64 MaximumVariableStorageSize;
  UINT64 RemainingVariableStorageSize;
  UINT64 MaximumVariableSize;
} VARSTORE_INFO;

#endif
