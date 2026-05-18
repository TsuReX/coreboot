/** @file
  The boot manager reference implementation

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Info.h"

UINT16    mKeyInput;
EFI_GUID  mInfoGuid = INFO_FORMSET_GUID;
//
// Boot video resolution and text mode.
//
UINT32  mBmBootHorizontalResolution = 0;
UINT32  mBmBootVerticalResolution   = 0;
UINT32  mBmBootTextModeColumn       = 0;
UINT32  mBmBootTextModeRow          = 0;
//
// BIOS setup video resolution and text mode.
//
UINT32  mBmSetupTextModeColumn       = 0;
UINT32  mBmSetupTextModeRow          = 0;
UINT32  mBmSetupHorizontalResolution = 0;
UINT32  mBmSetupVerticalResolution   = 0;

BOOLEAN  mBmModeInitialized = FALSE;


INFO_CALLBACK_DATA  gInfoPrivate = {
  INFO_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    InfoExtractConfig,
    InfoRouteConfig,
    InfoCallback
  }
};


HII_VENDOR_DEVICE_PATH  mInfoHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
        (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8)
      }
    },
  //
  // {1DDDBE15-481D-4d2b-8277-B191EAF66535}
  //
  { 0x1dddbe15, 0x481d, 0x4d2b, { 0x82, 0x77, 0xb1, 0x91, 0xea, 0xf6, 0x65, 0x35 }
  }
},
{
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  {
    (UINT8)(END_DEVICE_PATH_LENGTH),
    (UINT8)((END_DEVICE_PATH_LENGTH) >> 8)
  }
}
};


/**

  Update the banner information for the Front Page based on Smbios information.

**/
VOID
_UpdateFrontPageBannerStrings(
  VOID
)
{
  UINT8                    StrIndex;
  CHAR16* NewString;
  CHAR16* FirmwareVersionString;
  EFI_STATUS               Status;
  EFI_SMBIOS_HANDLE        SmbiosHandle;
  EFI_SMBIOS_PROTOCOL* Smbios;
  SMBIOS_TABLE_TYPE0* Type0Record;
  SMBIOS_TABLE_TYPE1* Type1Record;
  //SMBIOS_TABLE_TYPE4* Type4Record;
  SMBIOS_TABLE_TYPE19* Type19Record;
  EFI_SMBIOS_TABLE_HEADER* Record;
  UINT64                   InstalledMemory;
  BOOLEAN                  FoundCpu;

  InstalledMemory = 0;
  FoundCpu = 0;

  //
  // Update default banner string.
  //
  //NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE4_LEFT), NULL);
  //HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE4_LEFT), NewString, NULL);
  //FreePool(NewString);
  //
  //NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE4_RIGHT), NULL);
  //HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE4_RIGHT), NewString, NULL);
  //FreePool(NewString);
  //
  //NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE5_LEFT), NULL);
  //HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE5_LEFT), NewString, NULL);
  //FreePool(NewString);
  //
  //NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE5_RIGHT), NULL);
  //HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CUSTOMIZE_BANNER_LINE5_RIGHT), NewString, NULL);
  //FreePool(NewString);

  //
  // Update Front Page banner strings base on SmBios Table.
  //
  Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);
  if (EFI_ERROR(Status)) {
    //
    // Smbios protocol not found, get the default value.
    //
    NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_COMPUTER_MODEL_VALUE), NULL);
    HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_COMPUTER_MODEL_VALUE), NewString, NULL);
    FreePool(NewString);
    /*
    NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_MODEL_VALUE), NULL);
    HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_MODEL_VALUE), NewString, NULL);
    FreePool(NewString);

    NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_SPEED_VALUE), NULL);
    HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_SPEED_VALUE), NewString, NULL);
    FreePool(NewString);
    */
    NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_BIOS_VERSION), NULL);
    HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_BIOS_VERSION_VALUE), NewString, NULL);
    FreePool(NewString);

    NewString = HiiGetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_MEMORY_SIZE_VALUE), NULL);
    HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_MEMORY_SIZE_VALUE), NewString, NULL);
    FreePool(NewString);

    return;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->GetNext(Smbios, &SmbiosHandle, NULL, &Record, NULL);
  while (!EFI_ERROR(Status)) {
    if (Record->Type == SMBIOS_TYPE_BIOS_INFORMATION) {
      Type0Record = (SMBIOS_TABLE_TYPE0*)Record;
      StrIndex = Type0Record->BiosVersion;
      _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type0Record + Type0Record->Hdr.Length), StrIndex, &NewString);

      FirmwareVersionString = (CHAR16*)PcdGetPtr(PcdFirmwareVersionString);
      if (*FirmwareVersionString != 0x0000) {
        FreePool(NewString);
        NewString = (CHAR16*)PcdGetPtr(PcdFirmwareVersionString);
        HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_BIOS_VERSION_VALUE), NewString, NULL);
      }
      else {
        HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_BIOS_VERSION_VALUE), NewString, NULL);
        FreePool(NewString);
      }
    }

    if (Record->Type == SMBIOS_TYPE_SYSTEM_INFORMATION) {
      Type1Record = (SMBIOS_TABLE_TYPE1*)Record;
      StrIndex = Type1Record->ProductName;
      _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type1Record + Type1Record->Hdr.Length), StrIndex, &NewString);
      HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_COMPUTER_MODEL_VALUE), NewString, NULL);
      FreePool(NewString);
    }

    /*
    if ((Record->Type == SMBIOS_TYPE_PROCESSOR_INFORMATION) && !FoundCpu) {
      Type4Record = (SMBIOS_TABLE_TYPE4*)Record;
      //
      // The information in the record should be only valid when the CPU Socket is populated.
      //
      if ((Type4Record->Status & SMBIOS_TYPE4_CPU_SOCKET_POPULATED) == SMBIOS_TYPE4_CPU_SOCKET_POPULATED) {
        StrIndex = Type4Record->ProcessorVersion;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type4Record + Type4Record->Hdr.Length), StrIndex, &NewString);
        HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_MODEL_VALUE), NewString, NULL);
        FreePool(NewString);

        _ConvertProcessorToString(Type4Record->CurrentSpeed, 6, &NewString);
        HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_CPU_SPEED_VALUE), NewString, NULL);
        FreePool(NewString);

        FoundCpu = TRUE;
      }
    }
    */

    if (Record->Type == SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS) {
      Type19Record = (SMBIOS_TABLE_TYPE19*)Record;
      if (Type19Record->StartingAddress != 0xFFFFFFFF) {
        InstalledMemory += RShiftU64(
          Type19Record->EndingAddress -
          Type19Record->StartingAddress + 1,
          10
        );
      }
      else {
        InstalledMemory += RShiftU64(
          Type19Record->ExtendedEndingAddress -
          Type19Record->ExtendedStartingAddress + 1,
          20
        );
      }
    }

    Status = Smbios->GetNext(Smbios, &SmbiosHandle, NULL, &Record, NULL);
  }

  //
  // Now update the total installed RAM size
  //
  _ConvertMemorySizeToString((UINT32)InstalledMemory, &NewString);
  HiiSetString(gInfoPrivate.HiiHandle, STRING_TOKEN(STR_MEMORY_SIZE_VALUE), NewString, NULL);
  FreePool(NewString);
}


/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
InfoExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  CONST EFI_STRING                      Request,
  OUT EFI_STRING                            *Progress,
  OUT EFI_STRING                            *Results
  )
{
  if ((Progress == NULL) || (Results == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Request;
  return EFI_NOT_FOUND;
}

/**
  This function processes the results of changes in configuration.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param Progress        A pointer to a string filled in with the offset of the most
                         recent '&' before the first failing name/value pair (or the
                         beginning of the string if the failure is in the first
                         name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
InfoRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  CONST EFI_STRING                      Configuration,
  OUT EFI_STRING                            *Progress
  )
{
  if ((Configuration == NULL) || (Progress == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Configuration;

  return EFI_NOT_FOUND;
}


/**
  This call back function is registered with Boot Manager formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
InfoCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  EFI_BROWSER_ACTION                    Action,
  IN  EFI_QUESTION_ID                       QuestionId,
  IN  UINT8                                 Type,
  IN  EFI_IFR_TYPE_VALUE                    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{

  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
    //
    // Means enter the boot manager form.
    // Update the boot manage page,because the boot option may changed.
    //
    if (QuestionId == 0x1212) {
      
    }

    return EFI_SUCCESS;
  }

  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    //
    // Do nothing for other UEFI Action. Only do call back when data is changed.
    //
    return EFI_UNSUPPORTED;
  }

  if ((Value == NULL) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Clear  the  screen  before.
  //
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
  gST->ConOut->ClearScreen (gST->ConOut);

  return EFI_SUCCESS;
}

/**

  Install Boot Manager Menu driver.

  @param ImageHandle     The image handle.
  @param SystemTable     The system table.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
  @retval  Other        Return error status.

**/
EFI_STATUS
EFIAPI
InfoUiLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //
  gInfoPrivate.DriverHandle = NULL;
  Status                           = gBS->InstallMultipleProtocolInterfaces (
                                            &gInfoPrivate.DriverHandle,
                                            &gEfiDevicePathProtocolGuid,
                                            &mInfoHiiVendorDevicePath,
                                            &gEfiHiiConfigAccessProtocolGuid,
                                            &gInfoPrivate.ConfigAccess,
                                            NULL
                                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  gInfoPrivate.HiiHandle = HiiAddPackages (
                                    &mInfoGuid,
                                    gInfoPrivate.DriverHandle,
                                    InfoVfrBin,
                                    InfoUiLibStrings,
                                    NULL
                                    );
  ASSERT (gInfoPrivate.HiiHandle != NULL);

  InitFruStrings(gInfoPrivate.HiiHandle);
  DisplayCpus(gInfoPrivate.HiiHandle);
  DisplayMemTopology(gInfoPrivate.HiiHandle);
  _UpdateFrontPageBannerStrings();

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.
  @param[in]  SystemTable       System Table

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
InfoUiLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  gInfoPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mInfoHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gInfoPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  HiiRemovePackages (gInfoPrivate.HiiHandle);

  return EFI_SUCCESS;
}
