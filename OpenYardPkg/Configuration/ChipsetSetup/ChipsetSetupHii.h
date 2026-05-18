/** @file
  GUIDs used as HII FormSet and HII Package list GUID in Driver Sample driver.

Copyright (c) 2011 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __CHIPSET_SETUP_HII_GUID_H__
#define __CHIPSET_SETUP_HII_GUID_H__

#include <Guid/HiiPlatformSetupFormset2.h>

#define CHIPSET_SETUP_FORM_SET_GUID \
  { \
    0x2a46715f, 0x3581, 0x4a55, {0x8e, 0x73, 0x2b, 0x76, 0x9a, 0xaa, 0x30, 0xc8} \
  }

extern EFI_GUID  gEfiChipsetSetupFormSetGuid;

#endif
