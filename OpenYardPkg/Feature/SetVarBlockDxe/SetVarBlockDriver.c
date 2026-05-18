/** @file
  The driver entry point for SetVarBlockDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "SetVarBlockImpl.h"


STATIC SET_VAR_BLOCK_CONFIG_PRIVATE_DATA* mConfigPrivate = NULL;
STATIC EFI_SET_VARIABLE mSetVariableOriginal;

EFI_EVENT mExitBootServicesEvent = NULL;


EFI_STATUS
SetVariableNop(
  IN  CHAR16* VariableName,
  IN  EFI_GUID* VendorGuid,
  IN  UINT32                       Attributes,
  IN  UINTN                        DataSize,
  IN  VOID* Data
  )
{
  return EFI_SUCCESS;
}


/**
VOID
EFIAPI
ReadyToBootNotify(
  EFI_EVENT Event,
  VOID* Context)
{
  mSetVariableOriginal = gRT->SetVariable;

  gRT->SetVariable = &SetVariableNop;
}
**/


VOID
EFIAPI
ExitBootServicesNotify(
  IN EFI_EVENT  Event,
  IN VOID* Context
)
{
  mSetVariableOriginal = gRT->SetVariable;

  gRT->SetVariable = &SetVariableNop;
}


/**
  The entry point for SetVarBlockDxe driver.

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
SetVarBlockDxeEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  SET_VAR_BLOCK_CONFIGURATION Configuration;
  //EFI_EVENT   Event;

  mConfigPrivate = (SET_VAR_BLOCK_CONFIG_PRIVATE_DATA*) AllocateCopyPool (sizeof (SET_VAR_BLOCK_CONFIG_PRIVATE_DATA), &mSetVarBlockConfigPrivateDataTemplate);
  if (mConfigPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InstallSetVarBlockConfigForm (mConfigPrivate);

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Configuration = mConfigPrivate->ConfigStore;

  if (0x00 == Configuration.SetVarBlockEnable)
  {
    return Status;
  }

  //Status = EfiCreateEventReadyToBootEx(TPL_CALLBACK, ReadyToBootNotify, NULL, &Event);

  Status = gBS->CreateEvent(
    EVT_SIGNAL_EXIT_BOOT_SERVICES,    // Type
    TPL_NOTIFY,                       // NotifyTpl
    ExitBootServicesNotify,        // NotifyFunction
    NULL,                             // NotifyContext
    &mExitBootServicesEvent           // Event
  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
