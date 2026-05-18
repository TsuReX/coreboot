/** @file
  HII Config Access protocol implementation of BmcLanSetupDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016-2018 Hewlett Packard Enterprise Development LP<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BmcLanSetupImpl.h"

BMC_LAN_SETUP_CONFIG_PRIVATE_DATA mBmcLanSetupConfigPrivateDataTemplate = {
  BMC_LAN_SETUP_CONFIG_PRIVATE_DATA_SIGNATURE,
  {
    BmcLanSetupExtractConfig,
    BmcLanSetupRouteConfig,
    BmcLanSetupCallback
  }
};

HII_VENDOR_DEVICE_PATH       mBmcLanSetupHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    BMC_LAN_SETUP_FORM_SET_GUID
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
InstallBmcLanSetupConfigForm (
  IN OUT BMC_LAN_SETUP_CONFIG_PRIVATE_DATA       *ConfigPrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;

  DriverHandle = NULL;

  ConfigPrivateData->Signature = BMC_LAN_SETUP_CONFIG_PRIVATE_DATA_SIGNATURE;

  ConfigPrivateData->ConfigAccess.Callback = BmcLanSetupCallback;
  ConfigPrivateData->ConfigAccess.ExtractConfig = BmcLanSetupExtractConfig;
  ConfigPrivateData->ConfigAccess.RouteConfig = BmcLanSetupRouteConfig;

  ConfigAccess = &ConfigPrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mBmcLanSetupHiiVendorDevicePath,
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

  ConfigPrivateData->HiiConfigRouting = HiiConfigRouting;

  ConfigPrivateData->DriverHandle = DriverHandle;

  EFI_GUID gBmcLanSetupFormSetGuid = BMC_LAN_SETUP_FORM_SET_GUID;

  //
  // Publish the HII package list
  //
  HiiHandle = HiiAddPackages (
                &gBmcLanSetupFormSetGuid,
                DriverHandle,
                BmcLanSetupDxeStrings,
                BmcLanSetupHiiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mBmcLanSetupHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           ConfigAccess,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  ConfigPrivateData->HiiHandle = HiiHandle;

  return EFI_SUCCESS;
}


/**
  This function removes RAM disk configuration Form.

  @param[in, out]  ConfigPrivateData
                             Points to RAM disk configuration private data.

**/
VOID
UninstallBmcLanSetupConfigForm (
  IN OUT BMC_LAN_SETUP_CONFIG_PRIVATE_DATA       *ConfigPrivateData
  )
{
  //
  // Uninstall HII package list
  //
  if (ConfigPrivateData->HiiHandle != NULL) {
    HiiRemovePackages (ConfigPrivateData->HiiHandle);
    ConfigPrivateData->HiiHandle = NULL;
  }

  //
  // Uninstall HII Config Access Protocol
  //
  if (ConfigPrivateData->DriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           ConfigPrivateData->DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mBmcLanSetupHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           &ConfigPrivateData->ConfigAccess,
           NULL
           );
    ConfigPrivateData->DriverHandle = NULL;
  }

  FreePool (ConfigPrivateData);
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
BmcLanSetupExtractConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Request,
       OUT EFI_STRING                       *Progress,
       OUT EFI_STRING                       *Results
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
BmcLanSetupRouteConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Configuration,
       OUT EFI_STRING                       *Progress
  )
{
  if ((Configuration == NULL) || (Progress == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Configuration;

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
BmcLanSetupCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                 Action,
  IN     EFI_QUESTION_ID                    QuestionId,
  IN     UINT8                              Type,
  IN     EFI_IFR_TYPE_VALUE                 *Value,
  OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{
  EFI_STATUS              Status;

  BMC_LAN_SETUP_CONFIG_PRIVATE_DATA *PrivateData;

  CALLBACK_PARAMETERS     CallbackParameters;

  CallbackParameters.This = (VOID*)This;
  CallbackParameters.Action = Action;
  CallbackParameters.KeyValue = QuestionId;
  CallbackParameters.Type = Type;
  CallbackParameters.Value = Value;
  CallbackParameters.ActionRequest = ActionRequest;
  if (ActionRequest) {
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  }
  Status = EFI_UNSUPPORTED;

  PrivateData = BMC_LAN_SETUP_CONFIG_PRIVATE_FROM_THIS(This);


  Status = BmcLanIPValidation(PrivateData->HiiHandle, QuestionId, &CallbackParameters);
  Status = BmcLanConfigCallbackFunction(PrivateData->HiiHandle, QuestionId, &CallbackParameters);

  //BmcLanIpv6ConfigCallbackFunction(PrivateData->HiiHandle, QuestionId, &CallbackParameters);
  //BmcLanIpv6Router1ConfigCallbackFunction(PrivateData->HiiHandle, QuestionId, &CallbackParameters);
  //BmcLanIpv6Router1PrefixValueCallbackFunction(PrivateData->HiiHandle, QuestionId, &CallbackParameters);
  //BmcLanIpv6SupportCallbackFunction(PrivateData->HiiHandle, QuestionId, &CallbackParameters);

  Status = BmcLanParamSetupDisplayCallback(PrivateData->HiiHandle, QuestionId, &CallbackParameters);
  //BmcLanParamSetupDisplay(PrivateData->HiiHandle);

  if (EFI_BROWSER_ACTION_SUBMITTED == Action)
  {
    Status = InitializeIp4BmcLanConfig();

    /*
    EFI_INPUT_KEY  Key;

    CHAR16* CharPtrString1 = NULL;
    CHAR16* CharPtrString2 = HiiGetString(
      PrivateData->HiiHandle,
      STRING_TOKEN(STR_PRESS_ENTER_TO_CONTINUE),
      NULL);

    if (!EFI_ERROR(Status))
    {
      CharPtrString1 = HiiGetString(
        PrivateData->HiiHandle,
        STRING_TOKEN(STR_CONFIG_CHANGED),
        NULL);
    }
    else
    {
      CharPtrString1 = HiiGetString(
        PrivateData->HiiHandle,
        STRING_TOKEN(STR_BMCLAN_ERROR_INFO),
        NULL);
    }

    ZeroMem(&Key, sizeof(EFI_INPUT_KEY));

    if ((CharPtrString1 != NULL) && (CharPtrString2 != NULL)) {
      do {
        CreatePopUp(
          EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE | EFI_BACKGROUND_RED,
          &Key,
          CharPtrString1,
          CharPtrString2,
          NULL
        );
      } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
      gBS->FreePool(CharPtrString1);
      gBS->FreePool(CharPtrString2);
    }
    */

    return Status;
  }

  return EFI_SUCCESS;
}
