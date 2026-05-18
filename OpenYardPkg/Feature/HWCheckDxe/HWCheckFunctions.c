/** @file
  TODO: Brief Description of UEFI Driver HWCheckFunctions

  TODO: Detailed Description of UEFI Driver HWCheckFunctions

  TODO: Copyright for UEFI Driver HWCheckFunctions

  TODO: License for UEFI Driver HWCheckFunctions

**/

#include "HWCheckImpl.h"

EFI_GUID   mHWInfoVarGuid = HWDATA_VAR_GUID;
CHAR16     mHWInfoVarName[] = HWDATA_VAR_NAME;


CHAR8*
GetSmbiosString(
  IN  SMBIOS_STRUCTURE_POINTER* Smbios,
  IN  UINT16                    StringNumber
)
{
  UINT16  Index;
  CHAR8* String;

  ASSERT(Smbios != NULL);

  //
  // Skip over formatted section
  //
  String = (CHAR8*)(Smbios->Raw + Smbios->Hdr->Length);

  //
  // Look through unformated section
  //
  for (Index = 1; Index <= StringNumber; Index++) {
    if (StringNumber == Index) {
      return String;
    }

    //
    // Skip string
    //
    for (; *String != 0; String++) {
    }

    String++;

    if (*String == 0) {
      //
      // If double NULL then we are done.
      //  Return pointer to next structure in Smbios.
      //  if you pass in a -1 you will always get here
      //
      //Smbios->Raw = (UINT8*)++String;
      return NULL;
    }
  }

  return NULL;
}


UINT32
EFIAPI
HWCheckGetCpuInfoCrc32()
{
  EFI_STATUS Status = EFI_NOT_FOUND;

  EFI_SMBIOS_PROTOCOL* SmbiosProtocol;
  EFI_SMBIOS_HANDLE SmbiosHandle;
  SMBIOS_STRUCTURE_POINTER Smbios;

  EFI_SMBIOS_TYPE SmbiosType = EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION;

  UINT32 CalculatedCrc32 = 0x00;
  HWCHECK_CPU_INFO* CpuStructure = NULL;
  UINTN CpuStructureCount = 0;

  Status = gBS->LocateProtocol(
    &gEfiSmbiosProtocolGuid,
    NULL,
    (VOID**)&SmbiosProtocol);
  if (EFI_ERROR(Status)) {
    return 0x00;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;

  do {
    Status = SmbiosProtocol->GetNext(SmbiosProtocol, &SmbiosHandle, &SmbiosType, (EFI_SMBIOS_TABLE_HEADER**)&Smbios, NULL);
    if (Smbios.Hdr->Length != 0x00) {
      CpuStructure = (HWCHECK_CPU_INFO*)ReallocatePool(
        CpuStructureCount * sizeof(HWCHECK_CPU_INFO),
        (CpuStructureCount + 1) * sizeof(HWCHECK_CPU_INFO),
        CpuStructure
      );

      if (NULL == CpuStructure)
      {
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }

      AsciiStrCpyS(
        CpuStructure[CpuStructureCount].SocketDesignation,
        SMBIOS_STRING_MAX_LENGTH,
        GetSmbiosString(&Smbios, (UINT16) 1)
      ); // First string is location

      CpuStructure[CpuStructureCount].ProcessorId = Smbios.Type4->ProcessorId;

      CpuStructureCount++;
    }
  } while (!EFI_ERROR(Status));

  if (NULL == CpuStructure)
  {
    return 0x00;
  }

  gBS->CalculateCrc32(CpuStructure, CpuStructureCount * sizeof(HWCHECK_CPU_INFO), &CalculatedCrc32);

  FreePool(CpuStructure);

  return CalculatedCrc32;
}

UINT32
EFIAPI
HWCheckGetDimmInfoCrc32()
{
  EFI_STATUS Status = EFI_NOT_FOUND;

  EFI_SMBIOS_PROTOCOL* SmbiosProtocol;
  EFI_SMBIOS_HANDLE SmbiosHandle;
  SMBIOS_STRUCTURE_POINTER Smbios;

  EFI_SMBIOS_TYPE SmbiosType = EFI_SMBIOS_TYPE_MEMORY_DEVICE;

  UINT32 CalculatedCrc32 = 0x00;
  HWCHECK_DIMM_INFO* DimmStructure = NULL;
  UINTN DimmStructureCount = 0;

  Status = gBS->LocateProtocol(
    &gEfiSmbiosProtocolGuid,
    NULL,
    (VOID**)&SmbiosProtocol);
  if (EFI_ERROR(Status)) {
    return 0x00;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;

  do {
    Status = SmbiosProtocol->GetNext(SmbiosProtocol, &SmbiosHandle, &SmbiosType, (EFI_SMBIOS_TABLE_HEADER**)&Smbios, NULL);
    if (Smbios.Hdr->Length != 0x00) {
      DimmStructure = (HWCHECK_DIMM_INFO*)ReallocatePool(
        DimmStructureCount * sizeof(HWCHECK_DIMM_INFO),
        (DimmStructureCount + 1) * sizeof(HWCHECK_DIMM_INFO),
        DimmStructure
      );

      if (NULL == DimmStructure)
      {
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }

      //if ((0x00 == Smbios.Type17->TotalWidth) || (0x00 == Smbios.Type17->DataWidth))
      //{
      //  continue;
      //}

      /**
      AsciiStrCpyS(
        DimmStructure[DimmStructureCount].DeviceLocator,
        SMBIOS_STRING_MAX_LENGTH,
        GetSmbiosString(&Smbios, (UINT16) 1)
      ); // First string is location
      **/

      AsciiStrCpyS(
        DimmStructure[DimmStructureCount].Manufacturer,
        SMBIOS_STRING_MAX_LENGTH,
        GetSmbiosString(&Smbios, (UINT16) 3)
      ); // Third string is manufacturer

      AsciiStrCpyS(
        DimmStructure[DimmStructureCount].SerialNumber,
        SMBIOS_STRING_MAX_LENGTH,
        GetSmbiosString(&Smbios, (UINT16) 4)
      ); // Fourth string is serial number

      //DimmStructure[DimmStructureCount].Size = Smbios.Type17->Size;

      DimmStructureCount++;
    }
  } while (!EFI_ERROR(Status));

  if (NULL == DimmStructure)
  {
    return 0x00;
  }

  gBS->CalculateCrc32(DimmStructure, DimmStructureCount * sizeof(HWCHECK_DIMM_INFO), &CalculatedCrc32);

  FreePool(DimmStructure);

  return CalculatedCrc32;
}


UINT32
EFIAPI
HWCheckGetDisksInfoCrc32()
{
  EFI_HANDLE* HandleBuffer;
  EFI_DISK_INFO_PROTOCOL* DiskInfo;
  HWCHECK_DISK_INFO* DiskStructure;
  EFI_STATUS             Status;
  UINTN                  NumberOfDiskHandles = 0;
  UINT8                  Index;
  UINT32                 IdeChannel;
  UINT32                 IdeDevice;
  UINT32                 IdentifyDataSize;
  UINT16                 DisksCount = 0;
  VOID* IdentifyData = NULL;
  
  EFI_BLOCK_IO_PROTOCOL *BlockIoProtocol;

  UINT32 CalculatedCrc32 = 0x00;

  Status = gBS->LocateHandleBuffer(ByProtocol,
    &gEfiDiskInfoProtocolGuid,
    NULL,
    &NumberOfDiskHandles,
    &HandleBuffer);
  if (EFI_ERROR(Status))
  {
    return CalculatedCrc32;
  }

  Status = gBS->AllocatePool(
    EfiBootServicesData,
    NumberOfDiskHandles * sizeof(HWCHECK_DISK_INFO),
    (VOID**)&DiskStructure
  );
  if (EFI_ERROR(Status)) {
    gBS->FreePool(HandleBuffer);
    return CalculatedCrc32;
  }

  gBS->SetMem(DiskStructure, NumberOfDiskHandles * sizeof(HWCHECK_DISK_INFO), 0x00);

  for (Index = 0; Index < NumberOfDiskHandles; Index++)
  {
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiDiskInfoProtocolGuid, (VOID**)&DiskInfo);
    if (EFI_ERROR(Status)) continue;
    
    Status = gBS->OpenProtocol(
      HandleBuffer[Index],
      &gEfiBlockIoProtocolGuid,
      (VOID**)&BlockIoProtocol,
      NULL,
      HandleBuffer[Index],
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    if (!EFI_ERROR(Status))
    {
      if (TRUE == BlockIoProtocol->Media->RemovableMedia)
      {
        continue;
      }
    }

    ZeroMem(&DiskStructure[DisksCount], sizeof(HWCHECK_DISK_INFO));
    
    if (CompareGuid(&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid)) {
      Status = DiskInfo->WhichIde(DiskInfo, &IdeChannel, &IdeDevice);
      if (!EFI_ERROR(Status)) {
        DiskStructure[DisksCount].PortNumber = (UINT8) IdeChannel;
      }

      IdentifyDataSize = sizeof(IDENTIFY_DATA);
      gBS->AllocatePool(EfiBootServicesData,
        IdentifyDataSize,
        (VOID**)&IdentifyData);

      Status = DiskInfo->Identify(DiskInfo, IdentifyData, &IdentifyDataSize);
      if (EFI_ERROR(Status)) continue;

      gBS->CopyMem(&DiskStructure[DisksCount].IdentifyData, &((IDENTIFY_DATA*)IdentifyData)->Serial_Number_10, ATA_SERIAL_NUMBER_LENGTH);

      if (IdentifyData) {
        gBS->FreePool(IdentifyData);
        IdentifyData = NULL;
      }
    }
    else if (CompareGuid(&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid)) {
      Status = DiskInfo->WhichIde(DiskInfo, &IdeChannel, &IdeDevice);
      if (!EFI_ERROR(Status)) {
        DiskStructure[DisksCount].PortNumber = (UINT8) IdeChannel;
      }

      IdentifyDataSize = sizeof(NVME_IDENTIFY_NAMESPACE_DATA);
      gBS->AllocatePool(EfiBootServicesData,
        IdentifyDataSize,
        (VOID**)&IdentifyData);

      gBS->SetMem(IdentifyData, IdentifyDataSize, 0);

      Status = DiskInfo->Identify(DiskInfo, IdentifyData, &IdentifyDataSize);
      if (EFI_ERROR(Status)) continue;

      gBS->CopyMem(
        &DiskStructure[DisksCount].IdentifyData[0],
        &((NVME_IDENTIFY_NAMESPACE_DATA*)IdentifyData)->Eui64,
        sizeof(UINT64)
      );

      if (IdentifyData) {
        gBS->FreePool(IdentifyData);
        IdentifyData = NULL;
      }
    }
    else if (CompareGuid(&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid)) {
      Status = DiskInfo->WhichIde(DiskInfo, &IdeChannel, &IdeDevice);
      if (!EFI_ERROR(Status)) {
        DiskStructure[DisksCount].PortNumber = (UINT8) IdeChannel;
      }
      IdentifyDataSize = sizeof(IDENTIFY_DATA);
      gBS->AllocatePool(
        EfiBootServicesData,
        IdentifyDataSize,
        (VOID**)&IdentifyData
      );

      gBS->SetMem(IdentifyData, sizeof(IDENTIFY_DATA), 0);

      Status = DiskInfo->Identify(DiskInfo, IdentifyData, &IdentifyDataSize);
      if (EFI_ERROR(Status)) {
        continue;
      }

      gBS->CopyMem(&DiskStructure[DisksCount].IdentifyData, &((IDENTIFY_DATA*)IdentifyData)->Serial_Number_10, ATA_SERIAL_NUMBER_LENGTH);

      if (IdentifyData) {
        gBS->FreePool(IdentifyData);
        IdentifyData = NULL;
      }
    }
    else
    {
      continue;
    }

    DisksCount++;
  }

  Status = gBS->CalculateCrc32(&DiskStructure[0].PortNumber, (UINT16)(DisksCount * sizeof(HWCHECK_DISK_INFO)), &CalculatedCrc32);

  gBS->FreePool(DiskStructure);
  gBS->FreePool(HandleBuffer);

  return CalculatedCrc32;
}


UINT32
EFIAPI
HWCheckGetPciInfoCrc32()
{
  EFI_STATUS          Status;
  EFI_HANDLE* HandleBuffer;
  HWCHECK_PCI_INFO* PciInfo = NULL;
  UINTN               NumberOfPciHandles = 0;
  EFI_PCI_IO_PROTOCOL* EfiPciIoProtocol = NULL;
  UINTN               SegmentNumber = 0;
  UINTN               BusNumber = 0;
  UINTN               DeviceNumber = 0;
  UINTN               FunctionNumber = 0;
  UINT32              VidDid = 0;
  UINT16              Index;
  UINT16              PciDevCount = 0;

  UINT32 CalculatedCrc32 = 0x00;

  // Locate handle buffer for PCI IO Protocol.
  Status = gBS->LocateHandleBuffer(ByProtocol,
    &gEfiPciIoProtocolGuid,
    NULL,
    &NumberOfPciHandles,
    &HandleBuffer);
  if (EFI_ERROR(Status)) {
    return CalculatedCrc32;
  }

  Status = gBS->AllocatePool(
    EfiBootServicesData,
    NumberOfPciHandles * sizeof(HWCHECK_PCI_INFO),
    (VOID**)&PciInfo
  );
  if (EFI_ERROR(Status)) {
    gBS->FreePool(HandleBuffer);
    return CalculatedCrc32;
  }

  gBS->SetMem(PciInfo, NumberOfPciHandles * sizeof(HWCHECK_PCI_INFO), 0x00);

  for (Index = 0; Index < NumberOfPciHandles; Index++) {
    Status = gBS->HandleProtocol(
      HandleBuffer[Index],
      &gEfiPciIoProtocolGuid,
      (VOID**)&EfiPciIoProtocol
    );
    if (EFI_ERROR(Status)) {
      gBS->FreePool(HandleBuffer);
      gBS->FreePool(PciInfo);
      return CalculatedCrc32;
    }

    Status = EfiPciIoProtocol->GetLocation(EfiPciIoProtocol, &SegmentNumber, &BusNumber, &DeviceNumber, &FunctionNumber);
    if (EFI_ERROR(Status)) {
      gBS->FreePool(HandleBuffer);
      gBS->FreePool(PciInfo);
      return CalculatedCrc32;
    }

    if (0x00 == BusNumber) {
      continue;
    }

    Status = EfiPciIoProtocol->Pci.Read(EfiPciIoProtocol, EfiPciIoWidthUint32, 0, 1, &VidDid);
    if (EFI_ERROR(Status)) {
      continue;
    }

    PciInfo[PciDevCount].BusNumber = (UINT8)BusNumber;
    PciInfo[PciDevCount].DeviceNumber = (UINT8)DeviceNumber;
    PciInfo[PciDevCount].FunctionNumber = (UINT8)FunctionNumber;
    PciInfo[PciDevCount].VendorId = (UINT16)(VidDid);
    PciInfo[PciDevCount].DeviceId = (UINT16)(VidDid >> 16);

    PciDevCount++;
  }

  Status = gBS->CalculateCrc32(&PciInfo[0].PciNumber, (UINT16)(PciDevCount * sizeof(HWCHECK_PCI_INFO)), &CalculatedCrc32);

  gBS->FreePool(HandleBuffer);
  gBS->FreePool(PciInfo);

  return CalculatedCrc32;
}


EFI_STATUS
EFIAPI
HWCheckGetHardwareInfo(
IN OUT HWCHECK_INFO* HardwareInfo
)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (NULL == HardwareInfo)
  {
    return EFI_INVALID_PARAMETER;
  }

  HardwareInfo->CpuInfoCrc32 = HWCheckGetCpuInfoCrc32();

  HardwareInfo->DimmInfoCrc32 = HWCheckGetDimmInfoCrc32();

  HardwareInfo->DisksInfoCrc32 = HWCheckGetDisksInfoCrc32();

  HardwareInfo->PciInfoCrc32 = HWCheckGetPciInfoCrc32();

  return Status;
}


EFI_STATUS
EFIAPI
HWCheckSaveHardwareInfo()
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  HWCHECK_INFO HardwareInfo;

  Status = HWCheckGetHardwareInfo(&HardwareInfo);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Status = gRT->SetVariable(
    mHWInfoVarName,
    &mHWInfoVarGuid,
    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
    sizeof(HWCHECK_INFO),
    (VOID*) &HardwareInfo
  );

  return Status;
}
