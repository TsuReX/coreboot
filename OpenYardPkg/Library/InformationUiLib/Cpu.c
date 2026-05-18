#include "Cpu.h"

extern INFO_CALLBACK_DATA  gInfoPrivate;

STATIC
VOID
UpdateTokenValue(
  IN CHAR16* String1,
  IN CHAR16* String2,
  IN VOID* StartOpCodeHandle,
  EFI_HII_HANDLE StringsHiiHandle
)
{
  EFI_STRING_ID   StringToAdd1 = 0x00;
  EFI_STRING_ID   StringToAdd2 = 0x00;

  StringToAdd1 = HiiSetString(
    StringsHiiHandle,
    0,
    String1,
    NULL
  );

  StringToAdd2 = HiiSetString(
    StringsHiiHandle,
    0,
    String2,
    NULL
  );

  HiiCreateTextOpCode(
    StartOpCodeHandle,
    StringToAdd1,
    0,
    StringToAdd2
  );
}


VOID
GetCpus(
  VOID* StartOpCodeHandle
)
{
  UINT8                    StrIndex;
  CHAR16* NewStringSocket;
  CHAR16* NewStringProcessorVersion;
  CHAR16* NewStringCurrentSpeed;
  EFI_STATUS               Status;
  EFI_SMBIOS_HANDLE        SmbiosHandle;
  EFI_SMBIOS_PROTOCOL* Smbios;
  SMBIOS_TABLE_TYPE4* Type4Record;
  EFI_SMBIOS_TABLE_HEADER* Record;

  //
  // Update Front Page banner strings base on SmBios Table.
  //
  Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);
  if (EFI_ERROR(Status)) {
    return;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->GetNext(Smbios, &SmbiosHandle, NULL, &Record, NULL);
  while (!EFI_ERROR(Status)) {

    if (Record->Type == SMBIOS_TYPE_PROCESSOR_INFORMATION)
    {
      Type4Record = (SMBIOS_TABLE_TYPE4*)Record;
      if ((Type4Record->Status & SMBIOS_TYPE4_CPU_SOCKET_POPULATED) == SMBIOS_TYPE4_CPU_SOCKET_POPULATED)
      {
        StrIndex = Type4Record->Socket;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type4Record + Type4Record->Hdr.Length), StrIndex, &NewStringSocket);
        StrIndex = Type4Record->ProcessorVersion;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type4Record + Type4Record->Hdr.Length), StrIndex, &NewStringProcessorVersion);

        _ConvertProcessorToString(Type4Record->CurrentSpeed, 6, &NewStringCurrentSpeed);

        CHAR16* NewStringSocketModel = CatSPrint(NULL, (CHAR16*)L"%s %s", NewStringSocket, (CHAR16*)L"Model");
        CHAR16* NewStringSocketSpeed = CatSPrint(NULL, (CHAR16*)L"%s %s", NewStringSocket, (CHAR16*)L"Speed");

        UpdateTokenValue(NewStringSocketModel, NewStringProcessorVersion, StartOpCodeHandle, gInfoPrivate.HiiHandle);
        UpdateTokenValue(NewStringSocketSpeed, NewStringCurrentSpeed, StartOpCodeHandle, gInfoPrivate.HiiHandle);

        // -----------

        FreePool(NewStringSocket);
        FreePool(NewStringProcessorVersion);
        FreePool(NewStringCurrentSpeed);
        FreePool(NewStringSocketModel);
        FreePool(NewStringSocketSpeed);
      }
    }

    Status = Smbios->GetNext(Smbios, &SmbiosHandle, NULL, &Record, NULL);
  }
}


VOID
DisplayCpus(
  EFI_HII_HANDLE StringsHiiHandle
)
{
  EFI_IFR_GUID_LABEL* StartLabel;
  EFI_IFR_GUID_LABEL* EndLabel;
  VOID* StartOpCodeHandle;
  VOID* EndOpCodeHandle;
  EFI_GUID                  FormSetGuid = INFO_FORMSET_GUID;

  //
  // Initialize the container for dynamic opcodes
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle();
  ASSERT(StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle();
  ASSERT(EndOpCodeHandle != NULL);

  //
  // If we failed to allocate handle, skip creating the opcodes
  //
  if ((StartOpCodeHandle == NULL) || (EndOpCodeHandle == NULL)) {
    if (StartOpCodeHandle != NULL) {
      HiiFreeOpCodeHandle(StartOpCodeHandle);
    }
    if (EndOpCodeHandle != NULL) {
      HiiFreeOpCodeHandle(EndOpCodeHandle);
    }
    return;
  }

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL*)HiiCreateGuidOpCode(StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number = CPU_START_LABEL;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL*)HiiCreateGuidOpCode(EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number = CPU_END_LABEL;

  GetCpus(StartOpCodeHandle);

  //
  // Update Memory Topology info page form
  //
  HiiUpdateForm(
    StringsHiiHandle,         // HII handle
    &FormSetGuid,                    // Formset GUID
    FORM_INFO_INFO_ID,  // Form ID
    StartOpCodeHandle,               // Label for where to insert opcodes
    EndOpCodeHandle                  // Replace data
  );
}
