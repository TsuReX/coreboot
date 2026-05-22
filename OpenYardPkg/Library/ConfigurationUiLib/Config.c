/** @file
  The boot manager reference implementation

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Config.h"

//UINT16    mKeyInput;
EFI_GUID  mConfigGuid = CONFIG_FORMSET_GUID;
//
// Boot video resolution and text mode.
//
//UINT32  mBmBootHorizontalResolution = 0;
//UINT32  mBmBootVerticalResolution   = 0;
//UINT32  mBmBootTextModeColumn       = 0;
//UINT32  mBmBootTextModeRow          = 0;
//
// BIOS setup video resolution and text mode.
//
//UINT32  mBmSetupTextModeColumn       = 0;
//UINT32  mBmSetupTextModeRow          = 0;
//UINT32  mBmSetupHorizontalResolution = 0;
//UINT32  mBmSetupVerticalResolution   = 0;

//BOOLEAN  mBmModeInitialized = FALSE;


CONFIG_CALLBACK_DATA  gConfigPrivate = {
  CONFIG_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    ConfigExtractConfig,
    ConfigRouteConfig,
    ConfigCallback
  }
};


HII_VENDOR_DEVICE_PATH  mConfigHiiVendorDevicePath = {
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
  { 0x1dddbe15, 0x481d, 0x4d2b, { 0x82, 0x77, 0xb1, 0x91, 0xea, 0xf6, 0x65, 0x36 }
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
  Extract device path for given HII handle and class guid.

  @param Handle          The HII handle.

  @retval  NULL          Fail to get the device path string.
  @return  PathString    Get the device path string.

**/
CHAR16*
ConfigExtractDevicePathFromHiiHandle(
  IN      EFI_HII_HANDLE  Handle
)
{
  EFI_STATUS  Status;
  EFI_HANDLE  DriverHandle;

  ASSERT(Handle != NULL);

  if (Handle == NULL) {
    return NULL;
  }

  Status = gHiiDatabase->GetPackageListHandle(gHiiDatabase, Handle, &DriverHandle);
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  //
  // Get device path string.
  //
  return ConvertDevicePathToText(DevicePathFromHandle(DriverHandle), FALSE, FALSE);
}


/**
  Dynamic create Hii information for Device Manager.

  @param   NextShowFormId     The FormId which need to be show.

**/
VOID
CreateConfigForm(
  IN EFI_FORM_ID  NextShowFormId
)
{
  UINTN               Index;
  EFI_STRING          String;
  EFI_STRING_ID       Token;
  EFI_STRING_ID       TokenHelp;
  EFI_HII_HANDLE* HiiHandles;
  EFI_HII_HANDLE      HiiHandle;
  EFI_GUID            FormSetGuid;
  VOID* StartOpCodeHandle;
  VOID* EndOpCodeHandle;
  EFI_IFR_GUID_LABEL* StartLabel;
  EFI_IFR_GUID_LABEL* EndLabel;
//  BOOLEAN             AddNetworkMenu;
//  UINTN               AddItemCount;
  CHAR16* DevicePathStr;
  EFI_STRING_ID       DevicePathId;
  EFI_IFR_FORM_SET* Buffer;
  UINTN               BufferSize;
  UINT8               ClassGuidNum;
  EFI_GUID* ClassGuid;
  UINTN               TempSize;
  UINT8* Ptr;
  EFI_STATUS          Status;

  TempSize = 0;
  BufferSize = 0;
  Buffer = NULL;

  HiiHandle = gConfigPrivate.HiiHandle;
//  AddNetworkMenu = FALSE;
//  AddItemCount = 0;

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle();
  ASSERT(StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle();
  ASSERT(EndOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL*)HiiCreateGuidOpCode(StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  //
  // According to the next show Form id(mNextShowFormId) to decide which form need to update.
  //
  StartLabel->Number = (UINT16)(LABEL_FORM_ID_OFFSET + NextShowFormId);

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL*)HiiCreateGuidOpCode(EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number = LABEL_END;

  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles(NULL);
  ASSERT(HiiHandles != NULL);

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    Status = HiiGetFormSetFromHiiHandle(HiiHandles[Index], &Buffer, &BufferSize);
    if (EFI_ERROR(Status)) {
      continue;
    }

    Ptr = (UINT8*)Buffer;
    while (TempSize < BufferSize) {
      TempSize += ((EFI_IFR_OP_HEADER*)Ptr)->Length;
      if (((EFI_IFR_OP_HEADER*)Ptr)->Length <= OFFSET_OF(EFI_IFR_FORM_SET, Flags)) {
        Ptr += ((EFI_IFR_OP_HEADER*)Ptr)->Length;
        continue;
      }

      ClassGuidNum = (UINT8)(((EFI_IFR_FORM_SET*)Ptr)->Flags & 0x3);
      ClassGuid = (EFI_GUID*)(VOID*)(Ptr + sizeof(EFI_IFR_FORM_SET));
      while (ClassGuidNum-- > 0) {
        if (CompareGuid(&gEfiHiiPlatformConfigFormsetGuid, ClassGuid) == 0) {
          ClassGuid++;
          continue;
        }

        String = HiiGetString(HiiHandles[Index], ((EFI_IFR_FORM_SET*)Ptr)->FormSetTitle, NULL);
        if (String == NULL) {
          String = HiiGetString(HiiHandle, STRING_TOKEN(STR_MISSING_STRING), NULL);
          ASSERT(String != NULL);
        }

        Token = HiiSetString(HiiHandle, 0, String, NULL);
        FreePool(String);

        String = HiiGetString(HiiHandles[Index], ((EFI_IFR_FORM_SET*)Ptr)->Help, NULL);
        if (String == NULL) {
          String = HiiGetString(HiiHandle, STRING_TOKEN(STR_MISSING_STRING), NULL);
          ASSERT(String != NULL);
        }

        TokenHelp = HiiSetString(HiiHandle, 0, String, NULL);
        FreePool(String);

        CopyMem(&FormSetGuid, &((EFI_IFR_FORM_SET*)Ptr)->Guid, sizeof(EFI_GUID));

        //
        // Not network device process, only need to show at device manger form.
        //
        if (NextShowFormId == FORM_CONFIG_INFO_ID) {
          DevicePathStr = ConfigExtractDevicePathFromHiiHandle(HiiHandles[Index]);
          DevicePathId = 0;
          if (DevicePathStr != NULL) {
            DevicePathId = HiiSetString(HiiHandle, 0, DevicePathStr, NULL);
            FreePool(DevicePathStr);
          }

          HiiCreateGotoExOpCode(
            StartOpCodeHandle,
            0,
            Token,
            TokenHelp,
            0,
            (EFI_QUESTION_ID)(Index + DEVICE_KEY_OFFSET),
            0,
            &FormSetGuid,
            DevicePathId
          );
        }

        break;
      }

      Ptr += ((EFI_IFR_OP_HEADER*)Ptr)->Length;
    }

    FreePool(Buffer);
    Buffer = NULL;
    TempSize = 0;
    BufferSize = 0;
  }

  HiiUpdateForm(
    HiiHandle,
    &mConfigGuid,
    NextShowFormId,
    StartOpCodeHandle,
    EndOpCodeHandle
  );

  HiiFreeOpCodeHandle(StartOpCodeHandle);
  HiiFreeOpCodeHandle(EndOpCodeHandle);
  FreePool(HiiHandles);
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
ConfigExtractConfig (
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
ConfigRouteConfig (
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
ConfigCallback (
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
ConfigUiLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //
  gConfigPrivate.DriverHandle = NULL;
  Status                           = gBS->InstallMultipleProtocolInterfaces (
                                            &gConfigPrivate.DriverHandle,
                                            &gEfiDevicePathProtocolGuid,
                                            &mConfigHiiVendorDevicePath,
                                            &gEfiHiiConfigAccessProtocolGuid,
                                            &gConfigPrivate.ConfigAccess,
                                            NULL
                                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  gConfigPrivate.HiiHandle = HiiAddPackages (
                                    &mConfigGuid,
                                    gConfigPrivate.DriverHandle,
                                    ConfigVfrBin,
                                    ConfigUiLibStrings,
                                    NULL
                                    );
  ASSERT (gConfigPrivate.HiiHandle != NULL);

  CreateConfigForm(FORM_CONFIG_INFO_ID);

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
ConfigUiLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  gConfigPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gConfigPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  HiiRemovePackages (gConfigPrivate.HiiHandle);

  return EFI_SUCCESS;
}
