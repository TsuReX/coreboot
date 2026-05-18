#include "UpdateSataStrings.h"

#define NEWSTRING_SIZE 0x200

#define PCH_MAX_SATA_CONTROLLERS 3

#define SATA_0_CONTROLLER_INDEX             0
#define SATA_1_CONTROLLER_INDEX             1
#define SATA_2_CONTROLLER_INDEX             2

#define PCI_DEVICE_NUMBER_PCH_SATA_0                  23
#define PCI_FUNCTION_NUMBER_PCH_SATA_0                0
#define PCI_DEVICE_NUMBER_PCH_SATA_1                  24
#define PCI_FUNCTION_NUMBER_PCH_SATA_1               0
#define PCI_DEVICE_NUMBER_PCH_SATA_2                  25
#define PCI_FUNCTION_NUMBER_PCH_SATA_2                0

/**
  Get Maximum available Sata Controller Number

  @param[in] None

  @retval Maximum Sata Controller Number
**/
UINT8
MaxSataControllerNum(
  VOID
)
{
  return PCH_MAX_SATA_CONTROLLERS;
}

/**
  Get SATA controller PCIe Device Number

  @param[in]  SataCtrlIndex       SATA controller index

  @retval SATA controller PCIe Device Number
**/
UINT8
SataDevNumber(
  IN UINT32  SataCtrlIndex
)
{
  ASSERT(SataCtrlIndex < MaxSataControllerNum());

  if (SataCtrlIndex == SATA_0_CONTROLLER_INDEX) {
    return PCI_DEVICE_NUMBER_PCH_SATA_0;
  }
  else if (SataCtrlIndex == SATA_1_CONTROLLER_INDEX) {
    return PCI_DEVICE_NUMBER_PCH_SATA_1;
  }
  else if (SataCtrlIndex == SATA_2_CONTROLLER_INDEX) {
    return PCI_DEVICE_NUMBER_PCH_SATA_2;
  }
  else {
    ASSERT(FALSE);
    return 0;
  }
}

/**
  Get SATA controller PCIe Function Number

  @param[in]  SataCtrlIndex       SATA controller index

  @retval SATA controller PCIe Function Number
**/
UINT8
SataFuncNumber(
  IN UINT32  SataCtrlIndex
)
{
  ASSERT(SataCtrlIndex < MaxSataControllerNum());

  if (SataCtrlIndex == SATA_0_CONTROLLER_INDEX) {
    return PCI_FUNCTION_NUMBER_PCH_SATA_0;
  }
  else if (SataCtrlIndex == SATA_1_CONTROLLER_INDEX) {
    return PCI_FUNCTION_NUMBER_PCH_SATA_1;
  }
  else if (SataCtrlIndex == SATA_2_CONTROLLER_INDEX) {
    return PCI_FUNCTION_NUMBER_PCH_SATA_2;
  }
  else {
    ASSERT(FALSE);
    return 0;
  }
}

/**

  Swap order of nearby CHAR8 data.

  @param Data  -  The data to swap.
  @param Size  -  Size of data to swap.

  @retval None.

**/
VOID
SwapEntries(
  IN  CHAR8* Data,
  IN  UINT16 Size
)
{
  UINT16  Index;
  CHAR8   Temp8;

  for (Index = 0; (Index + 1) < Size; Index += 2) {
    Temp8 = Data[Index];
    Data[Index] = Data[Index + 1];
    Data[Index + 1] = Temp8;
  }

  return;
}


/**

  Converts an ascii string to unicode string 16 chars at a time.

  @param AsciiString    -  Address of Ascii string.
  @param UnicodeString  -  Address of Unicode String.

  @retval None.

**/
VOID
AsciiToUnicode(
  IN CHAR8* AsciiString,
  IN CHAR16* UnicodeString
)
{
  UINT8 Index;

  Index = 0;
  while (AsciiString[Index] != 0) {
    UnicodeString[Index] = (CHAR16)AsciiString[Index];
    Index++;
  }
  UnicodeString[Index] = 0;
}


/**

  Swap order of nearby CHAR8 data.

  @param Data  -  The data to swap.
  @param Size  -  Size of data to swap.

  @retval None.

**/
EFI_STRING_ID
UpdateAhciString(
  IN  UINT8     SataCtrlIndex,
  IN  UINTN     UpdateIndex
)
{
  if (SataCtrlIndex == SATA_0_CONTROLLER_INDEX) {
    switch (UpdateIndex) {
    case 0:
      return STR_SATA_0_SETUP_PORT_0_DISK;
    case 1:
      return STR_SATA_0_SETUP_PORT_1_DISK;
    case 2:
      return STR_SATA_0_SETUP_PORT_2_DISK;
    case 3:
      return STR_SATA_0_SETUP_PORT_3_DISK;
    case 4:
      return STR_SATA_0_SETUP_PORT_4_DISK;
    case 5:
      return STR_SATA_0_SETUP_PORT_5_DISK;
    case 6:
      return STR_SATA_0_SETUP_PORT_6_DISK;
    case 7:
      return STR_SATA_0_SETUP_PORT_7_DISK;
    default:
      return 0;
    }
  }
  else if (SataCtrlIndex == SATA_1_CONTROLLER_INDEX) {
    switch (UpdateIndex) {
    case 0:
      return STR_SATA_1_SETUP_PORT_0_DISK;
    case 1:
      return STR_SATA_1_SETUP_PORT_1_DISK;
    case 2:
      return STR_SATA_1_SETUP_PORT_2_DISK;
    case 3:
      return STR_SATA_1_SETUP_PORT_3_DISK;
    case 4:
      return STR_SATA_1_SETUP_PORT_4_DISK;
    case 5:
      return STR_SATA_1_SETUP_PORT_5_DISK;
    case 6:
      return STR_SATA_1_SETUP_PORT_6_DISK;
    case 7:
      return STR_SATA_1_SETUP_PORT_7_DISK;
    default:
      return 0;
    }
  }
  else if (SataCtrlIndex == SATA_2_CONTROLLER_INDEX) {
    switch (UpdateIndex) {
    case 0:
      return STR_SATA_2_SETUP_PORT_0_DISK;
    case 1:
      return STR_SATA_2_SETUP_PORT_1_DISK;
    case 2:
      return STR_SATA_2_SETUP_PORT_2_DISK;
    case 3:
      return STR_SATA_2_SETUP_PORT_3_DISK;
    case 4:
      return STR_SATA_2_SETUP_PORT_4_DISK;
    case 5:
      return STR_SATA_2_SETUP_PORT_5_DISK;
    case 6:
      return STR_SATA_2_SETUP_PORT_6_DISK;
    case 7:
      return STR_SATA_2_SETUP_PORT_7_DISK;
    default:
      return 0;
    }
  }
  return 0;
}


/**

  IDE data filter function.

  @param None

  @retval None.

**/
VOID
EFIAPI
ChipsetSetupSataUpdateDisks(EFI_HII_HANDLE HiiHandle
)
{
  EFI_STATUS                      Status;
  UINTN                           HandleCount;
  EFI_HANDLE* HandleBuffer;
  EFI_DISK_INFO_PROTOCOL* DiskInfo;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath;
  EFI_DEVICE_PATH_PROTOCOL* DevicePathNode;
  PCI_DEVICE_PATH* PciDevicePath;
  UINTN                           Index;
  UINT8                           Index1;
  UINT32                          BufferSize;
  // Changing the variable datatype as UINT64 since UINT32 is not sufficient to store 4TB Hard Disk size.
// APTIOV_SERVER_OVERRIDE_RC_START : BIOS setup just show 801.7 GB for the HDD which is 4TB.
  UINT64                          DriveSize;
  // APTIOV_SERVER_OVERRIDE_RC_END : BIOS setup just show 801.7 GB for the HDD which is 4TB.      
  UINT32                          SectorSize;
  UINT32                          IdeChannel;
  UINT32                          IdeDevice;
  EFI_IDENTIFY_DATA* IdentifyDriveInfo;
  CHAR16* NewString;
  CHAR16                          SizeString[20];
  EFI_STRING_ID                   NameToUpdate;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo;
  UINT8                           PciBuffer;
  UINT8                           SataMode;
  UINT8                           SataCtrlIndex;

  //
  // Assume no line strings is longer than 256 bytes.
  //
  NewString = AllocateZeroPool(NEWSTRING_SIZE);
  PciDevicePath = NULL;

  if (NewString == NULL) {
    DEBUG((EFI_D_ERROR, "\nEFI_OUT_OF_RESOURCES!!! AllocateZeroPool() returned NULL pointer.\n"));
    ASSERT_EFI_ERROR(EFI_OUT_OF_RESOURCES);
    return;
  }

  NameToUpdate = (EFI_STRING_ID)STR_SATA_1_SETUP_PORT_0_DISK;
  //
  // Initialize IDE Combined mode
  //
  Status = gBS->LocateProtocol(&gEfiPciRootBridgeIoProtocolGuid, NULL, &PciRootBridgeIo);

  Status = PciRootBridgeIo->Pci.Read(
    PciRootBridgeIo,
    EfiPciWidthUint8,
    0x1F020A,
    1,
    &PciBuffer
  );
  SataMode = PciBuffer;

  //
  // Fill IDE Information for IDE mode
  //
  Status = gBS->LocateHandleBuffer(
    ByProtocol,
    &gEfiDiskInfoProtocolGuid,
    NULL,
    &HandleCount,
    &HandleBuffer
  );
  if (EFI_ERROR(Status)) {
    return;
  }

  for (Index = 0; Index < HandleCount; Index++) {

    Status = gBS->HandleProtocol(
      HandleBuffer[Index],
      &gEfiDevicePathProtocolGuid,
      (VOID*)&DevicePath
    );
    ASSERT_EFI_ERROR(Status);

    DevicePathNode = DevicePath;
    while (!IsDevicePathEnd(DevicePathNode)) {
      if ((DevicePathType(DevicePathNode) == HARDWARE_DEVICE_PATH) &&
        (DevicePathSubType(DevicePathNode) == HW_PCI_DP)
        ) {
        PciDevicePath = (PCI_DEVICE_PATH*)DevicePathNode;
        break;
      }

      DevicePathNode = NextDevicePathNode(DevicePathNode);
    }

    if (PciDevicePath == NULL) {
      continue;
    }
    //
    // Check for onboard SATA
    //
    for (SataCtrlIndex = 0; SataCtrlIndex < MaxSataControllerNum(); SataCtrlIndex++) {
      if ((SataDevNumber(SataCtrlIndex) == PciDevicePath->Device) && (SataFuncNumber(SataCtrlIndex) == PciDevicePath->Function)) {
        break;
      }
    }
    if (SataCtrlIndex < MaxSataControllerNum()) {
      Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiDiskInfoProtocolGuid, &DiskInfo);
      ASSERT_EFI_ERROR(Status);

      Status = DiskInfo->WhichIde(
        DiskInfo,
        &IdeChannel,
        &IdeDevice
      );
      // APTIOV_SERVER_OVERRIDE_RC_START : Fix for EIP #119521 (IdeCallback() in SetupInfoRecords.c causes ASSERT when SATA cdrom is 
      // connected and ScsiPassThruAtapi module is present in project.)
      //ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR(Status)) {
        continue;
      }
      // APTIOV_SERVER_OVERRIDE_RC_END : Fix for EIP #119521 (IdeCallback() in SetupInfoRecords.c causes ASSERT when SATA cdrom is 
      // connected and ScsiPassThruAtapi module is present in project.)      

      BufferSize = sizeof(EFI_IDENTIFY_DATA);
      IdentifyDriveInfo = AllocatePool(BufferSize);

      if (IdentifyDriveInfo == NULL) {
        DEBUG((EFI_D_ERROR, "\nEFI_OUT_OF_RESOURCES!!! AllocateZeroPool() returned NULL pointer.\n"));
        ASSERT_EFI_ERROR(EFI_OUT_OF_RESOURCES);
        return;
      }

      Status = DiskInfo->Identify(
        DiskInfo,
        IdentifyDriveInfo,
        &BufferSize
      );
      ASSERT_EFI_ERROR(Status);

      if (SataMode == PCI_CLASS_MASS_STORAGE_IDE) {   // IDE MODE
        Status = DiskInfo->WhichIde(
          DiskInfo,
          &IdeChannel,
          &IdeDevice
        );
        IdeDevice = (IdeChannel * 2) + IdeDevice;
        // Port 0 = Primary Master
        // Port 2 = Primary Slave
        // Port 1 = Secondary Master
        // Port 3 = Secondary Slave
        if (IdeDevice == 1 || IdeDevice == 2) {   // SINDX Serial ATA Index (D31:F2)
          IdeDevice ^= 3;
        }
        if (PciDevicePath->Function == 5) {  //Port 4, 5 in SATA2
          IdeDevice += 4;
        }
        NameToUpdate = UpdateAhciString(
          SataCtrlIndex,
          IdeDevice
        );

      }
      else { // SATA or RAID MODE
        Status = DiskInfo->WhichIde(
          DiskInfo,
          &IdeChannel,
          &IdeDevice
        );
        NameToUpdate = UpdateAhciString(
          SataCtrlIndex,
          IdeChannel
        );
      }

      SwapEntries((CHAR8*)&IdentifyDriveInfo->AtaData.ModelName, 40);
      AsciiToUnicode((CHAR8*)&IdentifyDriveInfo->AtaData.ModelName, NewString);

      // APTIOV_SERVER_OVERRIDE_RC_START : Changes to support 4K Sector Size.
      // Update sector size if necessary
      if ((IdentifyDriveInfo->AtaData.phy_logic_sector_support & 0x4000) && (!(IdentifyDriveInfo->AtaData.phy_logic_sector_support & 0x8000)) && (IdentifyDriveInfo->AtaData.phy_logic_sector_support & 0x1000)) {
        SectorSize = (UINT32)(IdentifyDriveInfo->AtaData.logic_sector_size_lo + (IdentifyDriveInfo->AtaData.logic_sector_size_hi << 16)) * 2;
      }
      else {
        SectorSize = 512;
      }
      // APTIOV_SERVER_OVERRIDE_RC_END : Changes to support 4K Sector Size.

      //
      // Chop it off after 14 characters
      //
      NewString[14] = 0;

      //
      // For HardDisk append the size. Otherwise display atapi
      //
      if ((IdentifyDriveInfo->AtaData.config & 0x8000) == 00) {
        //
        // 48 bit address feature set is supported, get maximum capacity
        //

        if ((IdentifyDriveInfo->AtaData.phy_logic_sector_support & BIT12) == (UINT16)BIT12) {
          SectorSize = 2 * ((UINT32)(IdentifyDriveInfo->AtaData.logic_sector_size_hi << 16) | (UINT32)IdentifyDriveInfo->AtaData.logic_sector_size_lo);
        }
        else {
          // logical sector size is 256 words
          SectorSize = 2 * 256;
        }

        if ((IdentifyDriveInfo->AtaData.command_set_supported_83 & 0x0400) == 0) {
          DriveSize =
            (
              (
                (
                  (
                    (IdentifyDriveInfo->AtaData.user_addressable_sectors_hi << 16) +
                    IdentifyDriveInfo->AtaData.user_addressable_sectors_lo
                    ) / 1000
                  ) * SectorSize
                ) /
              1000
              );
        }
        else {
          DriveSize = IdentifyDriveInfo->AtaData.maximum_lba_for_48bit_addressing[0];
          for (Index1 = 1; Index1 < 4; Index1++) {
            DriveSize |= LShiftU64(
              IdentifyDriveInfo->AtaData.maximum_lba_for_48bit_addressing[Index1],
              16 * Index1
            );
          }
          DriveSize = (UINT32)DivU64x64Remainder(MultU64x32(DivU64x64Remainder(DriveSize, 1000, 0), SectorSize), 1000, 0);
        }

        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), L" - ");
        UnicodeValueToStringS(SizeString, NEWSTRING_SIZE, LEFT_JUSTIFY, DriveSize / 1000, 5);
        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), SizeString);
        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), L".");
        UnicodeValueToStringS(SizeString, NEWSTRING_SIZE, LEFT_JUSTIFY, (DriveSize % 1000) / 100, 1);
        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), SizeString);
        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), L" GB");
      }
      else {
        StrCatS(NewString, NEWSTRING_SIZE / sizeof(CHAR16), L" - ATAPI");
      }

      HiiSetString(HiiHandle, NameToUpdate, NewString, NULL); //Update Sata Port # page with info
      // APTIOV_SERVER_OVERRIDE_RC_START
      //HiiSetString(mMainHiiHandle, NameToUpdate, NewString, NULL);    //Update Main page with info as well
      // APTIOV_SERVER_OVERRIDE_RC_END
      gBS->FreePool(IdentifyDriveInfo);
    }
  }

  if (HandleBuffer != NULL) {
    gBS->FreePool(HandleBuffer);
  }

  gBS->FreePool(NewString);

  return;
}
