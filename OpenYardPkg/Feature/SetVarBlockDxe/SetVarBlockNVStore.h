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

#ifndef _SET_VAR_BLOCK_NVDATA_H_
#define _SET_VAR_BLOCK_NVDATA_H_

#include <Guid/HiiPlatformSetupFormset2.h>

#define SET_VAR_BLOCK_FORM_SET_GUID \
  { \
    0x2a46715f, 0x3581, 0x4a55, {0x8e, 0x73, 0x2b, 0x76, 0x9a, 0xaa, 0x30, 0xc9} \
  }

#define SET_VAR_BLOCK_VAR_GUID   { 0x363729f9, 0x35fc, 0x40a6, { 0xaf, 0xc8, 0xe8, 0xf5, 0x49, 0x11, 0xf1, 0xd8 } }
#define SET_VAR_BLOCK_VAR_NAME   L"SetVarBlockConfig"

#define MAIN_FORM_ID                        0x1000

typedef struct {
  UINT8   SetVarBlockEnable;
} SET_VAR_BLOCK_CONFIGURATION;

#endif
