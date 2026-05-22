/** @file
  The driver entry point for HWCheckDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "HWCheckImpl.h"

#define IPMI_COMMAND_HW_CHECK_RESULT                      0x30

extern EFI_GUID   mHWInfoVarGuid;// = HWDATA_VAR_GUID;
extern CHAR16     mHWInfoVarName[];// = HWDATA_VAR_NAME;


STATIC HW_CHECK_CONFIG_PRIVATE_DATA* mConfigPrivate = NULL;


VOID
EFIAPI
ReadyToBootNotify(
  EFI_EVENT Event,
  VOID* Context)
{
  CHAR16* Message;

  Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_SYSTEM_HALT), NULL);
  Print((CHAR16*)L"%s", Message);
  CpuDeadLoop();
}

VOID
EFIAPI
HWCheckPrintError(CHAR16* Message)
{
    STATIC BOOLEAN ErrorOccured = FALSE;
    
    if (!ErrorOccured)
    {
        CHAR16* ErrorMessage;
        ErrorMessage = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR), NULL);
        Print((CHAR16*)ErrorMessage);
        ErrorOccured = TRUE;
    }
    Print((CHAR16*)Message);
}
        

EFI_STATUS
EFIAPI
HWCheckCompare(
  IN HWCHECK_INFO* HWInfo,
  OUT HWCHECK_RESULT* Result
)
{
  EFI_STATUS Status = EFI_SUCCESS;
  HWCHECK_INFO* HWInfoCurrent = NULL;
  UINTN HWInfoSize = 0x00;
  CHAR16* Message;

  Result->Byte = 0x00;
  
  Print((CHAR16*)L"\n");
  Print((CHAR16*)L"\n");

  Status = GetVariable2(mHWInfoVarName, &mHWInfoVarGuid, (VOID**)&HWInfoCurrent, &HWInfoSize);
  if ((EFI_ERROR(Status)) || (sizeof(HWCHECK_INFO) != HWInfoSize))
  {
    Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_NO_REFERENCE), NULL);
    HWCheckPrintError(Message);
    Result->Byte = 0xFF;
  }
  else
  {
    if (HWInfoCurrent->CpuInfoCrc32 != HWInfo->CpuInfoCrc32)
    {
      Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_CPU), NULL);
      HWCheckPrintError(Message);
      Result->Bits.CpuMismatch = 0x01;
    }

    if (HWInfoCurrent->DimmInfoCrc32 != HWInfo->DimmInfoCrc32)
    {
      Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_DIMM), NULL);
      HWCheckPrintError(Message);
      Result->Bits.DimmMismatch = 0x01;
    }

    if (HWInfoCurrent->DisksInfoCrc32 != HWInfo->DisksInfoCrc32)
    {
      Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_DISKS), NULL);
      HWCheckPrintError(Message);
      Result->Bits.DisksMismatch = 0x01;
    }

    if (HWInfoCurrent->PciInfoCrc32 != HWInfo->PciInfoCrc32)
    {
      Message = HiiGetString(mConfigPrivate->HiiHandle, STRING_TOKEN(STR_ERROR_PCI), NULL);
      HWCheckPrintError(Message);
      Result->Bits.PciMismatch = 0x01;
    }

  } // if reference found

  if (NULL != HWInfoCurrent)
  {
    FreePool(HWInfoCurrent);
  }

  if (0x00 != Result->Byte)
  {
    Print((CHAR16*)L"%02X\n", Result->Byte);
    return EFI_SECURITY_VIOLATION;
  }

  return Status;
}


/**
  The entry point for HWCheckDxe driver.

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
HWCheckDxeEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  HW_CHECK_CONFIGURATION Configuration;
  HWCHECK_INFO HWInfo;
  HWCHECK_RESULT Result;
  EFI_EVENT   Event;

  mConfigPrivate = (HW_CHECK_CONFIG_PRIVATE_DATA*) AllocateCopyPool (sizeof (HW_CHECK_CONFIG_PRIVATE_DATA), &mHWCheckConfigPrivateDataTemplate);
  if (mConfigPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = InstallHWCheckConfigForm (mConfigPrivate);

    if (EFI_ERROR(Status))
  {
    return Status;
  }

  Configuration = mConfigPrivate->ConfigStore;

  if (0x00 == Configuration.HWCheckEnable)
  {
    return Status;
  }

  ZeroMem(&HWInfo, sizeof(HWCHECK_INFO));

  Status = HWCheckGetHardwareInfo(&HWInfo);

  Status = HWCheckCompare(&HWInfo, &Result);
  if (EFI_ERROR(Status))
  {
    if (0x00 != Configuration.HWCheckSystemHalt)
    {
      Status = EfiCreateEventReadyToBootEx(TPL_CALLBACK, ReadyToBootNotify, NULL, &Event);
    }
    if (0x00 != Configuration.HWCheckInformBmc)
    {
      Status = IpmiSendCommand(
        IPMI_COMMAND_HW_CHECK_RESULT,
        (UINT8*) &Result,
        sizeof(Result),
        NULL,
        0x00
      );
    }
  }

  return Status;
}
