/** @file
  The driver entry point for VarstoreInfoDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "VarstoreInfoImpl.h"


STATIC VARSTORE_INFO_PRIVATE_DATA* mConfigPrivate = NULL;

/**
  The entry point for VarstoreInfoDxe driver.

  @param[in] ImageHandle     The image handle of the driver.
  @param[in] SystemTable     The system table.

  @retval EFI_ALREADY_STARTED     The driver already exists in system.
  @retval EFI_OUT_OF_RESOURCES    Fail to execute entry point due to lack of
                                  resources.
  @retval EFI_SUCCES              All the related protocols are installed on
                                  the driver.

**/
EFI_STATUS
EFIAPI
VarstoreInfoDxeEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;

  mConfigPrivate = (VARSTORE_INFO_PRIVATE_DATA*) AllocateCopyPool (sizeof (VARSTORE_INFO_PRIVATE_DATA), &mVarstoreInfoPrivateDataTemplate);
  if (mConfigPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InstallVarstoreInfoConfigForm (mConfigPrivate);

  return Status;
}
