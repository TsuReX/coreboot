/** @file
  The driver entry point for CpuSetupDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "CpuSetupImpl.h"


STATIC CPU_SETUP_CONFIG_PRIVATE_DATA* mConfigPrivate = NULL;


/**
  The entry point for CpuSetupDxe driver.

  @param[in] ImageHandle     The image handle of the driver.
  @param[in] SystemTable     The system table.

  @retval EFI_ALREADY_STARTED     The driver already exists in system.
  @retval EFI_OUT_OF_RESOURCES    Fail to execute entry point due to lack of
                                  resources.
  @retval EFI_SUCCES              All the related protocols are installed on
                                  the driver.

**/
EFI_STATUS
EFIAPI
CpuSetupDxeEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  OY_SETUP_CONFIGURATION OYSetupConfig;
  EFI_GUID SetupVarGuid = OY_SETUP_VARIABLE_GUID;
  UINTN BufferSize = sizeof(OY_SETUP_CONFIGURATION);

  ZeroMem(&OYSetupConfig, BufferSize);

  for (UINT8 Socket = 0x00; Socket < MAX_SOCKET; Socket++)
  {
    OYSetupConfig.ConfigIOU0[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU1[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU2[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU3[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU4[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU5[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU6[Socket] = IIO_BIFURCATE_AUTO;
    OYSetupConfig.ConfigIOU7[Socket] = IIO_BIFURCATE_AUTO;
  }


  for (UINT8 SataPort = 0x00; SataPort < PCH_MAX_SATA_PORTS; SataPort++)
  {
    OYSetupConfig.SataPortController0[SataPort] = 0x01;
    OYSetupConfig.SataPortController1[SataPort] = 0x01;
    OYSetupConfig.SataPortController2[SataPort] = 0x01;
  }

  for (UINT8 UsbPort = 0x00; UsbPort < SETUP_MAX_USB2_PORTS; UsbPort++)
  {
    OYSetupConfig.PchUsbHsPort[UsbPort] = 0x01;
  }

  for (UINT8 UsbPort = 0x00; UsbPort < SETUP_MAX_USB3_PORTS; UsbPort++)
  {
    OYSetupConfig.PchUsbSsPort[UsbPort] = 0x01;
  }

  OYSetupConfig.StateAfterG3 = 0xFE; // Last State

  // IF driver is not part of the Platform then need to get/set defaults for the NVRAM configuration that the driver will use.
  Status = gRT->GetVariable(
    OY_SETUP_VARIABLE_NAME,
    &SetupVarGuid,
    NULL,
    &BufferSize,
    (VOID*)NULL
  );
  if (EFI_ERROR(Status)) {  // Not definded yet so add it to the NV Variables.
    Status = gRT->SetVariable(
      OY_SETUP_VARIABLE_NAME,
      &SetupVarGuid,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
      sizeof(OY_SETUP_CONFIGURATION),
      &OYSetupConfig
    );
  }

  Status = InstallCpuSetupConfigForm (mConfigPrivate);
  
  return Status;
}
