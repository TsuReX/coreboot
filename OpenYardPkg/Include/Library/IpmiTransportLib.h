/** @file
  TODO: Brief Description of Library Class IpmiTransportLib
  
  TODO: Detailed Description of Library Class IpmiTransportLib

  TODO: Copyright for Library Class IpmiTransportLib
  
  TODO: License for Library Class IpmiTransportLib
  
**/

#ifndef __EFI_IPMI_TRANSPORT_LIB_H__
#define __EFI_IPMI_TRANSPORT_LIB_H__

#include <Uefi.h>

//
// Libraries
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>

//
// Consumed Protocols
//
//#include <Protocol/IpmiTransportProtocol.h>

///
/// Library class public defines
///

///
/// Library class public structures/unions
///

///
/// Library class public functions
///
EFI_STATUS
EFIAPI
IpmiSendCommand(
  UINT8  Command,
  UINT8* CommandData,
  UINT8  CommandDataSize,
  UINT8* ResponseData,
  UINT8* ResponseDataSize
);

#endif
