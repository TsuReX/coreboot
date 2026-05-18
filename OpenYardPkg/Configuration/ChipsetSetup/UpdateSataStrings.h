/** @file
  TODO: Brief Description of UEFI Driver ChipsetSetupSata

  TODO: Detailed Description of UEFI Driver ChipsetSetupSata

  TODO: Copyright for UEFI Driver ChipsetSetupSata

  TODO: License for UEFI Driver ChipsetSetupSata

**/

#ifndef __EFI_UPDATE_SATA_STRINGS_H__
#define __EFI_UPDATE_SATA_STRINGS_H__

#include "ChipsetSetupImpl.h"

#include <Library/IoLib.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/AtaPassThru.h>


VOID
EFIAPI
ChipsetSetupSataUpdateDisks(EFI_HII_HANDLE HiiHandle
);


#endif
