/** @file
  This file include all platform action which can be customized
  by IBV/OEM.

Copyright (c) 2015 - 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PlatformBootManager.h"
#include "PlatformConsole.h"
#include <Protocol/FirmwareVolume2.h>

/**
  Check if device path is UEFI Shell
**/
BOOLEAN
IsShellDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL *Node;
  
  if (DevicePath == NULL) {
    return FALSE;
  }
  
  Node = DevicePath;
  while (!IsDevicePathEnd (Node)) {
    // Check for Media Device Path with Shell file
    if (DevicePathType (Node) == MEDIA_DEVICE_PATH) {
      if (DevicePathSubType (Node) == MEDIA_FILEPATH_DP) {
        FILEPATH_DEVICE_PATH *FileNode = (FILEPATH_DEVICE_PATH *)Node;
        if (FileNode->PathName != NULL) {
          if (StrStr (FileNode->PathName, L"Shell") != NULL ||
              StrStr (FileNode->PathName, L"shell") != NULL ||
              StrCmp (FileNode->PathName, L"\\Shell.efi") == 0 ||
              StrCmp (FileNode->PathName, L"\\EFI\\Shell.efi") == 0 ||
              StrCmp (FileNode->PathName, L"\\EFI\\Tools\\Shell.efi") == 0 ||
              StrCmp (FileNode->PathName, L"\\EFI\\Boot\\Shell.efi") == 0) {
            return TRUE;
          }
        }
      }
      
      // Check for Media Device Path with Shell GUID
      if (DevicePathSubType (Node) == MEDIA_PIWG_FW_FILE_DP) {
        MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFileNode = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)Node;
        if (CompareGuid (&FvFileNode->FvFileName, &gUefiShellFileGuid)) {
          return TRUE;
        }
      }
    }
    
    Node = NextDevicePathNode (Node);
  }
  
  return FALSE;
}

/**
  Check if device path is Network boot
**/
BOOLEAN
IsNetworkDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL *Node;
  
  if (DevicePath == NULL) {
    return FALSE;
  }

  Node = DevicePath;
  while (!IsDevicePathEnd (Node)) {
    UINT8 Type = DevicePathType (Node);

    // Messaging Device Path - это сетевые протоколы
    if (Type == MESSAGING_DEVICE_PATH) {
      return TRUE;
    }
    
    Node = NextDevicePathNode (Node);
  }

  return FALSE;
}

BOOLEAN
IsNetworkDescription (
  IN CHAR16 *Description
  )
{
  if (Description == NULL) {
    return FALSE;
  }

  // Проверяем типичные названия сетевых опций загрузки
  if (StrStr (Description, L"UEFI PXEv4") != NULL ||
      StrStr (Description, L"UEFI PXEv6") != NULL ||
      StrStr (Description, L"UEFI HTTPv4") != NULL || 
      StrStr (Description, L"UEFI HTTPv6") != NULL ||
      StrStr (Description, L"PXE") != NULL ||
      StrStr (Description, L"HTTP") != NULL) {
    return TRUE;
  }

  return FALSE;
}


/**
  Check if description indicates shell boot option
**/
BOOLEAN
IsShellDescription (
  IN CHAR16 *Description
  )
{
  if (Description == NULL) {
    return FALSE;
  }

  if (StrStr (Description, L"Shell") != NULL ||
      StrStr (Description, L"UEFI Shell") != NULL) {
    return TRUE;
  }

  return FALSE;
}

/**
  Set boot order from array of boot option numbers
**/
EFI_STATUS
SetBootOrder (
  IN UINT16  *BootOrder,
  IN UINTN   BootOrderSize
  )
{
  EFI_STATUS Status;
  UINTN      VarSize;
  
  if (BootOrder == NULL || BootOrderSize == 0) {
    DEBUG ((DEBUG_INFO, "SetBootOrder: Setting empty boot order - only Boot Manager will be available\n"));
    // Delete BootOrder variable - this will make only Boot Manager appear
    Status = gRT->SetVariable (
                    L"BootOrder",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    0,
                    NULL
                    );
    return Status;
  }
  
  VarSize = BootOrderSize * sizeof (UINT16);
  
  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  VarSize,
                  BootOrder
                  );
  
  DEBUG ((DEBUG_INFO, "SetBootOrder: Size=%d, Status=%r\n", BootOrderSize, Status));
  return Status;
}

/**
  Simplified version that handles boot order - if no options, only Boot Manager appears
**/
VOID
EFIAPI
BoardBdsUpdateBootOrder (
  VOID
  )
{
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN BootOptionCount;
  UINT16 *NewBootOrder = NULL;
  UINTN NewBootOrderSize = 0;
  UINTN Index;
  BOOLEAN EnableShellBoot = TRUE;
  BOOLEAN EnableNetworkBoot = TRUE;
  UINTN BoolVarSize;
  EFI_STATUS Status;
  BOOLEAN FoundAnyOption = FALSE;

  // Safe get variables
  BoolVarSize = sizeof(BOOLEAN);
  Status = gRT->GetVariable (
                  L"ShellBoot",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &BoolVarSize,
                  &EnableShellBoot
                  );
  if (EFI_ERROR(Status)) EnableShellBoot = TRUE;

  BoolVarSize = sizeof(BOOLEAN);
  Status = gRT->GetVariable (
                  L"NetworkBoot",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &BoolVarSize,
                  &EnableNetworkBoot
                  );
  if (EFI_ERROR(Status)) EnableNetworkBoot = TRUE;

  DEBUG ((
    DEBUG_INFO, 
    "BoardBdsUpdateBootOrder: Shell=%d Network=%d\n",
    EnableShellBoot,
    EnableNetworkBoot
    ));

  // Get all boot options
  BootOptions = EfiBootManagerGetLoadOptions (
                  &BootOptionCount, 
                  LoadOptionTypeBoot
                  );

  if (BootOptions == NULL || BootOptionCount == 0) {
    DEBUG ((DEBUG_INFO, "No boot options found - only Boot Manager will be available\n"));
    SetBootOrder (NULL, 0); // Empty boot order = only Boot Manager
    return;
  }

  DEBUG ((DEBUG_INFO, "Found %d total boot options\n", BootOptionCount));

  // Allocate memory for new boot order
  NewBootOrder = AllocateZeroPool (BootOptionCount * sizeof (UINT16));
  if (NewBootOrder == NULL) {
    DEBUG ((DEBUG_ERROR, "Failed to allocate memory for boot order\n"));
    EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
    return;
  }

  // Build new boot order with only active and enabled options
  for (Index = 0; Index < BootOptionCount; Index++) {
    EFI_BOOT_MANAGER_LOAD_OPTION *BootOption = &BootOptions[Index];
    BOOLEAN IsShellOption = FALSE;
    BOOLEAN IsNetworkOption = FALSE;
    BOOLEAN ShouldBeInBootOrder = TRUE;

    // Skip if not active
    if ((BootOption->Attributes & LOAD_OPTION_ACTIVE) == 0) {
      continue;
    }

    // Identify option type
    if (BootOption->Description != NULL) {
      IsShellOption = IsShellDescription(BootOption->Description);
      IsNetworkOption = IsNetworkDescription(BootOption->Description);
    }

    if (!IsShellOption && !IsNetworkOption && BootOption->FilePath != NULL) {
      IsShellOption = IsShellDevicePath (BootOption->FilePath);
      IsNetworkOption = IsNetworkDevicePath (BootOption->FilePath);
    }

    // Check if should be in boot order based on settings
    if (IsShellOption && !EnableShellBoot) {
      ShouldBeInBootOrder = FALSE;
      DEBUG ((
        DEBUG_INFO, 
        "Filtering out Shell: %s\n", 
        BootOption->Description ? BootOption->Description : L"Unknown"
      ));
    } else if (IsNetworkOption && !EnableNetworkBoot) {
      ShouldBeInBootOrder = FALSE;
      DEBUG ((
        DEBUG_INFO, 
        "Filtering out Network: %s\n", 
        BootOption->Description ? BootOption->Description : L"Unknown"
      ));
    }

    if (ShouldBeInBootOrder) {
      NewBootOrder[NewBootOrderSize++] = (UINT16)BootOption->OptionNumber;
      FoundAnyOption = TRUE;
      DEBUG ((
        DEBUG_INFO, 
        "Including in boot order: %s (Option: %d)\n", 
        BootOption->Description ? BootOption->Description : L"Unknown",
        BootOption->OptionNumber
      ));
    }
  }

  // Set new boot order - if no options, set empty to show only Boot Manager
  if (FoundAnyOption && NewBootOrderSize > 0) {
    Status = SetBootOrder (NewBootOrder, NewBootOrderSize);
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set boot order: %r\n", Status));
    } else {
      DEBUG ((DEBUG_INFO, "Boot order updated with %d entries\n", NewBootOrderSize));
    }
  } else {
    DEBUG ((DEBUG_INFO, "No boot options after filtering - user will see only Boot Manager\n"));
    SetBootOrder (NULL, 0); // Empty boot order = only Boot Manager appears
  }

  // Cleanup
  if (NewBootOrder != NULL) {
    FreePool (NewBootOrder);
  }
  EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
}


#define LANGUAGE        "en-us"

CHAR16*
GetHandleImageName(
  IN CONST EFI_HANDLE Handle
)
{
  EFI_STATUS                        Status;
  EFI_DRIVER_BINDING_PROTOCOL* DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL* DevPathNode;
  EFI_GUID* NameGuid;
  CHAR16* ImageName;
  UINTN                             BufferSize;
  UINT32                            AuthenticationStatus;
  EFI_FIRMWARE_VOLUME2_PROTOCOL* Fv2;

  LoadedImage = (EFI_LOADED_IMAGE_PROTOCOL*)NULL;
  DriverBinding = (EFI_DRIVER_BINDING_PROTOCOL*)NULL;
  ImageName = (CHAR16*)NULL;

  Status = gBS->OpenProtocol(
    Handle,
    &gEfiDriverBindingProtocolGuid,
    (VOID**)&DriverBinding,
    NULL,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );
  if (EFI_ERROR(Status)) {
    return (CHAR16*)NULL;
  }
  Status = gBS->OpenProtocol(
    DriverBinding->ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID**)&LoadedImage,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );
  if (!EFI_ERROR(Status)) {
    DevPathNode = LoadedImage->FilePath;
    if (DevPathNode == NULL) {
      return (CHAR16*)NULL;
    }
    while (!IsDevicePathEnd(DevPathNode)) {
      NameGuid = EfiGetNameGuidFromFwVolDevicePathNode((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH*)DevPathNode);
      if (NameGuid != NULL) {
        Status = gBS->HandleProtocol(
          LoadedImage->DeviceHandle,
          &gEfiFirmwareVolume2ProtocolGuid,
          (VOID**)&Fv2
        );
        if (!EFI_ERROR(Status)) {
          Status = Fv2->ReadSection(
            Fv2,
            NameGuid,
            EFI_SECTION_USER_INTERFACE,
            0,
            (VOID**)&ImageName,
            &BufferSize,
            &AuthenticationStatus
          );
          if (!EFI_ERROR(Status)) {
            break;
          }
          ImageName = (CHAR16*)NULL;
        }
      }
      //
      // Next device path node
      //
      DevPathNode = NextDevicePathNode(DevPathNode);
    }
    if (ImageName == NULL) {
      ImageName = ConvertDevicePathToText(LoadedImage->FilePath, TRUE, TRUE);
    }
  }
  return ImageName;
}

CHAR8*
EFIAPI
GetBestLanguageForDriver(
  IN CONST CHAR8* SupportedLanguages,
  IN CONST CHAR8* InputLanguage,
  IN BOOLEAN      Iso639Language
)
{
  CHAR8* LanguageVariable;
  CHAR8* BestLanguage;

  GetVariable2((CHAR16*)(Iso639Language ? L"Lang" : L"PlatformLang"), &gEfiGlobalVariableGuid, (VOID**)&LanguageVariable, (UINTN*)NULL);

  BestLanguage = GetBestLanguage(
    SupportedLanguages,
    Iso639Language,
    (InputLanguage != NULL) ? InputLanguage : "",
    (LanguageVariable != NULL) ? LanguageVariable : "",
    SupportedLanguages,
    NULL
  );

  if (LanguageVariable != NULL) {
    FreePool(LanguageVariable);
  }

  return BestLanguage;
}


CHAR16*
EFIAPI
GetHandleStringName(
  IN CONST EFI_HANDLE TheHandle,
  IN CONST CHAR8* Language
)
{
  EFI_COMPONENT_NAME2_PROTOCOL* CompNameStruct;
  EFI_STATUS                    Status;
  CHAR16* RetVal;
  CHAR8* BestLang;

  BestLang = (CHAR8*)NULL;

  Status = gBS->OpenProtocol(
    TheHandle,
    &gEfiComponentName2ProtocolGuid,
    (VOID**)&CompNameStruct,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (!EFI_ERROR(Status)) {
    BestLang = GetBestLanguageForDriver(CompNameStruct->SupportedLanguages, Language, FALSE);
    Status = CompNameStruct->GetDriverName(CompNameStruct, BestLang, &RetVal);
    if (BestLang != NULL) {
      FreePool(BestLang);
      BestLang = (CHAR8*)NULL;
    }
    if (!EFI_ERROR(Status)) {
      return (RetVal);
    }
  }
  Status = gBS->OpenProtocol(
    TheHandle,
    &gEfiComponentNameProtocolGuid,
    (VOID**)&CompNameStruct,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (!EFI_ERROR(Status)) {
    BestLang = GetBestLanguageForDriver(CompNameStruct->SupportedLanguages, Language, FALSE);
    Status = CompNameStruct->GetDriverName(CompNameStruct, BestLang, &RetVal);
    if (BestLang != NULL) {
      FreePool(BestLang);
    }
    if (!EFI_ERROR(Status)) {
      return (RetVal);
    }
  }
  return (CHAR16*)(NULL);
}


EFI_STATUS
EFIAPI
UnloadByName(CHAR16* DriverName
)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN HandleCount;
  EFI_HANDLE* HandleBuffer = (EFI_HANDLE*)NULL;

  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDriverBindingProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = EFI_NOT_FOUND;

  for (UINTN index = 0x00; index < HandleCount; index++) {

    CHAR16* HandleStringName = GetHandleStringName(HandleBuffer[index], LANGUAGE);
    CHAR16* HandleImageName = GetHandleImageName(HandleBuffer[index]);

    if (((CHAR16*)NULL != HandleStringName) && (0x00 != StrStr(HandleStringName, DriverName))) {
      Status = gBS->UnloadImage(HandleBuffer[index]);
    }
    if (((CHAR16*)NULL != HandleImageName) && (0x00 != StrStr(HandleImageName, DriverName))) {
      Status = gBS->UnloadImage(HandleBuffer[index]);
    }
  }

  return Status;
}

VOID
UnloadNetworkBootDrivers(
  VOID
)
{
  UnloadByName((CHAR16*)L"PXE Base Code");
  UnloadByName((CHAR16*)L"HTTP Boot");
}

/**
  Safe function to hide/show boot options
**/
VOID
EFIAPI
BoardBdsHideBootOptions (
  VOID
  )
{
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN BootOptionCount;
  UINTN Index;
  BOOLEAN EnableShellBoot = TRUE;
  BOOLEAN EnableNetworkBoot = TRUE;
  UINTN BoolVarSize;
  BOOLEAN Modified = FALSE;
  EFI_STATUS Status;

  // Safe get variables
  BoolVarSize = sizeof(BOOLEAN);
  Status = gRT->GetVariable (
                  L"ShellBoot",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &BoolVarSize,
                  &EnableShellBoot
                  );
  if (EFI_ERROR(Status)) EnableShellBoot = TRUE;

  BoolVarSize = sizeof(BOOLEAN);
  Status = gRT->GetVariable (
                  L"NetworkBoot",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &BoolVarSize,
                  &EnableNetworkBoot
                  );
  if (EFI_ERROR(Status)) EnableNetworkBoot = TRUE;
  
  if (!EnableNetworkBoot) 
  {
	UnloadNetworkBootDrivers();
  }

  DEBUG ((
    DEBUG_INFO, 
    "BoardBdsHideBootOptions: Shell=%d Network=%d\n",
    EnableShellBoot,
    EnableNetworkBoot
    ));

  // Get all boot options
  BootOptions = EfiBootManagerGetLoadOptions (
                  &BootOptionCount, 
                  LoadOptionTypeBoot
                  );

  if (BootOptions == NULL || BootOptionCount == 0) {
    DEBUG ((DEBUG_INFO, "No boot options found\n"));
    return;
  }

  for (Index = 0; Index < BootOptionCount; Index++) {
    EFI_BOOT_MANAGER_LOAD_OPTION *BootOption = &BootOptions[Index];
    BOOLEAN IsShellOption = FALSE;
    BOOLEAN IsNetworkOption = FALSE;
    BOOLEAN CurrentActive;
    BOOLEAN ShouldBeActive;

    // Quick identification by description first
    if (BootOption->Description != NULL) {
      IsShellOption = IsShellDescription(BootOption->Description);
      IsNetworkOption = IsNetworkDescription(BootOption->Description);
    }

    // If still not identified, check device path
    if (!IsShellOption && !IsNetworkOption && BootOption->FilePath != NULL) {
      IsShellOption = IsShellDevicePath (BootOption->FilePath);
      IsNetworkOption = IsNetworkDevicePath (BootOption->FilePath);
    }

    // Determine desired state
    CurrentActive = (BootOption->Attributes & LOAD_OPTION_ACTIVE) != 0;
    
    if (IsShellOption) {
      ShouldBeActive = EnableShellBoot;
    } else if (IsNetworkOption) {
      ShouldBeActive = EnableNetworkBoot;
    } else {
      // For other options, keep current state
      continue;
    }

    // Update if needed
    if (CurrentActive != ShouldBeActive) {
      if (ShouldBeActive) {
        BootOption->Attributes |= LOAD_OPTION_ACTIVE;
        DEBUG ((
          DEBUG_INFO, 
          "Enabling: %s\n", 
          BootOption->Description ? BootOption->Description : L"Unknown"
        ));
      } else {
        BootOption->Attributes &= ~LOAD_OPTION_ACTIVE;
        DEBUG ((
          DEBUG_INFO, 
          "Disabling: %s\n", 
          BootOption->Description ? BootOption->Description : L"Unknown"
        ));
      }
      Modified = TRUE;

      // Update the boot option variable
      Status = EfiBootManagerAddLoadOptionVariable (
                 BootOption, 
                 BootOption->OptionNumber
                 );
      if (EFI_ERROR(Status)) {
        DEBUG ((DEBUG_WARN, "Failed to update boot option: %r\n", Status));
      }
    }
  }

  if (Modified) {
    DEBUG ((DEBUG_INFO, "Boot options attributes updated\n"));
  }

  EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
}

/**
  Main function to process boot options
**/
VOID
EFIAPI
BoardBdsProcessBootOptions (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "BoardBdsProcessBootOptions: Starting boot options processing\n"));
  
  // Update individual boot option attributes (ACTIVE flag)
  BoardBdsHideBootOptions ();
  
  // Update boot order - if no options remain, only Boot Manager will appear
  BoardBdsUpdateBootOrder ();
  
  DEBUG ((DEBUG_INFO, "BoardBdsProcessBootOptions: Completed\n"));
}

/**
  Signal EndOfDxe event and install SMM Ready to lock protocol.

**/
VOID
InstallReadyToLock (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_SMM_ACCESS2_PROTOCOL  *SmmAccess;

  DEBUG ((DEBUG_INFO, "InstallReadyToLock  entering......\n"));
  //
  // Inform the SMM infrastructure that we're entering BDS and may run 3rd party code hereafter
  // Since PI1.2.1, we need signal EndOfDxe as ExitPmAuth
  //
  EfiEventGroupSignal (&gEfiEndOfDxeEventGroupGuid);
  DEBUG ((DEBUG_INFO, "All EndOfDxe callbacks have returned successfully\n"));

  //
  // Install DxeSmmReadyToLock protocol in order to lock SMM
  //
  Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
  if (!EFI_ERROR (Status)) {
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gEfiDxeSmmReadyToLockProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
  }

  DEBUG ((DEBUG_INFO, "InstallReadyToLock  end\n"));
  return;
}

/**
  Return the index of the load option in the load option array.

  The function consider two load options are equal when the
  OptionType, Attributes, Description, FilePath and OptionalData are equal.

  @param Key    Pointer to the load option to be found.
  @param Array  Pointer to the array of load options to be found.
  @param Count  Number of entries in the Array.

  @retval -1          Key wasn't found in the Array.
  @retval 0 ~ Count-1 The index of the Key in the Array.
**/
INTN
PlatformFindLoadOption (
  IN CONST EFI_BOOT_MANAGER_LOAD_OPTION  *Key,
  IN CONST EFI_BOOT_MANAGER_LOAD_OPTION  *Array,
  IN UINTN                               Count
  )
{
  UINTN  Index;

  for (Index = 0; Index < Count; Index++) {
    if ((Key->OptionType == Array[Index].OptionType) &&
        (Key->Attributes == Array[Index].Attributes) &&
        (StrCmp (Key->Description, Array[Index].Description) == 0) &&
        (CompareMem (Key->FilePath, Array[Index].FilePath, GetDevicePathSize (Key->FilePath)) == 0) &&
        (Key->OptionalDataSize == Array[Index].OptionalDataSize) &&
        (CompareMem (Key->OptionalData, Array[Index].OptionalData, Key->OptionalDataSize) == 0))
    {
      return (INTN)Index;
    }
  }

  return -1;
}

/**
  Get the FV device path for the shell file.

  @return   A pointer to device path structure.
**/
EFI_DEVICE_PATH_PROTOCOL *
BdsGetShellFvDevicePath (
  VOID
  )
{
  UINTN                          FvHandleCount;
  EFI_HANDLE                     *FvHandleBuffer;
  UINTN                          Index;
  EFI_STATUS                     Status;
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *Fv;
  UINTN                          Size;
  UINT32                         AuthenticationStatus;
  EFI_DEVICE_PATH_PROTOCOL       *DevicePath;
  EFI_FV_FILETYPE                FoundType;
  EFI_FV_FILE_ATTRIBUTES         FileAttributes;

  Status = EFI_SUCCESS;
  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiFirmwareVolume2ProtocolGuid,
         NULL,
         &FvHandleCount,
         &FvHandleBuffer
         );

  for (Index = 0; Index < FvHandleCount; Index++) {
    Size = 0;
    gBS->HandleProtocol (
           FvHandleBuffer[Index],
           &gEfiFirmwareVolume2ProtocolGuid,
           (VOID **)&Fv
           );
    Status = Fv->ReadFile (
                   Fv,
                   &gUefiShellFileGuid,
                   NULL,
                   &Size,
                   &FoundType,
                   &FileAttributes,
                   &AuthenticationStatus
                   );
    if (!EFI_ERROR (Status)) {
      //
      // Found the shell file
      //
      break;
    }
  }

  if (EFI_ERROR (Status)) {
    if (FvHandleCount) {
      FreePool (FvHandleBuffer);
    }

    return NULL;
  }

  DevicePath = DevicePathFromHandle (FvHandleBuffer[Index]);

  if (FvHandleCount) {
    FreePool (FvHandleBuffer);
  }

  return DevicePath;
}

/**
  Register a boot option using a file GUID in the FV.

  @param FileGuid     The file GUID name in FV.
  @param Description  The boot option description.
  @param Attributes   The attributes used for the boot option loading.
**/
VOID
PlatformRegisterFvBootOption (
  EFI_GUID  *FileGuid,
  CHAR16    *Description,
  UINT32    Attributes
  )
{
  EFI_STATUS                         Status;
  UINTN                              OptionIndex;
  EFI_BOOT_MANAGER_LOAD_OPTION       NewOption;
  EFI_BOOT_MANAGER_LOAD_OPTION       *BootOptions;
  UINTN                              BootOptionCount;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  FileNode;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;

  EfiInitializeFwVolDevicepathNode (&FileNode, FileGuid);
  DevicePath = AppendDevicePathNode (
                 BdsGetShellFvDevicePath (),
                 (EFI_DEVICE_PATH_PROTOCOL *)&FileNode
                 );

  Status = EfiBootManagerInitializeLoadOption (
             &NewOption,
             LoadOptionNumberUnassigned,
             LoadOptionTypeBoot,
             Attributes,
             Description,
             DevicePath,
             NULL,
             0
             );
  if (!EFI_ERROR (Status)) {
    BootOptions = EfiBootManagerGetLoadOptions (&BootOptionCount, LoadOptionTypeBoot);

    OptionIndex = PlatformFindLoadOption (&NewOption, BootOptions, BootOptionCount);

    if (OptionIndex == -1) {
      Status = EfiBootManagerAddLoadOptionVariable (&NewOption, (UINTN)-1);
      ASSERT_EFI_ERROR (Status);
    }

    EfiBootManagerFreeLoadOption (&NewOption);
    EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
  }
}

/**
  Do the platform specific action before the console is connected.

  Such as:
    Update console variable;
    Register new Driver#### or Boot####;
    Signal ReadyToLock event.
**/
VOID
EFIAPI
PlatformBootManagerBeforeConsole (
  VOID
  )
{
  EFI_INPUT_KEY                 Enter;
  EFI_INPUT_KEY                 CustomKey;
  EFI_INPUT_KEY                 Down;
  EFI_BOOT_MANAGER_LOAD_OPTION  BootOption;

  //
  // Register ENTER as CONTINUE key
  //
  Enter.ScanCode    = SCAN_NULL;
  Enter.UnicodeChar = CHAR_CARRIAGE_RETURN;
  EfiBootManagerRegisterContinueKeyOption (0, &Enter, NULL);

  if (FixedPcdGetBool (PcdBootManagerEscape)) {
    //
    // Map Esc to Boot Manager Menu
    //
    CustomKey.ScanCode    = SCAN_ESC;
    CustomKey.UnicodeChar = CHAR_NULL;
  } else {
    //
    // Map Delete to Boot Manager Menu
    //
    CustomKey.ScanCode    = SCAN_DELETE;
    CustomKey.UnicodeChar = CHAR_NULL;
  }

  EfiBootManagerGetBootManagerMenu (&BootOption);
  EfiBootManagerAddKeyOptionVariable (NULL, (UINT16)BootOption.OptionNumber, 0, &CustomKey, NULL);

  //
  // Also add Down key to Boot Manager Menu since some serial terminals don't support F2 key.
  //
  Down.ScanCode    = SCAN_DOWN;
  Down.UnicodeChar = CHAR_NULL;
  EfiBootManagerGetBootManagerMenu (&BootOption);
  EfiBootManagerAddKeyOptionVariable (NULL, (UINT16)BootOption.OptionNumber, 0, &Down, NULL);

  //
  // Install ready to lock.
  // This needs to be done before option rom dispatched.
  //
  InstallReadyToLock ();

  //
  // Dispatch deferred images after EndOfDxe event and ReadyToLock installation.
  //
  EfiBootManagerDispatchDeferredImages ();

  PlatformConsoleInit ();
}

/**
  Do the platform specific action after the console is connected.

  Such as:
    Dynamically switch output mode;
    Signal console ready platform customized event;
    Run diagnostics like memory testing;
    Connect certain devices;
    Dispatch additional option roms.
**/
VOID
EFIAPI
PlatformBootManagerAfterConsole (
  VOID
  )
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  Black;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  White;
  EDKII_PLATFORM_LOGO_PROTOCOL   *PlatformLogo;
  EFI_STATUS                     Status;

  Black.Blue = Black.Green = Black.Red = Black.Reserved = 0;
  White.Blue = White.Green = White.Red = White.Reserved = 0xFF;

  Status = gBS->LocateProtocol (&gEdkiiPlatformLogoProtocolGuid, NULL, (VOID **)&PlatformLogo);

  if (!EFI_ERROR (Status)) {
    gST->ConOut->ClearScreen (gST->ConOut);
    BootLogoEnableLogo ();
  }

  EfiBootManagerConnectAll ();
  EfiBootManagerRefreshAllBootOption ();

  //
  // Register UEFI Shell
  //
  PlatformRegisterFvBootOption (&gUefiShellFileGuid, L"UEFI Shell", LOAD_OPTION_ACTIVE);
  
  BoardBdsProcessBootOptions ();

  if (FixedPcdGetBool (PcdBootManagerEscape)) {
    Print (
      L"\n"
      L"    Esc or Down      to enter Boot Manager Menu.\n"
      L"    ENTER            to boot directly.\n"
      L"\n"
      );
  } else {
    Print (
      L"Press DELETE to enter OYBoot Setup.\n"
      );
  }
}

/**
  This function is called each second during the boot manager waits the timeout.

  @param TimeoutRemain  The remaining timeout.
**/
VOID
EFIAPI
PlatformBootManagerWaitCallback (
  UINT16  TimeoutRemain
  )
{
  return;
}

/**
  The function is called when no boot option could be launched,
  including platform recovery options and options pointing to applications
  built into firmware volumes.

  If this function returns, BDS attempts to enter an infinite loop.
**/
VOID
EFIAPI
PlatformBootManagerUnableToBoot (
  VOID
  )
{
  return;
}
