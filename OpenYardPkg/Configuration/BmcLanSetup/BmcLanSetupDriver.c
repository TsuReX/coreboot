/** @file
  The driver entry point for BmcLanSetupDxe driver.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BmcLanSetupImpl.h"
#include "BmcLanConfigCommon.h"

STATIC BMC_LAN_SETUP_CONFIG_PRIVATE_DATA* mConfigPrivate = NULL;


EFI_IPMI_TRANSPORT* gIpmiTransport;
SERVER_MGMT_SETUP_CONFIGURATION      gServerMgmtConfiguration;


/** @internal
    Set the LAN configuration values from BIOS setup to BMC.

    @param[in] LanChannelNumber     LAN channel to use.

    @param[in] BmcLanIpSetupValues  Structure containing IP and MAC to be
                                    entered in BMC.

    @return EFI_STATUS  Status returned by function calls used.

**/
EFI_STATUS
SetStaticBmcNetworkParameters(
  IN UINT8                  LanChannelNumber,
  IN BMC_IP_BIOS_SETTINGS   BmcLanIpSetupValues
)
{
  EFI_STATUS                                              Status;
  AMI_SET_LAN_CONFIG_PARAM_IPV4_ADDRESS_REQUEST           IpAddressCommandData;
  AMI_SET_LAN_CONFIG_PARAM_IPV4_SUBNET_MASK_REQUEST       SubnetMaskCommandData;
  AMI_SET_LAN_CONFIG_PARAM_GATEWAY_MAC_ADDRESS_REQUEST    GatewayMacCommandData;
  UINT8                                                   CompletionCode;
  UINT8                                                   ResponseDataSize;

  IpAddressCommandData.LanChannel.ChannelNumber = LanChannelNumber;
  IpAddressCommandData.LanChannel.Reserved = 0;

  // Set station IP address.
  IpAddressCommandData.ParameterSelector = IpmiLanIpAddress;
  CopyMem(
    &IpAddressCommandData.Ipv4Address.Addr[0],
    BmcLanIpSetupValues.StationIp,
    sizeof(BmcLanIpSetupValues.StationIp));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a IpmiWaitSetInProgressClear() Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  ResponseDataSize = sizeof(CompletionCode);
  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&IpAddressCommandData,
    sizeof(IpAddressCommandData),
    &CompletionCode,
    &ResponseDataSize);

  DEBUG((DEBUG_INFO, " IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS: IpmiLanIpAddress: LanChannelNumber: %x Status: %r\n", LanChannelNumber, Status));

  if (EFI_ERROR(Status)) {
    return Status;
  }

  // Set Subnet Mask.
  SubnetMaskCommandData.LanChannel.ChannelNumber = LanChannelNumber;
  SubnetMaskCommandData.LanChannel.Reserved = 0;
  SubnetMaskCommandData.ParameterSelector = IpmiLanSubnetMask;
  CopyMem(
    &SubnetMaskCommandData.SubnetMask[0],
    BmcLanIpSetupValues.Subnet,
    sizeof(BmcLanIpSetupValues.Subnet));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a IpmiWaitSetInProgressClear() Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  CompletionCode = 0;
  ResponseDataSize = sizeof(CompletionCode);
  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&SubnetMaskCommandData,
    sizeof(SubnetMaskCommandData),
    &CompletionCode,
    &ResponseDataSize);

  DEBUG((DEBUG_INFO, " IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS: IpmiLanSubnetMask: LanChannelNumber: %x Status: %r\n", LanChannelNumber, Status));

  if (EFI_ERROR(Status)) {
    return Status;
  }

  IpAddressCommandData.LanChannel.ChannelNumber = LanChannelNumber;
  IpAddressCommandData.LanChannel.Reserved = 0;
  // Set Default Gateway Ip Address.
  IpAddressCommandData.ParameterSelector = IpmiLanDefaultGateway;
  CopyMem(
    &IpAddressCommandData.Ipv4Address.Addr[0],
    BmcLanIpSetupValues.RouterIp,
    sizeof(BmcLanIpSetupValues.RouterIp));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a IpmiWaitSetInProgressClear() Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  CompletionCode = 0;
  ResponseDataSize = sizeof(CompletionCode);
  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&IpAddressCommandData,
    sizeof(IpAddressCommandData),
    &CompletionCode,
    &ResponseDataSize);

  DEBUG((DEBUG_INFO, " IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS: IpmiLanDefaultGateway: LanChannelNumber: %x Status: %r CompletionCode: %X\n", LanChannelNumber, Status, CompletionCode));

  if (EFI_ERROR(Status)) {
    return Status;
  }

  // Set Default Gateway MAC Address.
  GatewayMacCommandData.LanChannel.ChannelNumber = LanChannelNumber;
  GatewayMacCommandData.LanChannel.Reserved = 0;
  GatewayMacCommandData.ParameterSelector = IpmiLanDefaultGatewayMac;
  CopyMem(
    &GatewayMacCommandData.MacAddress[0],
    BmcLanIpSetupValues.RouterMac,
    sizeof(BmcLanIpSetupValues.RouterMac));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a IpmiWaitSetInProgressClear() Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  CompletionCode = 0;
  ResponseDataSize = sizeof(CompletionCode);
  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&GatewayMacCommandData,
    sizeof(GatewayMacCommandData),
    &CompletionCode,
    &ResponseDataSize);
  DEBUG((DEBUG_INFO, " IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS: IpmiLanDefaultGatewayMac: LanChannelNumber: %x Status: %r\n", LanChannelNumber, Status));
  return Status;
}

/**
    Set the LAN IP address source values from BIOS setup to BMC.

    @param[in] LanChannelNumber     LAN channel to use.

    @param[in] IpAddessSource       Value of address source.

    @return EFI_STATUS  Status returned by function calls used.

**/
EFI_STATUS
SetIpAddressSource(
  IN UINT8  LanChannelNumber,
  IN UINT8  IpAddessSource
)
{
  EFI_STATUS                                          Status;
  AMI_SET_LAN_CONFIG_PARAM_IP_ADDRESS_SOURCE_REQUEST  CommandData;
  UINT8                                               CompletionCode;
  UINT8                                               ResponseDataSize;

  // Set IP address source.
  CommandData.LanChannel.ChannelNumber = LanChannelNumber;
  CommandData.LanChannel.Reserved = 0;
  CommandData.ParameterSelector = IpmiLanIpAddressSource;
  CommandData.AddressSource.Source = IpAddessSource;
  CommandData.AddressSource.Reserved = 0;

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (!EFI_ERROR(Status)) {
    ResponseDataSize = sizeof(CompletionCode);
    Status = gIpmiTransport->SendIpmiCommand(
      gIpmiTransport,
      IPMI_NETFN_TRANSPORT,
      AMI_BMC_LUN,
      IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
      (UINT8*)&CommandData,
      sizeof(CommandData),
      &CompletionCode,
      &ResponseDataSize);
    DEBUG((DEBUG_INFO, " IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS: IpAddessSource:%x LanChannelNumber: %x Status: %r\n", IpAddessSource, LanChannelNumber, Status));
  }
  return Status;
}


/**
    In this driver, we will configure BMC LAN channel using static
    information available through setup or using a DHCP client as
    per user request.

    @param[in] ImageHandle  Handle of this driver image.
    @param[in] SystemTable  Table containing standard EFI services.

    @retval EFI_SUCCESS     Driver initialization is done successfully.
    @retval Others          Return status of the function calls used.

**/
EFI_STATUS
EFIAPI
InitializeIp4BmcLanConfig()
{
  EFI_STATUS              Status;
  UINTN                   Size;
  UINT32                  Attributes;
  BMC_IP_BIOS_SETTINGS    BmcLanIp1SetupValues;
#if (BMC_LAN_COUNT == 2)
  BMC_IP_BIOS_SETTINGS    BmcLanIp2SetupValues;
#endif
  UINT8                   ChannelNumberBuffer[BMC_LAN_COUNT];
  UINT8                   ChannelCount = BMC_LAN_COUNT;

  DEBUG((DEBUG_INFO, "%a Entry... \n", __FUNCTION__));

  Status = gBS->LocateProtocol(
    &gEfiDxeIpmiTransportProtocolGuid,
    NULL,
    (VOID**)&gIpmiTransport);
  DEBUG((DEBUG_INFO, " gEfiDxeIpmiTransportProtocolGuid Status: %r \n", Status));
  if (EFI_ERROR(Status)) {
    return Status;
  }

  DEBUG((DEBUG_INFO, "%a ChannelCount = %d\n", __FUNCTION__, ChannelCount));

  Status = AmiIpmiGetLanChannelNumber(
    gIpmiTransport,
    ChannelNumberBuffer,
    &ChannelCount);

  if (EFI_ERROR(Status))
    return Status;

  Size = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  Status = gRT->GetVariable(
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    &gEfiServerMgmtSetupVariableGuid,
    &Attributes,
    &Size,
    &gServerMgmtConfiguration);

  DEBUG((DEBUG_INFO, "GetVariable Status of ServerSetup%r \n", Status));
  if (EFI_ERROR(Status)) {
    BmcLanIp1SetupValues.BmcLan = IpmiUnspecified;
#if (BMC_LAN_COUNT == 2)
    BmcLanIp2SetupValues.BmcLan = IpmiUnspecified;
#endif
  }
  else {
    BmcLanIp1SetupValues.BmcLan = gServerMgmtConfiguration.BmcLan1;
#if (BMC_LAN_COUNT == 2)
    BmcLanIp2SetupValues.BmcLan = gServerMgmtConfiguration.BmcLan2;
#endif
  }

  DEBUG((DEBUG_INFO, "%a ChannelNumberBuffer[0]: %x \n", __FUNCTION__, ChannelNumberBuffer[0]));
  if (ChannelNumberBuffer[0] != 0) {
    gServerMgmtConfiguration.BmcLan1 = IpmiUnspecified;
    /* Static IP has already been validated in Setup. Convert IP and MAC
       strings into Valid IP and MAC format and copy them to module
       structure BmcLanIpSetupValues. Convert IP and MAC of BMC LAN Channel-1.*/
    if (ConvertToIpAddress(
      BmcLanIp1SetupValues.StationIp,
      gServerMgmtConfiguration.StationIp1) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN1 station IP!!!\n"));
      SetDefaultIp(gServerMgmtConfiguration.StationIp1);
    }

    if (ConvertToIpAddress(
      BmcLanIp1SetupValues.Subnet,
      gServerMgmtConfiguration.Subnet1) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN1 Sub-net Mask!!!\n"));
      SetDefaultIp(gServerMgmtConfiguration.Subnet1);
    }

    if (ConvertToIpAddress(
      BmcLanIp1SetupValues.RouterIp,
      gServerMgmtConfiguration.RouterIp1) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN1 Router IP!!!\n"));
      SetZeroIp(gServerMgmtConfiguration.RouterIp1);
    }

    if (ConvertToMacAddress(
      BmcLanIp1SetupValues.RouterMac,
      gServerMgmtConfiguration.RouterMac1) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN1 Router MAC!!!\n"));
      SetZeroMac(gServerMgmtConfiguration.RouterMac1);
    }
  }

#if (BMC_LAN_COUNT == 2)
  DEBUG((DEBUG_INFO, "%a ChannelNumberBuffer[1]: %x\n", __FUNCTION__, ChannelNumberBuffer[1]));
  if (ChannelNumberBuffer[1] != 0) {
    gServerMgmtConfiguration.BmcLan2 = IpmiUnspecified;
    // Convert IP and MAC of BMC LAN Channel-2.
    if (ConvertToIpAddress(
      BmcLanIp2SetupValues.StationIp,
      gServerMgmtConfiguration.StationIp2) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN2 station IP!!!\n"));
      SetDefaultIp(gServerMgmtConfiguration.StationIp2);
    }

    if (ConvertToIpAddress(
      BmcLanIp2SetupValues.Subnet,
      gServerMgmtConfiguration.Subnet2) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN2 Sub-Net Mask!!!\n"));
      SetDefaultIp(gServerMgmtConfiguration.Subnet2);
    }

    if (ConvertToIpAddress(
      BmcLanIp2SetupValues.RouterIp,
      gServerMgmtConfiguration.RouterIp2) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN2 Router IP!!!\n"));
      SetZeroIp(gServerMgmtConfiguration.RouterIp2);
    }

    if (ConvertToMacAddress(
      BmcLanIp2SetupValues.RouterMac,
      gServerMgmtConfiguration.RouterMac2) != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR, "Failed to convert LAN2 Router MAC!!!\n"));
      SetZeroMac(gServerMgmtConfiguration.RouterMac2);
    }
  }
#endif

  Status = gRT->SetVariable(
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    &gEfiServerMgmtSetupVariableGuid,
    Attributes,   // Attributes
    sizeof(SERVER_MGMT_SETUP_CONFIGURATION),
    &gServerMgmtConfiguration);

  DEBUG((DEBUG_INFO, "IPMI : IPv4 SetVariable status %r \n", Status));

  if (EFI_ERROR(Status)) {
    return Status;
  }
  DEBUG((DEBUG_INFO, "#########################\n"));
  DEBUG((DEBUG_INFO, "###Static BMC LAN configuration values from BIOS setup\n"));
  DEBUG((DEBUG_INFO, "#########################\n"));
  DEBUG((DEBUG_INFO, "###  Lan1: %d\n", BmcLanIp1SetupValues.BmcLan));
  DEBUG((DEBUG_INFO, "###  StationIp:  %d %d %d %d\n", BmcLanIp1SetupValues.StationIp[0], BmcLanIp1SetupValues.StationIp[1], BmcLanIp1SetupValues.StationIp[2], BmcLanIp1SetupValues.StationIp[3]));
  DEBUG((DEBUG_INFO, "###  SubnetMask: %d %d %d %d\n", BmcLanIp1SetupValues.Subnet[0], BmcLanIp1SetupValues.Subnet[1], BmcLanIp1SetupValues.Subnet[2], BmcLanIp1SetupValues.Subnet[3]));
  DEBUG((DEBUG_INFO, "###  RouterIp: %d %d %d %d\n", BmcLanIp1SetupValues.RouterIp[0], BmcLanIp1SetupValues.RouterIp[1], BmcLanIp1SetupValues.RouterIp[2], BmcLanIp1SetupValues.RouterIp[3]));
  DEBUG((DEBUG_INFO, "###  Router MAC: %x %x %x %x %x %x\n", BmcLanIp1SetupValues.RouterMac[0], BmcLanIp1SetupValues.RouterMac[1], BmcLanIp1SetupValues.RouterMac[2], BmcLanIp1SetupValues.RouterMac[3], BmcLanIp1SetupValues.RouterMac[4], BmcLanIp1SetupValues.RouterMac[5]));
  DEBUG((DEBUG_INFO, "###----------------------\n"));

  if (ChannelNumberBuffer[0] != 0) {
    if (BmcLanIp1SetupValues.BmcLan != IpmiUnspecified) {
      // Setting LAN1 address source to BMC.
      SetIpAddressSource(
        ChannelNumberBuffer[0],
        BmcLanIp1SetupValues.BmcLan);
    }

    // Configure Static address source parameters to BMC.
    if (BmcLanIp1SetupValues.BmcLan == IpmiStaticAddrsss) {
      SetStaticBmcNetworkParameters(
        ChannelNumberBuffer[0],
        BmcLanIp1SetupValues);
    }
  }
#if (BMC_LAN_COUNT == 2)
  DEBUG((DEBUG_INFO, "###  Lan2: %d\n", BmcLanIp2SetupValues.BmcLan));
  DEBUG((DEBUG_INFO, "###  StationIp:  %d %d %d %d\n", BmcLanIp2SetupValues.StationIp[0], BmcLanIp2SetupValues.StationIp[1], BmcLanIp2SetupValues.StationIp[2], BmcLanIp2SetupValues.StationIp[3]));
  DEBUG((DEBUG_INFO, "###  SubnetMask: %d %d %d %d\n", BmcLanIp2SetupValues.Subnet[0], BmcLanIp2SetupValues.Subnet[1], BmcLanIp2SetupValues.Subnet[2], BmcLanIp2SetupValues.Subnet[3]));
  DEBUG((DEBUG_INFO, "###  RouterIp: %d %d %d %d\n", BmcLanIp2SetupValues.RouterIp[0], BmcLanIp2SetupValues.RouterIp[1], BmcLanIp2SetupValues.RouterIp[2], BmcLanIp2SetupValues.RouterIp[3]));
  DEBUG((DEBUG_INFO, "###  Router MAC: %x %x %x %x %x %x\n", BmcLanIp2SetupValues.RouterMac[0], BmcLanIp2SetupValues.RouterMac[1], BmcLanIp2SetupValues.RouterMac[2], BmcLanIp2SetupValues.RouterMac[3], BmcLanIp2SetupValues.RouterMac[4], BmcLanIp2SetupValues.RouterMac[5]));
  DEBUG((DEBUG_INFO, "#########################\n"));

  if (ChannelNumberBuffer[1] != 0) {
    if (BmcLanIp2SetupValues.BmcLan != IpmiUnspecified) {
      // Setting LAN2 address source to BMC.
      SetIpAddressSource(
        ChannelNumberBuffer[1],
        BmcLanIp2SetupValues.BmcLan);
    }

    // Configure Static address source parameters to BMC.
    if (BmcLanIp2SetupValues.BmcLan == IpmiStaticAddrsss) {
      SetStaticBmcNetworkParameters(
        ChannelNumberBuffer[1],
        BmcLanIp2SetupValues);
    }
  }
#endif

  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));
  return EFI_SUCCESS;
}


/**
  The entry point for BmcLanSetupDxe driver.

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
BmcLanSetupDxeEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  SERVER_MGMT_SETUP_CONFIGURATION *ServerMgmtSetup;
  UINTN ServerMgmtSetupSize;

  Status = GetVariable2(SERVER_MGMT_SETUP_VARIABLE_NAME, &gEfiServerMgmtSetupVariableGuid, &ServerMgmtSetup, &ServerMgmtSetupSize);
  if (EFI_ERROR(Status))
  {
    ServerMgmtSetup = AllocateZeroPool(sizeof(SERVER_MGMT_SETUP_CONFIGURATION));
    if (!ServerMgmtSetup)
    {
      return EFI_OUT_OF_RESOURCES;
    }
    ServerMgmtSetup->RefInterval = 60;
    Status = gRT->SetVariable(
      SERVER_MGMT_SETUP_VARIABLE_NAME,
      &gEfiServerMgmtSetupVariableGuid,
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
      sizeof(SERVER_MGMT_SETUP_CONFIGURATION),
      ServerMgmtSetup
    );
    if (EFI_ERROR(Status))
    {
      return Status;
    }
  }

  Status = InitializeIp4BmcLanConfig();

  Status = InstallBmcLanSetupConfigForm (mConfigPrivate);
  
  return Status;
}
