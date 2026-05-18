/** @file
  TODO: Brief Description of UEFI Libray IpmiTransportLibNull

  TODO: Detailed Description of UEFI Libray IpmiTransportLibNull

  TODO: Copyright for UEFI Libray IpmiTransportLibNull

  TODO: License for UEFI Libray IpmiTransportLibNull

**/

#include <Library/IpmiTransportLib.h>

#define AMI_BMC_LUN                                         0x00

EFI_STATUS
EFIAPI
IpmiSendCommand(
  UINT8  Command,
  UINT8  *CommandData,
  UINT8  CommandDataSize,
  UINT8  *ResponseData,
  UINT8  *ResponseDataSize
)
{
  EFI_STATUS Status = EFI_SUCCESS;

  *ResponseData = 0xFF;
  *ResponseDataSize = 0x01;

  /*

  UINT8 NetFunction = 0x2e;
  UINT8 Lun = AMI_BMC_LUN;
  UINT8 TempResponseData[0xFF];
  UINT8 TempResponseDataSize = sizeof(TempResponseData);

  EFI_IPMI_TRANSPORT* IpmiTransportProtocol = NULL;

  Status = gBS->LocateProtocol(&gEfiDxeIpmiTransportProtocolGuid, NULL, (VOID**)&IpmiTransportProtocol);

  DEBUG((DEBUG_INFO, "LocateProtocol: DxeIpmiTransportProtocol, Status = %r\n", Status));

  if (EFI_ERROR(Status)) {
    return Status;
  }

  DEBUG((DEBUG_INFO, "Ipmi request: netfn = %02X, command = %02X, data = ", NetFunction, Command));
  for (UINTN Index = 0x00; Index < CommandDataSize; Index++)
  {
    DEBUG((DEBUG_INFO, "%02X ", CommandData[Index]));
  }
  DEBUG((DEBUG_INFO, "\n"));

  ZeroMem(TempResponseData, sizeof(TempResponseData));

  Status = IpmiTransportProtocol->SendIpmiCommand(
    IpmiTransportProtocol,
    NetFunction,
    Lun,
    Command,
    CommandData,
    CommandDataSize,
    TempResponseData,
    &TempResponseDataSize
  );

  DEBUG((DEBUG_INFO, "IpmiTransportProtocol: SendIpmiCommand, Status = %r\n", Status));

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  DEBUG((DEBUG_INFO, "Ipmi response: data = "));
  for (UINTN Index = 0x00; Index < TempResponseDataSize; Index++)
  {
    DEBUG((DEBUG_INFO, "%02X ", TempResponseData[Index]));
  }
  DEBUG((DEBUG_INFO, "\n"));

  if (NULL != ResponseData)
  {
    CopyMem(ResponseData, TempResponseData, *ResponseDataSize);
    *ResponseDataSize = TempResponseDataSize;
  }

  */

  return Status;
}
