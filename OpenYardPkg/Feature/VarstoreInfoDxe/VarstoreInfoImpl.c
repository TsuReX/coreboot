/** @file
  HII Config Access protocol implementation of VarstoreInfoDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016-2018 Hewlett Packard Enterprise Development LP<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "VarstoreInfoImpl.h"


VARSTORE_INFO_PRIVATE_DATA mVarstoreInfoPrivateDataTemplate = {
  VARSTORE_INFO_PRIVATE_DATA_SIGNATURE,
  {
    0,
    0,
    0
  },
  {
    VarstoreInfoExtractConfig,
    VarstoreInfoRouteConfig,
    VarstoreInfoCallback
  }
};

HII_VENDOR_DEVICE_PATH       mVarstoreInfoHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    VARSTORE_INFO_FORM_SET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


/**
  This function publish the RAM disk configuration Form.

  @param[in, out]  ConfigPrivateData
                             Points to RAM disk configuration private data.

  @retval EFI_SUCCESS             HII Form is installed successfully.
  @retval EFI_OUT_OF_RESOURCES    Not enough resource for HII Form installation.
  @retval Others                  Other errors as indicated.

**/
EFI_STATUS
InstallVarstoreInfoConfigForm (
  IN OUT VARSTORE_INFO_PRIVATE_DATA       *PrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;

  DriverHandle = NULL;
  ConfigAccess = &PrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mVarstoreInfoHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol(&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID**)&HiiConfigRouting);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  PrivateData->HiiConfigRouting = HiiConfigRouting;

  PrivateData->DriverHandle = DriverHandle;

  EFI_GUID gVarstoreInfoFormSetGuid = VARSTORE_INFO_FORM_SET_GUID;

  //
  // Publish the HII package list
  //
  HiiHandle = HiiAddPackages (
                &gVarstoreInfoFormSetGuid,
                DriverHandle,
                VarstoreInfoDxeStrings,
                VarstoreInfoHiiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mVarstoreInfoHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           ConfigAccess,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->HiiHandle = HiiHandle;

  return EFI_SUCCESS;
}


/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param[in]  This           Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Request        A null-terminated Unicode string in
                             <ConfigRequest> format.
  @param[out] Progress       On return, points to a character in the Request
                             string. Points to the string's null terminator if
                             request was successful. Points to the most recent
                             '&' before the first failing name/value pair (or
                             the beginning of the string if the failure is in
                             the first name/value pair) if the request was not
                             successful.
  @param[out] Results        A null-terminated Unicode string in
                             <ConfigAltResp> format which has all values filled
                             in for the names in the Request string. String to
                             be allocated by the called function.

  @retval EFI_SUCCESS             The Results is filled with the requested
                                  values.
  @retval EFI_OUT_OF_RESOURCES    Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER   Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND           Routing data doesn't match any storage in
                                  this driver.

**/
EFI_STATUS
EFIAPI
VarstoreInfoExtractConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Request,
       OUT EFI_STRING                       *Progress,
       OUT EFI_STRING                       *Results
  )
{
  return EFI_NOT_FOUND;
}


/**
  This function processes the results of changes in configuration.

  @param[in]  This           Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Configuration  A null-terminated Unicode string in <ConfigResp>
                             format.
  @param[out] Progress       A pointer to a string filled in with the offset of
                             the most recent '&' before the first failing
                             name/value pair (or the beginning of the string if
                             the failure is in the first name/value pair) or
                             the terminating NULL if all was successful.

  @retval EFI_SUCCESS             The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER   Configuration is NULL.
  @retval EFI_NOT_FOUND           Routing data doesn't match any storage in
                                  this driver.

**/
EFI_STATUS
EFIAPI
VarstoreInfoRouteConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Configuration,
       OUT EFI_STRING                       *Progress
  )
{
  return EFI_NOT_FOUND;
}


/**
  This function processes the results of changes in configuration.

  @param[in]  This           Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Action         Specifies the type of action taken by the browser.
  @param[in]  QuestionId     A unique value which is sent to the original
                             exporting driver so that it can identify the type
                             of data to expect.
  @param[in]  Type           The type of value for the question.
  @param[in]  Value          A pointer to the data being sent to the original
                             exporting driver.
  @param[out] ActionRequest  On return, points to the action requested by the
                             callback function.

  @retval EFI_SUCCESS             The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES    Not enough storage is available to hold the
                                  variable and its data.
  @retval EFI_DEVICE_ERROR        The variable could not be saved.
  @retval EFI_UNSUPPORTED         The specified Action is not supported by the
                                  callback.

**/
EFI_STATUS
EFIAPI
VarstoreInfoCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                 Action,
  IN     EFI_QUESTION_ID                    QuestionId,
  IN     UINT8                              Type,
  IN     EFI_IFR_TYPE_VALUE                 *Value,
     OUT EFI_BROWSER_ACTION_REQUEST         *ActionRequest
  )
{
  VARSTORE_INFO_PRIVATE_DATA    *PrivateData;
  EFI_STATUS                      Status;
  CHAR16 NewString[16];
  VARSTORE_INFO VarstoreInfo;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE)) ||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  PrivateData = VARSTORE_INFO_PRIVATE_FROM_THIS(This);

  switch (Action) {
    case EFI_BROWSER_ACTION_FORM_OPEN:
    {
      if (QuestionId == KEY_UPDATE) {

        gRT->QueryVariableInfo(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, &VarstoreInfo.MaximumVariableStorageSize, &VarstoreInfo.RemainingVariableStorageSize, &VarstoreInfo.MaximumVariableSize);

        UnicodeSPrint(NewString, sizeof(CHAR16) * 16, (CHAR16*)L"%d", VarstoreInfo.MaximumVariableStorageSize);
        HiiSetString(PrivateData->HiiHandle, STRING_TOKEN(MAXIMUM_VARIABLE_STORAGE_SIZE_STRING2), NewString, NULL);
        ZeroMem(NewString, 32);

        UnicodeSPrint(NewString, sizeof(CHAR16) * 16, (CHAR16*)L"%d", VarstoreInfo.RemainingVariableStorageSize);
        HiiSetString(PrivateData->HiiHandle, STRING_TOKEN(REMAINING_VARIABLE_STORAGE_SIZE2), NewString, NULL);
        ZeroMem(NewString, 32);

        UnicodeSPrint(NewString, sizeof(CHAR16) * 16, (CHAR16*)L"%d", VarstoreInfo.MaximumVariableSize);
        HiiSetString(PrivateData->HiiHandle, STRING_TOKEN(MAXIMUM_VARIABLE_SIZE2), NewString, NULL);
        ZeroMem(NewString, 32);

        break;
      }
    }
  }

  return Status;

  //  return EFI_UNSUPPORTED;

}
