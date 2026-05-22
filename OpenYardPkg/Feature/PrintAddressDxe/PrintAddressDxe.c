/** @file
  TODO: Brief Description of UEFI Driver PrintAddressDxe
  
  TODO: Detailed Description of UEFI Driver PrintAddressDxe

  TODO: Copyright for UEFI Driver PrintAddressDxe
  
  TODO: License for UEFI Driver PrintAddressDxe

**/

#include "PrintAddressDxe.h"


/**
  Unloads an image.

  @param  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
PrintAddressDxeUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
//  EFI_STATUS  Status;

//  Status = EFI_SUCCESS;
  //
  // Do any additional cleanup that is required for this driver
  //

  return EFI_SUCCESS;
}


/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers.

  @param  ImageHandle           The firmware allocated handle for the UEFI image.
  @param  SystemTable           A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/
EFI_STATUS
EFIAPI
PrintAddressDxeEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE* SystemTable
)
{
  EFI_STATUS Status;
  UINTN MaxColumns = 0x00;
  UINTN MaxRows = 0x00;

  IPv4_ADDRESS* BmcIp = PcdGetPtr(PcdBmcIpAddress);
  EFI_HANDLE* HandleBuffer = NULL;
  UINTN      HandleCount;

  CHAR16* OutputString = CatSPrint(NULL, (UINT16*)L"BMC IP: %d.%d.%d.%d\0", (UINT8)BmcIp->Addr[0], (UINT8)BmcIp->Addr[1], (UINT8)BmcIp->Addr[2], (UINT8)BmcIp->Addr[3]);

  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleTextOutProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (!EFI_ERROR(Status))
  {
    for (UINTN Index = 0x00; Index < HandleCount; Index++)
    {
      EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* TextOutProtocol;

      Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiSimpleTextOutProtocolGuid, (VOID**)&TextOutProtocol);
      if (EFI_ERROR(Status)) {
        continue;
      }

      TextOutProtocol->QueryMode(TextOutProtocol, TextOutProtocol->Mode->Mode, &MaxColumns, &MaxRows);

      UINTN CurPosX = TextOutProtocol->Mode->CursorColumn;
      UINTN CurPosY = TextOutProtocol->Mode->CursorRow;

      TextOutProtocol->SetCursorPosition(TextOutProtocol, 0, MaxRows - 2);
      TextOutProtocol->OutputString(TextOutProtocol, OutputString);
      TextOutProtocol->SetCursorPosition(TextOutProtocol, 0, MaxRows - 1);
      TextOutProtocol->OutputString(TextOutProtocol, L"Copyright (c) 2025 OpenYard");

      TextOutProtocol->SetCursorPosition(TextOutProtocol, CurPosX, CurPosY);
    }
  }

  if (OutputString)
  {
    FreePool(OutputString);
  }

  return Status;
}
