/** @file
  HII Config Access protocol implementation of ChipsetSetupDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016-2018 Hewlett Packard Enterprise Development LP<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "ChipsetSetupImpl.h"

EFI_GUID   mSetupVarGuid = OY_SETUP_VARIABLE_GUID;
CHAR16     mSetupVarName[] = OY_SETUP_VARIABLE_NAME;

CHIPSET_SETUP_CONFIG_PRIVATE_DATA mChipsetSetupConfigPrivateDataTemplate = {
  CHIPSET_SETUP_CONFIG_PRIVATE_DATA_SIGNATURE,
  {
    0
  },
  {
    ChipsetSetupExtractConfig,
    ChipsetSetupRouteConfig,
    ChipsetSetupCallback
  }
};

HII_VENDOR_DEVICE_PATH       mChipsetSetupHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    CHIPSET_SETUP_FORM_SET_GUID
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
InstallChipsetSetupConfigForm (
  IN OUT CHIPSET_SETUP_CONFIG_PRIVATE_DATA       *ConfigPrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;
  UINTN                            BufferSize;

  DriverHandle = NULL;
  ConfigAccess = &ConfigPrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mChipsetSetupHiiVendorDevicePath,
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

  EFI_GUID gChipsetSetupFormSetGuid = CHIPSET_SETUP_FORM_SET_GUID;

  //
  // Publish the HII package list
  //
  HiiHandle = HiiAddPackages (
                &gChipsetSetupFormSetGuid,
                DriverHandle,
                ChipsetSetupDxeStrings,
                ChipsetSetupHiiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mChipsetSetupHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           ConfigAccess,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  ConfigPrivateData->HiiHandle = HiiHandle;
  
  BufferSize = sizeof(OY_SETUP_CONFIGURATION);

  // IF driver is not part of the Platform then need to get/set defaults for the NVRAM configuration that the driver will use.
  Status = gRT->GetVariable(
    mSetupVarName,
    &mSetupVarGuid,
    NULL,
    &BufferSize,
    &ConfigPrivateData->ConfigStore
  );
  if (EFI_ERROR(Status)) {  // Not definded yet so add it to the NV Variables.
    Status = gRT->SetVariable(
      mSetupVarName,
      &mSetupVarGuid,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
      sizeof(OY_SETUP_CONFIGURATION),
      &ConfigPrivateData->ConfigStore   //  buffer is 000000  now
    );
  }

  return EFI_SUCCESS;
}


/**
  This function removes RAM disk configuration Form.

  @param[in, out]  ConfigPrivateData
                             Points to RAM disk configuration private data.

**/
VOID
UninstallChipsetSetupConfigForm (
  IN OUT CHIPSET_SETUP_CONFIG_PRIVATE_DATA       *ConfigPrivateData
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
           &mChipsetSetupHiiVendorDevicePath,
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
ChipsetSetupExtractConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Request,
       OUT EFI_STRING                       *Progress,
       OUT EFI_STRING                       *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  CHIPSET_SETUP_CONFIG_PRIVATE_DATA* ConfigPrivate;
  EFI_HII_CONFIG_ROUTING_PROTOCOL* HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Initialize the local variables.
  //
  ConfigRequestHdr = NULL;
  ConfigRequest = NULL;
  Size = 0;
  *Progress = Request;
  AllocatedRequest = FALSE;
  ConfigPrivate = CHIPSET_SETUP_CONFIG_PRIVATE_FROM_THIS(This);
  HiiConfigRouting = ConfigPrivate->HiiConfigRouting;
  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof(OY_SETUP_CONFIGURATION);
  Status = gRT->GetVariable(
    mSetupVarName,
    &mSetupVarGuid,
    NULL,
    &BufferSize,
    &ConfigPrivate->ConfigStore
  );
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }
  if (Request != NULL) {
    ConfigRequest = Request;
  }
  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = HiiConfigRouting->BlockToConfig(
    HiiConfigRouting,
    ConfigRequest,
    (UINT8*)&ConfigPrivate->ConfigStore,
    BufferSize,
    Results,
    Progress
  );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool(ConfigRequest);
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  }
  else if (StrStr(Request, (CHAR16*)L"OFFSET") == NULL) {
    *Progress = Request + StrLen(Request);
  }
  return Status;

  //return EFI_NOT_FOUND;
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
ChipsetSetupRouteConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Configuration,
       OUT EFI_STRING                       *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  CHIPSET_SETUP_CONFIG_PRIVATE_DATA* ConfigPrivate;
  EFI_HII_CONFIG_ACCESS_PROTOCOL* ConfigAccess;
  EFI_HII_CONFIG_ROUTING_PROTOCOL* ConfigRouting;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ConfigPrivate = CHIPSET_SETUP_CONFIG_PRIVATE_FROM_THIS(This);
  ConfigAccess = &ConfigPrivate->ConfigAccess;
  ConfigRouting = ConfigPrivate->HiiConfigRouting;
  *Progress = Configuration;

  //
  // Get Buffer Storage data from EFI variable
  //
  BufferSize = sizeof(OY_SETUP_CONFIGURATION);
  Status = gRT->GetVariable(
    mSetupVarName,
    &mSetupVarGuid,
    NULL,
    &BufferSize,
    &ConfigPrivate->ConfigStore
  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  BufferSize = sizeof(OY_SETUP_CONFIGURATION);
  Status = ConfigRouting->ConfigToBlock(
    ConfigRouting,
    Configuration,
    (UINT8*)&ConfigPrivate->ConfigStore,
    &BufferSize,
    Progress
  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Store Buffer Storage back to EFI variable
  //
  Status = gRT->SetVariable(
    mSetupVarName,
    &mSetupVarGuid,
    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
    sizeof(OY_SETUP_CONFIGURATION),
    &ConfigPrivate->ConfigStore
  );

  return Status;


  //return EFI_NOT_FOUND;
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
ChipsetSetupCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                 Action,
  IN     EFI_QUESTION_ID                    QuestionId,
  IN     UINT8                              Type,
  IN     EFI_IFR_TYPE_VALUE                 *Value,
     OUT EFI_BROWSER_ACTION_REQUEST         *ActionRequest
  )
{
  CHIPSET_SETUP_CONFIG_PRIVATE_DATA    *ConfigPrivate;
  EFI_STATUS                      Status;


  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE)) ||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  ConfigPrivate = CHIPSET_SETUP_CONFIG_PRIVATE_FROM_THIS(This);

  switch (Action) {
  case EFI_BROWSER_ACTION_CHANGING:
  {
    if (QuestionId == 0x00) {
      //break;
    }
	
    //break;
  }
  }

  return Status;

  //  return EFI_UNSUPPORTED;

}
