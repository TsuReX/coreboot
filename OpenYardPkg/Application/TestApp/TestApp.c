#include "TestApp.h"


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE* SystemTable
)
{
  EFI_STATUS Status = EFI_SUCCESS;

  IPMI_TRANSPORT* IpmiTransportProtocol;

  Status = gBS->LocateProtocol(&gIpmiTransportProtocolGuid, NULL, (VOID**)&IpmiTransportProtocol);
  if (EFI_ERROR(Status)) {
    Print((CHAR16*)L"IpmiTransportProtocol is not found! %r\n", Status);
    return Status;
  }

  Print((CHAR16*)L"IpmiTransportProtocol is found!\n");

  Print((CHAR16*)L"Hello from TestApp.efi\n");
  DEBUG((DEBUG_INFO, "Hello from TestApp.efi\n"));

  return Status;
}
