#include "Memory.h"

extern INFO_CALLBACK_DATA  gInfoPrivate;

STATIC
VOID
UpdateTokenValue(
  IN CHAR16* String,
  IN VOID* StartOpCodeHandle,
  EFI_HII_HANDLE StringsHiiHandle
)
{
  EFI_STRING_ID   TokenToUpdate;

  TokenToUpdate = 0;
  TokenToUpdate = HiiSetString(
    StringsHiiHandle,
    0,
    String,
    NULL
  );

  HiiCreateSubTitleOpCode(
    StartOpCodeHandle,
    TokenToUpdate,
    0,
    0,
    0
  );
}


VOID
GetDimms(
  VOID* StartOpCodeHandle
)
{
  UINT8                    StrIndex;
  CHAR16* NewStringVendor;
  CHAR16* NewStringSerial;
  CHAR16* NewStringPart;
  CHAR16* NewStringLocator;
  UINT32 Size;
  UINT16 Speed;
  EFI_STATUS               Status;
  EFI_SMBIOS_HANDLE        SmbiosHandle;
  EFI_SMBIOS_PROTOCOL* Smbios;
  SMBIOS_TABLE_TYPE17* Type17Record;
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

    if (Record->Type == SMBIOS_TYPE_MEMORY_DEVICE) {
      Type17Record = (SMBIOS_TABLE_TYPE17*)Record;
      Size = (UINT16)Type17Record->Size;
      if (Size < 0x7fff) {
        Size = (UINT16)Type17Record->Size; // in MiB
      }
      else {
        Size = Type17Record->ExtendedSize;
      }
      if (0x00 != Size)
      {
        StrIndex = Type17Record->DeviceLocator;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type17Record + Type17Record->Hdr.Length), StrIndex, &NewStringLocator);
        StrIndex = Type17Record->Manufacturer;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type17Record + Type17Record->Hdr.Length), StrIndex, &NewStringVendor);
        StrIndex = Type17Record->PartNumber;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type17Record + Type17Record->Hdr.Length), StrIndex, &NewStringPart);
        StrIndex = Type17Record->SerialNumber;
        _GetOptionalStringByIndex((CHAR8*)((UINT8*)Type17Record + Type17Record->Hdr.Length), StrIndex, &NewStringSerial);
        Speed = Type17Record->ConfiguredMemoryClockSpeed;
        CHAR16* NewString = CatSPrint(NULL, (CHAR16*)L"%s: %s %s %s %dGB %dMT/s", NewStringLocator, NewStringVendor, NewStringPart, NewStringSerial, Size / 1024, Speed);
        UpdateTokenValue(NewString, StartOpCodeHandle, gInfoPrivate.HiiHandle);
        FreePool(NewStringLocator);
        FreePool(NewStringVendor);
        FreePool(NewStringPart);
        FreePool(NewStringSerial);
        FreePool(NewString);
      }
    }

    Status = Smbios->GetNext(Smbios, &SmbiosHandle, NULL, &Record, NULL);
  }
}


VOID
DisplayMemTopology(
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
  StartLabel->Number = MEM_TOPOLOGY_START_LABEL;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL*)HiiCreateGuidOpCode(EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number = MEM_TOPOLOGY_END_LABEL;

  GetDimms(StartOpCodeHandle);

  //
  // Update Memory Topology info page form
  //
  HiiUpdateForm(
    StringsHiiHandle,         // HII handle
    &FormSetGuid,                    // Formset GUID
    FORM_MEMORY_INFO_ID,  // Form ID
    StartOpCodeHandle,               // Label for where to insert opcodes
    EndOpCodeHandle                  // Replace data
  );
}
