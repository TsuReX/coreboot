//***********************************************************************
//*                                                                     *
//*   Copyright (c) 1985-2020, American Megatrends International LLC.   *
//*                                                                     *
//*      All rights reserved. Subject to AMI licensing agreement.       *
//*                                                                     *
//***********************************************************************


/** @file BmcLanParam.c
    Reads the LAN parameters from BMC and updates in setup and verifies
    the Static BMC Network configuration with Setup Callback.

*/

//---------------------------------------------------------------------------

#define BmcLanConfig_Live_status 1

#include <Uefi.h>
#include <AmiIpmiNetFnTransportDefinitions.h>
#include "BmcLanConfigCommon.h"
#include <Library/NetLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>

#include "BmcLanSetupImpl.h"
#include "BmcLanParam.h"


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#define DEFAULT_ZERO_START         ((UINTN) ~0)
#define STRING_BUFFER_LENGTH       100
#define SUBNETMASK_CLASS_VALUE     255
#define ENABLED_STATIC_AND_DYNAMIC 3
#define IPV6_SUPPORTED             1

static EFI_IPMI_TRANSPORT* gIpmiTransport;
UINT8                       gChannelNumberBuffer[BMC_LAN_COUNT];
UINT8                       gChannelCount = BMC_LAN_COUNT;

//---------------------------------------------------------------------------

VOID
EFIAPI
PrintPopup(
  IN EFI_STRING String1,
  IN EFI_STRING String2,
  IN EFI_STRING String3
)
{
  EFI_INPUT_KEY  Key;

  do {
    CreatePopUp(
      EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE | EFI_BACKGROUND_RED,
      &Key,
      //String1,
      String2,
      String3,
      NULL
    );
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);

}

/**
     Initialize global variables gIpmiTransport,gChannelNumberBuffer and gChannelCount.

    @return EFI_STATUS      Status returned from AmiInitBmcLanChannel.

**/
EFI_STATUS
AmiInitgVariables(
)
{
  EFI_STATUS               Status;

  Status = gBS->LocateProtocol(
    &gEfiDxeIpmiTransportProtocolGuid,
    NULL,
    (VOID**)&gIpmiTransport);

  DEBUG((DEBUG_INFO, " gEfiDxeIpmiTransportProtocolGuid Status: %r \n", Status));

  if (EFI_ERROR(Status))
    return Status;

  Status = AmiIpmiGetLanChannelNumber(
    gIpmiTransport,
    gChannelNumberBuffer,
    &gChannelCount);

  return Status;
}

/**
    This function validate the given ip and MAC addresses and display error
    messages if given input is invalid data.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Invalid key.
    @retval EFI_SUCCESS         Given input IP and MAC address is valid.
    @retval Others              Return status from functions used.

**/
EFI_STATUS
BmcLanConfigCallbackFunction(
  IN  EFI_HII_HANDLE     HiiHandle,
  IN  UINT16             Key,
  IN CALLBACK_PARAMETERS* CallbackParameters)
{
  SERVER_MGMT_SETUP_CONFIGURATION      ServerMgmtConfiguration;
  BOOLEAN                             BrowserDataStatus;
  BMC_IP_BIOS_SETTINGS                BmcLanIp1SetupValues;
#if (BMC_LAN_COUNT == 2)
  BMC_IP_BIOS_SETTINGS                BmcLanIp2SetupValues;
#endif
  EFI_STRING                          OutputString;
  EFI_STATUS                          Status = EFI_SUCCESS;
  UINTN                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  ;
  DEBUG((DEBUG_INFO, "%a Entered with Key: %x \n", __FUNCTION__, Key));

  // Check for the key and Return if Key value does not match.
  if ((Key != BMC_NETWORK_STATION_IP1_KEY) && \
    (Key != BMC_NETWORK_SUBNET1_KEY) && \
    (Key != BMC_NETWORK_ROUTER_IP1_KEY) && \
    (Key != BMC_NETWORK_ROUTER_MAC1_KEY)) {
    DEBUG((DEBUG_ERROR, "\nCallback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }
  //
  // Get the call back parameters and verify the action
  //
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "\n IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load the defaults \n"));
    return EFI_UNSUPPORTED;
  }

  if (CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED) {
    DEBUG((DEBUG_INFO, "\n IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED... so return EFI_SUCCESS\n"));
    return EFI_SUCCESS;
  }

  //
  // Get Browser DATA
  //
  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nStatus of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }
  // Validate Ip/MAC, for error case, display error message.
  switch (Key) {
  case BMC_NETWORK_STATION_IP1_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp1SetupValues.StationIp,
      ServerMgmtConfiguration.StationIp1);
    DEBUG((DEBUG_INFO, "\nConvertToIpAddress(StationIp1) = %r\n", Status));
    if (EFI_ERROR(Status)) {
      SetDefaultIp(ServerMgmtConfiguration.StationIp1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_STATION_IP),
        NULL);
    }
    break;

  case BMC_NETWORK_SUBNET1_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp1SetupValues.Subnet,
      ServerMgmtConfiguration.Subnet1);
    DEBUG((DEBUG_INFO, "\nConvertToIpAddress(Subnet1) = %r\n", Status));
    if (EFI_ERROR(Status) || BmcLanIp1SetupValues.Subnet[0] != SUBNETMASK_CLASS_VALUE) {
      Status = EFI_INVALID_PARAMETER;
      SetDefaultIp(ServerMgmtConfiguration.Subnet1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_SUBNETMASK),
        NULL);
    }
    break;

  case BMC_NETWORK_ROUTER_IP1_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp1SetupValues.RouterIp,
      ServerMgmtConfiguration.RouterIp1);
    DEBUG((DEBUG_INFO, "\nConvertToIpAddress(RouterIp1) = %r\n", Status));
    if (EFI_ERROR(Status)) {
      SetZeroIp(ServerMgmtConfiguration.RouterIp1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_ROUTER_IP),
        NULL);
    }
    break;

  case BMC_NETWORK_ROUTER_MAC1_KEY:
    Status = ConvertToMacAddress(
      BmcLanIp1SetupValues.RouterMac,
      ServerMgmtConfiguration.RouterMac1);
    DEBUG((DEBUG_INFO, "\nConvertToIpAddress(RouterMac1) = %r\n", Status));
    if (EFI_ERROR(Status)) {
      SetZeroMac(ServerMgmtConfiguration.RouterMac1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_ROUTER_MAC),
        NULL);
    }
    break;
#if (BMC_LAN_COUNT == 2) 

  case BMC_NETWORK_STATION_IP2_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp2SetupValues.StationIp,
      ServerMgmtConfiguration.StationIp2);
    if (EFI_ERROR(Status)) {
      SetDefaultIp(ServerMgmtConfiguration.StationIp2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_STATION_IP),
        NULL);
    }
    break;

  case BMC_NETWORK_SUBNET2_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp2SetupValues.Subnet,
      ServerMgmtConfiguration.Subnet2);
    if (EFI_ERROR(Status) || BmcLanIp2SetupValues.Subnet[0] != SUBNETMASK_CLASS_VALUE) {
      Status = EFI_INVALID_PARAMETER;
      SetDefaultIp(ServerMgmtConfiguration.Subnet2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_SUBNETMASK),
        NULL);
    }
    break;

  case BMC_NETWORK_ROUTER_IP2_KEY:
    Status = ConvertToIpAddress(
      BmcLanIp2SetupValues.RouterIp,
      ServerMgmtConfiguration.RouterIp2);
    if (EFI_ERROR(Status)) {
      SetZeroIp(ServerMgmtConfiguration.RouterIp2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_ROUTER_IP),
        NULL);
    }
    break;

  case BMC_NETWORK_ROUTER_MAC2_KEY:
    Status = ConvertToMacAddress(
      BmcLanIp2SetupValues.RouterMac,
      ServerMgmtConfiguration.RouterMac2);
    if (EFI_ERROR(Status)) {
      SetZeroMac(ServerMgmtConfiguration.RouterMac2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_ROUTER_MAC),
        NULL);
    }
    break;
#endif
  default:
    DEBUG((DEBUG_ERROR, "\nCallback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }
  if (EFI_ERROR(Status)) {

    // POPUP

    CHAR16* ContinueString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_PRESS_ENTER_TO_CONTINUE),
      NULL);
    CHAR16* CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_BMCLAN_ERROR_INFO),
      NULL);
    if ((OutputString != NULL) && (CharPtrString != NULL)) {
      PrintPopup(
        CharPtrString,
        OutputString,
        ContinueString
        );
      gBS->FreePool(OutputString);
      gBS->FreePool(CharPtrString);
      gBS->FreePool(ContinueString);
    }
    
    // Set Browser DATA
    BrowserDataStatus = HiiSetBrowserData(
      &gEfiServerMgmtSetupVariableGuid,
      SERVER_MGMT_SETUP_VARIABLE_NAME,
      SelectionBufferSize,
      (UINT8*)&ServerMgmtConfiguration,
      NULL);

    if (!BrowserDataStatus) {
      DEBUG((DEBUG_ERROR, "\nStatus of HiiSetBrowserData() = %d\n", BrowserDataStatus));
      return EFI_UNSUPPORTED;
    }
  }

  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));

  return EFI_SUCCESS;
}

/**
    Reads the LAN parameters from BMC and updates in Setup.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.

    @return VOID    Nothing.

**/
VOID
UpdateIp4Parameters(
  IN EFI_HII_HANDLE HiiHandle
)
{
  EFI_STATUS                                          Status;
  UINT8                                               CommandDataSize;
  AMI_GET_LAN_CONFIG_PARAM_REQUEST                    GetLanParamCmd;
  AMI_GET_LAN_CONFIG_PARAM_IP_ADDRESS_SRC_RESPONSE    LanAddressSource;
  AMI_GET_LAN_CONFIG_PARAM_MAC_ADDRESS_RESPONSE       LanMacAddress;
  AMI_GET_LAN_CONFIG_PARAM_IP_ADDRESS_RESPONSE        LanIpAddress;
  AMI_GET_LAN_CONFIG_PARAM_SUBNET_MASK_RESPONSE       LanSubnetMask;
  UINT8                                               ResponseDataSize;
  UINT16                                              IpSource[5] = {
                                                                        STRING_TOKEN(STR_UNSPECIFIED),
                                                                        STRING_TOKEN(STR_IPSOURCE_STATIC_ADDRESS),
                                                                        STRING_TOKEN(STR_IPSOURCE_DYNAMIC_ADDRESS_BMC_DHCP),
                                                                        STRING_TOKEN(STR_IPSOURCE_DYNAMIC_ADDRESS_BIOS_DHCP),
                                                                        STRING_TOKEN(STR_IPSOURCE_DYNAMIC_ADDRESS_BMC_NON_DHCP)
  };
  UINT8                                               IpSourceIndex;
  EFI_STRING                    CharPtrString;
  UINTN                                               BufferLength = STRING_BUFFER_LENGTH;

  DEBUG((DEBUG_INFO, "%a Entry...\n", __FUNCTION__));

  if (gChannelNumberBuffer[0] != 0) {
    // Common for all LAN 1 Channel.
    GetLanParamCmd.LanChannel.ChannelNumber = gChannelNumberBuffer[0];
    GetLanParamCmd.LanChannel.Reserved = 0;
    GetLanParamCmd.LanChannel.GetParam = 0;
    GetLanParamCmd.SetSelector = AMI_IPMI_SELECTOR_NONE;
    GetLanParamCmd.BlockSelector = AMI_IPMI_SELECTOR_NONE;
    CommandDataSize = sizeof(GetLanParamCmd);

    // Get IP address Source for Channel-1
    GetLanParamCmd.ParameterSelector = IpmiLanIpAddressSource;

    // Wait until Set In Progress field is cleared.
    Status = IpmiWaitSetInProgressClear(
      gIpmiTransport,
      gChannelNumberBuffer[0]);
    if (!EFI_ERROR(Status)) {
      ResponseDataSize = sizeof(LanAddressSource);
      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&GetLanParamCmd,
        CommandDataSize,
        (UINT8*)&LanAddressSource,
        &ResponseDataSize);
      DEBUG((DEBUG_INFO, "IPMI IPV4 common GET LAN CONFIG IP ADDR STATUS= %r...\n", Status));
      if (!EFI_ERROR(Status)) {
        if (LanAddressSource.AddressSrc > IpmiDynamicAddressBmcNonDhcp) {
          IpSourceIndex = 0;
        }
        else {
          IpSourceIndex = LanAddressSource.AddressSrc;
        }
        CharPtrString = HiiGetString(
          HiiHandle,
          IpSource[IpSourceIndex],
          NULL);
        if (CharPtrString != NULL) {
          DEBUG((DEBUG_INFO, "IPv4 common ip addr CharPtrString= %x...\n", CharPtrString));
          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_CURR_LANCAS1_VAL),
            CharPtrString,
            NULL);
          gBS->FreePool(CharPtrString);
        }
      }
    }
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "IPv4 common ip addr failed \n"));
      CharPtrString = HiiGetString(
        HiiHandle,
        IpSource[0],
        NULL);
      if (CharPtrString != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_CURR_LANCAS1_VAL),
          CharPtrString,
          NULL);
        gBS->FreePool(CharPtrString);
      }
    }

    // Get station IP address of Channel-1
    GetLanParamCmd.ParameterSelector = IpmiLanIpAddress;

    CharPtrString = AllocateZeroPool(STRING_BUFFER_LENGTH);
    if (CharPtrString != NULL) {
      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[0]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanIpAddress);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanIpAddress,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP ADDR STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            LanIpAddress.Ipv4Address.Addr[0],
            LanIpAddress.Ipv4Address.Addr[1],
            LanIpAddress.Ipv4Address.Addr[2],
            LanIpAddress.Ipv4Address.Addr[3]);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_STATION_IP1_VAL),
            CharPtrString,
            NULL);
        }
      }
      if (EFI_ERROR(Status)) {
        UnicodeSPrint(
          CharPtrString,
          STRING_BUFFER_LENGTH,
          L"%d.%d.%d.%d",
          0,
          0,
          0,
          0);

        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP1_VAL),
          CharPtrString,
          NULL);
      }

      // Get Subnet mask of Channel-1.
      GetLanParamCmd.ParameterSelector = IpmiLanSubnetMask;

      // Wait until Set In Progress field is cleared
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[0]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanSubnetMask);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanSubnetMask,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG SUBNET MASK STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            LanSubnetMask.SubnetMask[0],
            LanSubnetMask.SubnetMask[1],
            LanSubnetMask.SubnetMask[2],
            LanSubnetMask.SubnetMask[3]);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_SUBNET_MASK1_VAL),
            CharPtrString,
            NULL);
        }
      }
      if (EFI_ERROR(Status)) {
        UnicodeSPrint(
          CharPtrString,
          STRING_BUFFER_LENGTH,
          L"%d.%d.%d.%d",
          0,
          0,
          0,
          0);

        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_SUBNET_MASK1_VAL),
          CharPtrString,
          NULL);
      }

      // Get MAC address of Channel-1.
      GetLanParamCmd.ParameterSelector = IpmiLanMacAddress;

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[0]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanMacAddress);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanMacAddress,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG MAC ADDR STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%02x-%02x-%02x-%02x-%02x-%02x",
            LanMacAddress.MacAddress[0],
            LanMacAddress.MacAddress[1],
            LanMacAddress.MacAddress[2],
            LanMacAddress.MacAddress[3],
            LanMacAddress.MacAddress[4],
            LanMacAddress.MacAddress[5]);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_STATION_MAC1_VAL),
            CharPtrString,
            NULL);
        }
      }
      if (EFI_ERROR(Status)) {
        UnicodeSPrint(
          CharPtrString,
          STRING_BUFFER_LENGTH,
          L"%02x-%02x-%02x-%02x-%02x-%02x",
          0,
          0,
          0,
          0,
          0,
          0);

        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_MAC1_VAL),
          CharPtrString,
          NULL);
      }

      // Get Router Gateway IP Address of Channel-1.
      GetLanParamCmd.ParameterSelector = IpmiLanDefaultGateway;

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[0]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanIpAddress);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanIpAddress,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG ROUTER GATE WAY STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            LanIpAddress.Ipv4Address.Addr[0],
            LanIpAddress.Ipv4Address.Addr[1],
            LanIpAddress.Ipv4Address.Addr[2],
            LanIpAddress.Ipv4Address.Addr[3]);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_ROUTER_IP1_VAL),
            CharPtrString,
            NULL);
        }
      }
      if (EFI_ERROR(Status)) {
        UnicodeSPrint(
          CharPtrString,
          STRING_BUFFER_LENGTH,
          L"%d.%d.%d.%d",
          0,
          0,
          0,
          0);

        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_ROUTER_IP1_VAL),
          CharPtrString,
          NULL);
      }

      // Get Router MAC address of Channel-1.
      GetLanParamCmd.ParameterSelector = IpmiLanDefaultGatewayMac;

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[0]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanMacAddress);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanMacAddress,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG ROUTER MAC ADDR STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%02x-%02x-%02x-%02x-%02x-%02x",
            LanMacAddress.MacAddress[0],
            LanMacAddress.MacAddress[1],
            LanMacAddress.MacAddress[2],
            LanMacAddress.MacAddress[3],
            LanMacAddress.MacAddress[4],
            LanMacAddress.MacAddress[5]);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_ROUTER_MAC1_VAL),
            CharPtrString,
            NULL);
        }
      }
      if (EFI_ERROR(Status)) {
        UnicodeSPrint(
          CharPtrString,
          STRING_BUFFER_LENGTH,
          L"%02x-%02x-%02x-%02x-%02x-%02x",
          0,
          0,
          0,
          0,
          0,
          0);

        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_ROUTER_MAC1_VAL),
          CharPtrString,
          NULL);
      }
      gBS->FreePool(CharPtrString);
    }
  }
  if (BMC_LAN_COUNT == 2) {
    if (gChannelNumberBuffer[1] != 0) {
      // Common for all LAN 2 Channel.
      GetLanParamCmd.LanChannel.ChannelNumber = gChannelNumberBuffer[1];
      GetLanParamCmd.LanChannel.Reserved = 0;
      GetLanParamCmd.LanChannel.GetParam = 0;
      GetLanParamCmd.SetSelector = AMI_IPMI_SELECTOR_NONE;
      GetLanParamCmd.BlockSelector = AMI_IPMI_SELECTOR_NONE;
      CommandDataSize = sizeof(GetLanParamCmd);

      // Get IP address Source for Channel-2.
      GetLanParamCmd.ParameterSelector = IpmiLanIpAddressSource;
      BufferLength = STRING_BUFFER_LENGTH;

      // Wait until Set In Progress field is cleared
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        gChannelNumberBuffer[1]);
      if (!EFI_ERROR(Status)) {
        ResponseDataSize = sizeof(LanAddressSource);
        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetLanParamCmd,
          CommandDataSize,
          (UINT8*)&LanAddressSource,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG common IP addr channel 2 STATUS= %r...\n", Status));
        if (!EFI_ERROR(Status)) {
          if (LanAddressSource.AddressSrc > 4) {
            IpSourceIndex = 0;
          }
          else {
            IpSourceIndex = LanAddressSource.AddressSrc;
          }

          CharPtrString = HiiGetString(
            HiiHandle,
            IpSource[IpSourceIndex],
            NULL);
          if (CharPtrString != NULL) {
            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_CURR_LANCAS2_VAL),
              CharPtrString,
              NULL);
            gBS->FreePool(CharPtrString);
          }
        }
      }
      if (EFI_ERROR(Status)) {
        CharPtrString = HiiGetString(
          HiiHandle,
          IpSource[0],
          NULL);
        if (CharPtrString != NULL) {
          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_CURR_LANCAS2_VAL),
            CharPtrString,
            NULL);
          gBS->FreePool(CharPtrString);
        }
      }

      // Get station IP address of Channel-2.
      GetLanParamCmd.ParameterSelector = IpmiLanIpAddress;

      CharPtrString = AllocateZeroPool(STRING_BUFFER_LENGTH);
      if (CharPtrString != NULL) {
        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          gChannelNumberBuffer[1]);
        if (!EFI_ERROR(Status)) {
          ResponseDataSize = sizeof(LanIpAddress);
          Status = gIpmiTransport->SendIpmiCommand(
            gIpmiTransport,
            IPMI_NETFN_TRANSPORT,
            AMI_BMC_LUN,
            IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
            (UINT8*)&GetLanParamCmd,
            CommandDataSize,
            (UINT8*)&LanIpAddress,
            &ResponseDataSize);
          DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP addr channel 2 STATUS= %r...\n", Status));
          if (!EFI_ERROR(Status)) {
            UnicodeSPrint(
              CharPtrString,
              STRING_BUFFER_LENGTH,
              L"%d.%d.%d.%d",
              LanIpAddress.Ipv4Address.Addr[0],
              LanIpAddress.Ipv4Address.Addr[1],
              LanIpAddress.Ipv4Address.Addr[2],
              LanIpAddress.Ipv4Address.Addr[3]);

            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_STATION_IP2_VAL),
              CharPtrString,
              NULL);
          }
        }
        if (EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            0,
            0,
            0,
            0);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_STATION_IP2_VAL),
            CharPtrString,
            NULL);
        }

        // Get Subnet mask of Channel-2.
        GetLanParamCmd.ParameterSelector = IpmiLanSubnetMask;

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          gChannelNumberBuffer[1]);
        if (!EFI_ERROR(Status)) {
          ResponseDataSize = sizeof(LanSubnetMask);
          Status = gIpmiTransport->SendIpmiCommand(
            gIpmiTransport,
            IPMI_NETFN_TRANSPORT,
            AMI_BMC_LUN,
            IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
            (UINT8*)&GetLanParamCmd,
            CommandDataSize,
            (UINT8*)&LanSubnetMask,
            &ResponseDataSize);
          DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP 2 sub net mask STATUS= %r...\n", Status));
          if (!EFI_ERROR(Status)) {
            UnicodeSPrint(
              CharPtrString,
              STRING_BUFFER_LENGTH,
              L"%d.%d.%d.%d",
              LanSubnetMask.SubnetMask[0],
              LanSubnetMask.SubnetMask[1],
              LanSubnetMask.SubnetMask[2],
              LanSubnetMask.SubnetMask[3]);

            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_SUBNET_MASK2_VAL),
              CharPtrString,
              NULL);
          }
        }
        if (EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            0,
            0,
            0,
            0);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_SUBNET_MASK2_VAL),
            CharPtrString,
            NULL);
        }

        // Get MAC address of Channel-2.
        GetLanParamCmd.ParameterSelector = IpmiLanMacAddress;

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          gChannelNumberBuffer[1]);
        if (!EFI_ERROR(Status)) {
          ResponseDataSize = sizeof(LanMacAddress);
          Status = gIpmiTransport->SendIpmiCommand(
            gIpmiTransport,
            IPMI_NETFN_TRANSPORT,
            AMI_BMC_LUN,
            IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
            (UINT8*)&GetLanParamCmd,
            CommandDataSize,
            (UINT8*)&LanMacAddress,
            &ResponseDataSize);
          DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP 2 MAC STATUS= %r...\n", Status));
          if (!EFI_ERROR(Status)) {
            UnicodeSPrint(
              CharPtrString,
              STRING_BUFFER_LENGTH,
              L"%02x-%02x-%02x-%02x-%02x-%02x",
              LanMacAddress.MacAddress[0],
              LanMacAddress.MacAddress[1],
              LanMacAddress.MacAddress[2],
              LanMacAddress.MacAddress[3],
              LanMacAddress.MacAddress[4],
              LanMacAddress.MacAddress[5]);

            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_STATION_MAC2_VAL),
              CharPtrString,
              NULL);
          }
        }
        if (EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%02x-%02x-%02x-%02x-%02x-%02x",
            0,
            0,
            0,
            0,
            0,
            0);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_STATION_MAC2_VAL),
            CharPtrString,
            NULL);
        }

        // Get Router Gateway IP Address of Channel-2.
        GetLanParamCmd.ParameterSelector = IpmiLanDefaultGateway;

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          gChannelNumberBuffer[1]);
        if (!EFI_ERROR(Status)) {
          ResponseDataSize = sizeof(LanIpAddress);
          Status = gIpmiTransport->SendIpmiCommand(
            gIpmiTransport,
            IPMI_NETFN_TRANSPORT,
            AMI_BMC_LUN,
            IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
            (UINT8*)&GetLanParamCmd,
            CommandDataSize,
            (UINT8*)&LanIpAddress,
            &ResponseDataSize);
          DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP 2 ROUTER GATEWAY STATUS= %r...\n", Status));
          if (!EFI_ERROR(Status)) {
            UnicodeSPrint(
              CharPtrString,
              STRING_BUFFER_LENGTH,
              L"%d.%d.%d.%d",
              LanIpAddress.Ipv4Address.Addr[0],
              LanIpAddress.Ipv4Address.Addr[1],
              LanIpAddress.Ipv4Address.Addr[2],
              LanIpAddress.Ipv4Address.Addr[3]);

            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_ROUTER_IP2_VAL),
              CharPtrString,
              NULL);
          }
        }
        if (EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%d.%d.%d.%d",
            0,
            0,
            0,
            0);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_ROUTER_IP2_VAL),
            CharPtrString,
            NULL);
        }

        // Get Router MAC address of Channel-2.
        GetLanParamCmd.ParameterSelector = IpmiLanDefaultGatewayMac;

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          gChannelNumberBuffer[1]);
        if (!EFI_ERROR(Status)) {
          ResponseDataSize = sizeof(LanMacAddress);
          Status = gIpmiTransport->SendIpmiCommand(
            gIpmiTransport,
            IPMI_NETFN_TRANSPORT,
            AMI_BMC_LUN,
            IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
            (UINT8*)&GetLanParamCmd,
            CommandDataSize,
            (UINT8*)&LanMacAddress,
            &ResponseDataSize);
          DEBUG((DEBUG_INFO, "IPMI IPV4  GET LAN CONFIG IP 2 ROUTER MAC STATUS= %r...\n", Status));
          if (!EFI_ERROR(Status)) {
            UnicodeSPrint(
              CharPtrString,
              STRING_BUFFER_LENGTH,
              L"%02x-%02x-%02x-%02x-%02x-%02x",
              LanMacAddress.MacAddress[0],
              LanMacAddress.MacAddress[1],
              LanMacAddress.MacAddress[2],
              LanMacAddress.MacAddress[3],
              LanMacAddress.MacAddress[4],
              LanMacAddress.MacAddress[5]);

            HiiSetString(
              HiiHandle,
              STRING_TOKEN(STR_ROUTER_MAC2_VAL),
              CharPtrString,
              NULL);
          }
        }
        if (EFI_ERROR(Status)) {
          UnicodeSPrint(
            CharPtrString,
            STRING_BUFFER_LENGTH,
            L"%02x-%02x-%02x-%02x-%02x-%02x",
            0,
            0,
            0,
            0,
            0,
            0);

          HiiSetString(
            HiiHandle,
            STRING_TOKEN(STR_ROUTER_MAC2_VAL),
            CharPtrString,
            NULL);
        }
        gBS->FreePool(CharPtrString);
      }
    }
  }
  DEBUG((DEBUG_INFO, "%a Returning...\n", __FUNCTION__));
}

/**
    This function validate the given ip and display error messages
    if given input is invalid data.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Invalid key.
    @retval EFI_SUCCESS         Given input IP is valid.
    @retval Others              Return status from functions used.

**/
EFI_STATUS
BmcLanIpv6ConfigCallbackFunction(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  SERVER_MGMT_SETUP_CONFIGURATION      ServerMgmtConfiguration;
  BOOLEAN                             BrowserDataStatus;
  EFI_STRING                          OutputString;
  EFI_STATUS                          Status = EFI_SUCCESS;
  UINTN                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  EFI_IPv6_ADDRESS                    Ipv6Address;
  CHAR16                              StaticIp6Lan[40];

  DEBUG((DEBUG_INFO, "%a Entered with Key: %x \n", __FUNCTION__, Key));

  // Check for the key and return if Key value does not match.
  if ((Key != BMC_NETWORK_STATION_IP6_LAN1_KEY) && \
    (Key != BMC_NETWORK_STATION_IP6_LAN2_KEY)) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Callback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }

  // Get the call back parameters and verify the action
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load defaults \n"));
    return EFI_UNSUPPORTED;
  }
  else if (CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED) {
    DEBUG((DEBUG_INFO, "\n IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED... so return EFI_SUCCESS\n"));
    return EFI_SUCCESS;
  }

  // Get Browser DATA
  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  if (Key == BMC_NETWORK_STATION_IP6_LAN1_KEY) {
    /* Validate Ipv6 value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.StaticIp6Lan1,
      sizeof(ServerMgmtConfiguration.StaticIp6Lan1));
    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);
    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.StaticIp6Lan1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_BMC_IPV6_ADDRESS),
        NULL);
    }
  }

#if (BMC_LAN_COUNT == 2) 
  if (Key == BMC_NETWORK_STATION_IP6_LAN2_KEY) {
    /* Validate Ipv6 value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.StaticIp6Lan2,
      sizeof(ServerMgmtConfiguration.StaticIp6Lan2));
    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);
    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.StaticIp6Lan2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_BMC_IPV6_ADDRESS),
        NULL);
    }
  }
#endif

  if (EFI_ERROR(Status)) {
    // POPUP

    CHAR16* ContinueString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_PRESS_ENTER_TO_CONTINUE),
      NULL);
    CHAR16* CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_BMCLAN_ERROR_INFO),
      NULL);
    if ((OutputString != NULL) && (CharPtrString != NULL)) {
      PrintPopup(
        CharPtrString,
        OutputString,
        ContinueString
      );
      gBS->FreePool(OutputString);
      gBS->FreePool(CharPtrString);
      gBS->FreePool(ContinueString);
    }

    // Set browser data.
    BrowserDataStatus = HiiSetBrowserData(
      &gEfiServerMgmtSetupVariableGuid,
      SERVER_MGMT_SETUP_VARIABLE_NAME,
      SelectionBufferSize,
      (UINT8*)&ServerMgmtConfiguration,
      NULL);

    if (!BrowserDataStatus) {
      DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiSetBrowserData() = %d\n", BrowserDataStatus));
      return EFI_UNSUPPORTED;
    }
  }

  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));

  return EFI_SUCCESS;
}

/**
    This function validate the given router IPv6 address and display error
    messages if given input is invalid data.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Invalid key.
    @retval EFI_SUCCESS         Given input router IPv6 address is valid.
    @retval Others              Return status from functions used.

**/
EFI_STATUS
BmcLanIpv6Router1ConfigCallbackFunction(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  SERVER_MGMT_SETUP_CONFIGURATION     ServerMgmtConfiguration;
  BOOLEAN                             BrowserDataStatus;
  EFI_STRING                          OutputString;
  EFI_STATUS                          Status = EFI_SUCCESS;
  UINTN                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  EFI_IPv6_ADDRESS                    Ipv6Address;
  CHAR16                              StaticIp6Lan[40];

  DEBUG((DEBUG_INFO, "%a Entered with Key: %x \n", __FUNCTION__, Key));

  // Check for the key and Return if Key value does not match.
  if ((Key != BMC_NETWORK_STATION_IP6_ROUTER1_LAN2_KEY) && \
    (Key != BMC_NETWORK_STATION_IP6_ROUTER1_LAN1_KEY)) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Callback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }

  // Get the call back parameters and verify the action
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load defaults \n"));
    return EFI_UNSUPPORTED;
  }
  else if (CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED) {
    DEBUG((DEBUG_INFO, "\n IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED... so return EFI_SUCCESS\n"));
    return EFI_SUCCESS;
  }

  // Get Browser DATA
  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  if (Key == BMC_NETWORK_STATION_IP6_ROUTER1_LAN1_KEY) {
    /* Validate Ipv6 value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.StaticIp6Lan1Router1Ip,
      sizeof(ServerMgmtConfiguration.StaticIp6Lan1Router1Ip));
    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);
    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.StaticIp6Lan1Router1Ip);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_BMC_IPV6_ADDRESS),
        NULL);
    }
  }

#if (BMC_LAN_COUNT == 2)
  if (Key == BMC_NETWORK_STATION_IP6_ROUTER1_LAN2_KEY) {
    /* Validate Ipv6 value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.StaticIp6Lan2Router1Ip,
      sizeof(ServerMgmtConfiguration.StaticIp6Lan2Router1Ip));
    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);
    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.StaticIp6Lan2Router1Ip);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_BMC_IPV6_ADDRESS),
        NULL);
    }
  }
#endif
  if (EFI_ERROR(Status)) {

    // POPUP

    CHAR16* ContinueString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_PRESS_ENTER_TO_CONTINUE),
      NULL);
    CHAR16* CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_BMCLAN_ERROR_INFO),
      NULL);
    if ((OutputString != NULL) && (CharPtrString != NULL)) {
      PrintPopup(
        CharPtrString,
        OutputString,
        ContinueString
      );
      gBS->FreePool(OutputString);
      gBS->FreePool(CharPtrString);
      gBS->FreePool(ContinueString);
    }

    // Set browser data.
    BrowserDataStatus = HiiSetBrowserData(
      &gEfiServerMgmtSetupVariableGuid,
      SERVER_MGMT_SETUP_VARIABLE_NAME,
      SelectionBufferSize,
      (UINT8*)&ServerMgmtConfiguration,
      NULL);
    DEBUG((DEBUG_INFO, "\nIPMI: Status of HiiSetBrowserData() = %d\n", BrowserDataStatus));
    if (!BrowserDataStatus) {
      return EFI_UNSUPPORTED;
    }
  }


  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));

  return EFI_SUCCESS;
}

/**
    This function enables/disables the Ipv6 Support setup option.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Invalid key.
    @retval EFI_SUCCESS         Ipv6 Support is enabled/disabled successfully.
    @retval Others              Return status from functions used.

**/
EFI_STATUS
BmcLanIpv6SupportCallbackFunction(
  IN  EFI_HII_HANDLE     HiiHandle,
  IN  UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  SERVER_MGMT_SETUP_CONFIGURATION                      ServerMgmtConfiguration;
  BOOLEAN                                             BrowserDataStatus;
  EFI_STATUS                                          Status = EFI_SUCCESS;
  UINTN                                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  AMI_SET_LAN_CONFIG_IP_4_6_SUPPORT_ENABLE_REQUEST    SetParam;
  UINT8                                               ResponseDataSize;
  UINT8                                               RequestDataSize;
  UINT8                                               CompletionCode;

  DEBUG((DEBUG_INFO, "%a Entered with Key: %x \n", __FUNCTION__, Key));

  // Check for the key and Return if Key value does not match.
  if ((Key != BMC_IPV6_SUPPORT_LAN1_KEY) && \
    (Key != BMC_IPV6_SUPPORT_LAN2_KEY)) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Callback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }

  // Get the call back parameters and verify the action.
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "\n IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load defaults \n"));
    return EFI_UNSUPPORTED;
  }
  else if (CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED) {
    DEBUG((DEBUG_INFO, "\n IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED... so return EFI_SUCCESS\n"));
    return EFI_SUCCESS;
  }

  // Check for valid Lan Channel
  if (!gChannelNumberBuffer[0]) {
    Status = AmiInitgVariables();

    if (EFI_ERROR(Status))
      return EFI_UNSUPPORTED;
  }

  // Get browser data.
  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  if (gIpmiTransport == NULL) {   // IPMI transport protocol is not found.
    DEBUG((DEBUG_ERROR, "\nIPMI: IPMI transport protocol is not found"));
    goto END;
  }

  if (Key == BMC_IPV6_SUPPORT_LAN1_KEY) {
    if (gChannelNumberBuffer[0] != 0) {
      RequestDataSize = sizeof(SetParam);
      SetParam.LanChannel.ChannelNumber = gChannelNumberBuffer[0];
      SetParam.LanChannel.Reserved = 0;
      SetParam.ParameterSelector = AmiIpmiIpv4OrIpv6AddressEnable;
      if (ServerMgmtConfiguration.Ip6Support1 == 0) {
        SetParam.Data1 = 0;
      }
      else if (ServerMgmtConfiguration.Ip6Support1 == 1) {
        SetParam.Data1 = 2;
      }
      ResponseDataSize = sizeof(CompletionCode);
      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&SetParam,
        RequestDataSize,
        &CompletionCode,
        &ResponseDataSize);
    }
  }

#if (BMC_LAN_COUNT == 2)
  if (Key == BMC_IPV6_SUPPORT_LAN2_KEY) {
    if (gChannelNumberBuffer[1] != 0) {
      RequestDataSize = sizeof(SetParam);
      SetParam.LanChannel.ChannelNumber = gChannelNumberBuffer[1];
      SetParam.LanChannel.Reserved = 0;
      SetParam.ParameterSelector = AmiIpmiIpv4OrIpv6AddressEnable;
      if (ServerMgmtConfiguration.Ip6Support2 == 0) {
        SetParam.Data1 = 0;
      }
      else if (ServerMgmtConfiguration.Ip6Support2 == 1) {
        SetParam.Data1 = 2;
      }
      ResponseDataSize = sizeof(CompletionCode);

      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&SetParam,
        RequestDataSize,
        &CompletionCode,
        &ResponseDataSize);
    }
  }
#endif
END:
  // Set browser data.
  BrowserDataStatus = HiiSetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration,
    NULL);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiSetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));

  return EFI_SUCCESS;
}

/** @internal
    This function update Ipv6 LAN parameters data.

    @param[in] HiiHandle            Handle to HII database.
    @param[in] LanChannelNumber     Channel need to be updated.

    @return VOID    Nothing.

**/
VOID
UpdateIp6Parameters(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT8              LanChannelNumber)
{
  EFI_STATUS                                                          Status;
  AMI_GET_LAN_CONFIG_PARAM_REQUEST                                    GetParamCommand;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_ADDRESS_RESPONSE                      Ipv6AddressData;
  UINT8                                                               ResponseDataSize;
  UINT8                                                               IsIpv6Static = 1;
  EFI_STRING                                                          CharPtrString;
  UINT16                                                              Ipv6Status[7] = {
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_ACTIVE),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_DISABLE),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_PENDING),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_FAILED),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_DEPRECATED),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_INVALID),
                                                                                        STRING_TOKEN(STR_BMC_IPV6_ADDRESS_STATUS_RESERVED)
  };
  UINT8                                                               Ipv6StatusIndex = 0;

  DEBUG((DEBUG_INFO, "%a Entry...\n", __FUNCTION__));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    return;
  }

  // Get IPV6 address Status.
  GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
  GetParamCommand.LanChannel.GetParam = 0;
  GetParamCommand.LanChannel.Reserved = 0;
  GetParamCommand.ParameterSelector = AmiIpmiIpv6StaticAddress;
  GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
  GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
  ResponseDataSize = sizeof(Ipv6AddressData);

  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&GetParamCommand,
    sizeof(GetParamCommand),
    (UINT8*)&Ipv6AddressData,
    &ResponseDataSize);

  DEBUG((DEBUG_INFO, " IPMI : Get IPv6 Address Status: %r\n", Status));

  if (EFI_ERROR(Status)) {
    return;
  }

  DEBUG((DEBUG_INFO, " IPMI : Static Support: %d\n", Ipv6AddressData.Ip6AddressParam.AddressSource.EnableStaticSource));

  // If the Ipv6 address static bit is 0, then Ipv6 address is not static.
  if (Ipv6AddressData.Ip6AddressParam.AddressSource.EnableStaticSource == AMI_IPMI_BIT_CLEAR) {
    IsIpv6Static = 0;
  }
  if (IsIpv6Static) {
    // Get the address source mapping UNI string.
    CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_IPSOURCE_STATIC_ADDRESS),
      NULL);
  }
  else {
    // Get the address source mapping UNI string.
    CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_IPSOURCE_DYNAMIC_ADDRESS_BMC_DHCP),
      NULL);

  }

  if (CharPtrString != NULL) {
    if (LanChannelNumber == gChannelNumberBuffer[0]) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_IP6_CURR_LANCAS1_VAL),
        CharPtrString,
        NULL);
    }
    if ((BMC_LAN_COUNT == 2) && (LanChannelNumber == gChannelNumberBuffer[1])) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_IP6_CURR_LANCAS2_VAL),
        CharPtrString,
        NULL);
    }
    gBS->FreePool(CharPtrString);
  }

  // Update address parameter if Dynamic is selected.
  if (!IsIpv6Static) {
    /* Update the parameter for getting Dynamic address.
       Once again command will be sent to get dynamic address.*/
    GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
    GetParamCommand.LanChannel.GetParam = 0;
    GetParamCommand.LanChannel.Reserved = 0;
    GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
    GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
    GetParamCommand.ParameterSelector = AmiIpmiIpv6DhcpAddress;
    ResponseDataSize = sizeof(Ipv6AddressData);

    Status = gIpmiTransport->SendIpmiCommand(
      gIpmiTransport,
      IPMI_NETFN_TRANSPORT,
      AMI_BMC_LUN,
      IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
      (UINT8*)&GetParamCommand,
      sizeof(GetParamCommand),
      (UINT8*)&Ipv6AddressData,
      &ResponseDataSize);

    DEBUG((DEBUG_INFO, " IPMI: Get IPv6 Dynamic Address Status: %r\n", Status));

    if (EFI_ERROR(Status)) {
      return;
    }
  }

  CharPtrString = AllocateZeroPool(STRING_BUFFER_LENGTH);
  if (CharPtrString != NULL) {
    // Update Ipv6 address.
    NetLibIp6ToStr(
      &Ipv6AddressData.Ip6AddressParam.Ipv6Address,
      CharPtrString,
      STRING_BUFFER_LENGTH);
    DEBUG((DEBUG_INFO, " IPMI : Ipv6 string: %S\n", CharPtrString));

    if (LanChannelNumber == gChannelNumberBuffer[0]) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN1_VAL),
        CharPtrString,
        NULL);

      UnicodeSPrint(
        CharPtrString,
        STRING_BUFFER_LENGTH,
        L"%d",
        Ipv6AddressData.Ip6AddressParam.AddressPrefixLen);

      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN1_PREFIX_LENGTH_VAL),
        CharPtrString,
        NULL);
    }

    if ((BMC_LAN_COUNT == 2) && (LanChannelNumber == gChannelNumberBuffer[1])) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN2_VAL),
        CharPtrString,
        NULL);

      UnicodeSPrint(CharPtrString,
        STRING_BUFFER_LENGTH,
        L"%d",
        Ipv6AddressData.Ip6AddressParam.AddressPrefixLen);

      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN2_PREFIX_LENGTH_VAL),
        CharPtrString,
        NULL);

    }
    gBS->FreePool(CharPtrString);
  }

  // Update the algorithm used.
  if (!IsIpv6Static) {
    if (Ipv6AddressData.Ip6AddressParam.AddressSource.Type == AMI_DHCP_SLAAC) {
      CharPtrString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN_DHCP_SLAAC),
        NULL);
    }
    else if (Ipv6AddressData.Ip6AddressParam.AddressSource.Type == AMI_DHCP_DHCP6) {
      CharPtrString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN_DHCP_DHCP6),
        NULL);
    }
    else {
      CharPtrString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_UNKNOWN_STRING),
        NULL);
    }
  }
  else {
    // For Static source algorithm is not defined.
    CharPtrString = HiiGetString(
      HiiHandle,
      STRING_TOKEN(STR_UNKNOWN_STRING),
      NULL);
  }

  if (LanChannelNumber == gChannelNumberBuffer[0]) {
    if (CharPtrString != NULL) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN1_DHCP_ALGO),
        CharPtrString,
        NULL);
    }
  }

  if ((BMC_LAN_COUNT == 2) && (LanChannelNumber == gChannelNumberBuffer[1])) {
    if (CharPtrString != NULL) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN2_DHCP_ALGO),
        CharPtrString,
        NULL);
    }
  }
  if (CharPtrString != NULL) {
    gBS->FreePool(CharPtrString);
  }
  // Update the address status.
  if (Ipv6AddressData.Ip6AddressParam.AddressStatus > 5) {
    Ipv6StatusIndex = 5;
  }
  else {
    Ipv6StatusIndex = Ipv6AddressData.Ip6AddressParam.AddressStatus;
  }

  CharPtrString = HiiGetString(
    HiiHandle,
    Ipv6Status[Ipv6StatusIndex],
    NULL);
  if (LanChannelNumber == gChannelNumberBuffer[0]) {
    if (CharPtrString != NULL) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN1_ADD_STATUS),
        CharPtrString,
        NULL);
    }
  }

  if ((BMC_LAN_COUNT == 2) && (LanChannelNumber == gChannelNumberBuffer[1])) {
    if (CharPtrString != NULL) {
      HiiSetString(
        HiiHandle,
        STRING_TOKEN(STR_STATION_IP6_LAN2_ADD_STATUS),
        CharPtrString,
        NULL);
    }
  }
  if (CharPtrString != NULL) {
    gBS->FreePool(CharPtrString);
  }
  DEBUG((DEBUG_INFO, "%a Exiting...\n", __FUNCTION__));
}

/** @internal
    This function will validate the entered router prefix value.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Protocol error or unable to get setup
                                variable.
    @retval EFI_SUCCESS         IPv6 configuration details are updated.

**/

EFI_STATUS
BmcLanIpv6Router1PrefixValueCallbackFunction(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  SERVER_MGMT_SETUP_CONFIGURATION     ServerMgmtConfiguration;
  BOOLEAN                             BrowserDataStatus;
  EFI_STRING                          OutputString;
  EFI_STATUS                          Status = EFI_SUCCESS;
  UINTN                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);
  EFI_IPv6_ADDRESS                    Ipv6Address;
  CHAR16                              StaticIp6Lan[40];

  DEBUG((DEBUG_INFO, "%a Entered with Key: %x \n", __FUNCTION__, Key));

  // Check for the key and Return if Key value does not match.
  if ((Key != BMC_NETWORK_STATION_IP6_ROUTER1_PREFIX_VALUE_LAN1_KEY) && \
    (Key != BMC_NETWORK_STATION_IP6_ROUTER1_PREFIX_VALUE_LAN2_KEY)) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Callback function is called with Wrong Key Value. \
                                              Returning EFI_UNSUPPORTED\n"));
    return EFI_UNSUPPORTED;
  }

  // Get the call back parameters and verify the action
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load defaults \n"));
    return EFI_UNSUPPORTED;
  }
  else if (CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED) {
    DEBUG((DEBUG_INFO, "\n IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGED... so return EFI_SUCCESS\n"));
    return EFI_SUCCESS;
  }

  // Get Browser DATA
  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nIPMI: Status of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  if (Key == BMC_NETWORK_STATION_IP6_ROUTER1_PREFIX_VALUE_LAN1_KEY) {
    /* Validate Ipv6 Router Prefix Value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.RouterPrefixValue1,
      sizeof(ServerMgmtConfiguration.RouterPrefixValue1));

    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);

    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.RouterPrefixValue1);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_STATIC_PREFIX_VALUE),
        NULL);
    }
  }

#if (BMC_LAN_COUNT == 2)
  if (Key == BMC_NETWORK_STATION_IP6_ROUTER1_PREFIX_VALUE_LAN2_KEY) {
    /* Validate Ipv6 Router Prefix Value, for error case, display error message.
       Local copy is made to prevent ASSERT due to alignment
       checking in NetLibStrToIp6().*/
    CopyMem(
      (UINT8*)StaticIp6Lan,
      (UINT8*)ServerMgmtConfiguration.RouterPrefixValue2,
      sizeof(ServerMgmtConfiguration.RouterPrefixValue2));

    Status = NetLibStrToIp6(
      StaticIp6Lan,
      &Ipv6Address);

    if (EFI_ERROR(Status)) {
      SetZeroIp6(ServerMgmtConfiguration.RouterPrefixValue2);
      OutputString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_STATIC_PREFIX_VALUE),
        NULL);
    }
  }
#endif
  if (EFI_ERROR(Status)) {

    // POPUP

    // Set browser data.
    BrowserDataStatus = HiiSetBrowserData(
      &gEfiServerMgmtSetupVariableGuid,
      SERVER_MGMT_SETUP_VARIABLE_NAME,
      SelectionBufferSize,
      (UINT8*)&ServerMgmtConfiguration,
      NULL);
    DEBUG((DEBUG_INFO, "\nIPMI: Status of HiiSetBrowserData() = %d\n", BrowserDataStatus));
    if (!BrowserDataStatus) {
      return EFI_UNSUPPORTED;
    }
  }

  DEBUG((DEBUG_INFO, "%a Exiting... \n", __FUNCTION__));
  return EFI_SUCCESS;
}

/** @internal
    This function update Ipv6 Router LAN parameters data.

    @param[in] HiiHandle            Handle to HII database.
    @param[in] LanChannelNumber     Channel need to be updated.

    @return VOID    Nothing.

**/
VOID
UpdateIp6RouterParameters(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT8              LanChannelNumber)
{
  AMI_GET_LAN_CONFIG_PARAM_IPV6_ROUTER_CONTROL_RESPONSE               Ipv6RouterControlData;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_DYNAMIC_ROUTER_INFO_SET_RESPONSE      Ipv6DynamicRouterInfoSet;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_STATIC_ROUTER_IP_ADDRESS_RESPONSE     GetIpv6StaticRouter1AddressData;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_STATIC_ROUTER1_PREFIX_LENGTH_RESPONSE GetRouter1PrefixLength;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_STATIC_ROUTER1_PREFIX_VALUE_RESPONSE  GetRouter1PrefixValue;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_DYNAMIC_ROUTER_IP_ADDRESS_RESPONSE    GetIpv6DynamicRouterAddressData;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_DYNAMIC_ROUTER_PREFIX_LENGTH_RESPONSE GetDynamicRouterPrefixLength;
  AMI_GET_LAN_CONFIG_PARAM_IPV6_DYNAMIC_ROUTER_PREFIX_VALUE_RESPONSE  GetDynamicRouterPrefixValue;
  AMI_GET_LAN_CONFIG_PARAM_REQUEST                                    GetParamCommand;
  UINT8                                                               ResponseDataSize;
  EFI_STRING                                                          CharPtrString;
  EFI_STRING                                                          CharPtrString1;
  EFI_STRING                                                          CharPtrString2;
  EFI_STRING                                                          CharPtrString3;
  EFI_STATUS                                                          Status;

  DEBUG((DEBUG_INFO, "%a Entry...\n", __FUNCTION__));

  // Wait until Set In Progress field is cleared.
  Status = IpmiWaitSetInProgressClear(
    gIpmiTransport,
    LanChannelNumber);
  if (EFI_ERROR(Status)) {
    return;
  }

  // Get Ipv6 Router Address Config Control
  GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
  GetParamCommand.LanChannel.GetParam = 0;
  GetParamCommand.LanChannel.Reserved = 0;
  GetParamCommand.ParameterSelector = AmiIpmiIpv6RouterConfig;
  GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
  GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
  ResponseDataSize = sizeof(Ipv6RouterControlData);

  Status = gIpmiTransport->SendIpmiCommand(
    gIpmiTransport,
    IPMI_NETFN_TRANSPORT,
    AMI_BMC_LUN,
    IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
    (UINT8*)&GetParamCommand,
    sizeof(GetParamCommand),
    (UINT8*)&Ipv6RouterControlData,
    &ResponseDataSize);

  DEBUG((DEBUG_INFO, " IPMI : Get Lan Configuration IPv6 Router Address Control Data Status: %r\n", Status));

  if (EFI_ERROR(Status)) {
    return;
  }

  DEBUG((DEBUG_INFO, " IPMI :RouterControlData =%x\n", Ipv6RouterControlData.Ip6RouterControlData));
  if (Ipv6RouterControlData.Ip6RouterControlData != IpmiUnspecified) {

    if ((Ipv6RouterControlData.Ip6RouterControlData == IpmiStaticAddrsss) || \
      (Ipv6RouterControlData.Ip6RouterControlData == ENABLED_STATIC_AND_DYNAMIC)) {

      CharPtrString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_IPSOURCE_STATIC_ADDRESS),
        NULL);

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        LanChannelNumber);

      if (EFI_ERROR(Status)) {
        return;
      }

      // Get IPV6 Static Router IP address.
      GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
      GetParamCommand.LanChannel.GetParam = 0;
      GetParamCommand.LanChannel.Reserved = 0;
      GetParamCommand.ParameterSelector = AmiIpmiIpv6StaticRouter1IpAddr;
      GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
      GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
      ResponseDataSize = sizeof(GetIpv6StaticRouter1AddressData);

      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&GetParamCommand,
        sizeof(GetParamCommand),
        (UINT8*)&GetIpv6StaticRouter1AddressData,
        &ResponseDataSize);

      DEBUG((DEBUG_INFO, " IPMI : Get Lan configuration Ipv6 Static Router IPAddr Status: %r\n", Status));

      if (EFI_ERROR(Status)) {
        return;
      }

      CharPtrString1 = AllocateZeroPool(STRING_BUFFER_LENGTH);
      if (CharPtrString1 != NULL) {
        NetLibIp6ToStr(
          &GetIpv6StaticRouter1AddressData.Ip6Address,
          CharPtrString1,
          STRING_BUFFER_LENGTH);
        DEBUG((DEBUG_INFO, " IPMI : Ipv6 Static Router IP string: %S \n", CharPtrString1));
      }

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        LanChannelNumber);

      if (EFI_ERROR(Status)) {
        return;
      }

      // Get IPV6 Static Router Prefix Length.
      GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
      GetParamCommand.LanChannel.GetParam = 0;
      GetParamCommand.LanChannel.Reserved = 0;
      GetParamCommand.ParameterSelector = AmiIpmiIpv6StaticRouter1PrefixLength;
      GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
      GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
      ResponseDataSize = sizeof(GetRouter1PrefixLength);

      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&GetParamCommand,
        sizeof(GetParamCommand),
        (UINT8*)&GetRouter1PrefixLength,
        &ResponseDataSize);

      DEBUG((DEBUG_INFO, " IPMI : Get Lan configuration Ipv6 Static Router Prefix Length Status: %r\n", Status));

      if (EFI_ERROR(Status)) {
        return;
      }

      CharPtrString2 = AllocateZeroPool(STRING_BUFFER_LENGTH);
      if (CharPtrString2 != NULL) {
        UnicodeSPrint(
          CharPtrString2,
          STRING_BUFFER_LENGTH,
          L"%d",
          GetRouter1PrefixLength.Ip6RouterPrefixLength);
        DEBUG((DEBUG_INFO, " IPMI : Ipv6 Static Router Prefix Length string: %S \n", CharPtrString2));
      }

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        LanChannelNumber);

      if (EFI_ERROR(Status)) {
        return;
      }

      // Get IPV6 Static Router Prefix Value.
      GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
      GetParamCommand.LanChannel.GetParam = 0;
      GetParamCommand.LanChannel.Reserved = 0;
      GetParamCommand.ParameterSelector = AmiIpmiIpv6StaticRouter1PrefixValue;
      GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
      GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
      ResponseDataSize = sizeof(GetRouter1PrefixValue);

      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&GetParamCommand,
        sizeof(GetParamCommand),
        (UINT8*)&GetRouter1PrefixValue,
        &ResponseDataSize);
      DEBUG((DEBUG_INFO, " IPMI : Get Lan configuration Ipv6 Static Router Prefix Value Status: %r\n", Status));

      if (EFI_ERROR(Status)) {
        return;
      }

      CharPtrString3 = AllocateZeroPool(STRING_BUFFER_LENGTH);
      if (CharPtrString3 != NULL) {
        NetLibIp6ToStr(
          &GetRouter1PrefixValue.IpPrefixValue,
          CharPtrString3,
          STRING_BUFFER_LENGTH);
        DEBUG((DEBUG_INFO, " IPMI : Ipv6 Static Router Prefix Value string: %S \n", CharPtrString3));
      }
    }
    else if (Ipv6RouterControlData.Ip6RouterControlData == IpmiDynamicAddressBmcDhcp) {

      // Wait until Set In Progress field is cleared.
      Status = IpmiWaitSetInProgressClear(
        gIpmiTransport,
        LanChannelNumber);
      if (EFI_ERROR(Status)) {
        return;
      }

      // Get IPV6 address.
      GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
      GetParamCommand.LanChannel.GetParam = 0;
      GetParamCommand.LanChannel.Reserved = 0;
      GetParamCommand.ParameterSelector = AmiIpmiIpv6DynamicRouterInfoSet;
      GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
      GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
      ResponseDataSize = sizeof(Ipv6DynamicRouterInfoSet);

      Status = gIpmiTransport->SendIpmiCommand(
        gIpmiTransport,
        IPMI_NETFN_TRANSPORT,
        AMI_BMC_LUN,
        IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
        (UINT8*)&GetParamCommand,
        sizeof(GetParamCommand),
        (UINT8*)&Ipv6DynamicRouterInfoSet,
        &ResponseDataSize);

      DEBUG((DEBUG_INFO, " IPMI : Number of Ipv6DynamicRouterInfoSet = %x\n", Ipv6DynamicRouterInfoSet.Ip6DynamicRouterInfoSet));
      if (Ipv6DynamicRouterInfoSet.Ip6DynamicRouterInfoSet != 0) {

        CharPtrString = HiiGetString(
          HiiHandle,
          STRING_TOKEN(STR_IPSOURCE_DYNAMIC_ADDRESS_BMC_DHCP),
          NULL);

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          LanChannelNumber);
        if (EFI_ERROR(Status)) {
          return;
        }

        // Get IPV6 Dynamic Router IP Address.
        GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
        GetParamCommand.LanChannel.GetParam = 0;
        GetParamCommand.LanChannel.Reserved = 0;
        GetParamCommand.ParameterSelector = AmiIpmiIpv6DynamicRouterIpAddr;
        GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
        GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
        ResponseDataSize = sizeof(GetIpv6DynamicRouterAddressData);

        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetParamCommand,
          sizeof(GetParamCommand),
          (UINT8*)&GetIpv6DynamicRouterAddressData,
          &ResponseDataSize);

        DEBUG((DEBUG_INFO, " IPMI : Get Ipv6 Dynamic Router IpAddr Status: %r\n", Status));
        if (EFI_ERROR(Status)) {
          return;
        }

        CharPtrString1 = AllocateZeroPool(STRING_BUFFER_LENGTH);
        if (CharPtrString1 != NULL) {
          NetLibIp6ToStr(
            &GetIpv6DynamicRouterAddressData.Ip6AddressParam.Ipv6Address,
            CharPtrString1,
            STRING_BUFFER_LENGTH);
          DEBUG((DEBUG_INFO, " IPMI : Ipv6 Dynamic Router Info IAddrP string: %S \n", CharPtrString1));
        }

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          LanChannelNumber);
        if (EFI_ERROR(Status)) {
          return;
        }

        // Get IPV6 Dynamic Router Prefix Length.
        GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
        GetParamCommand.LanChannel.GetParam = 0;
        GetParamCommand.LanChannel.Reserved = 0;
        GetParamCommand.ParameterSelector = AmiIpmiIpv6DynamicRouterInfoPrefixLength;
        GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
        GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
        ResponseDataSize = sizeof(GetDynamicRouterPrefixLength);

        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetParamCommand,
          sizeof(GetParamCommand),
          (UINT8*)&GetDynamicRouterPrefixLength,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, " IPMI : Get Ipv6 Dynamic Router Info Prefix Length Status: %r\n", Status));

        if (EFI_ERROR(Status)) {
          return;
        }

        CharPtrString2 = AllocateZeroPool(STRING_BUFFER_LENGTH);
        if (CharPtrString2 != NULL) {
          UnicodeSPrint(
            CharPtrString2,
            STRING_BUFFER_LENGTH,
            L"%d",
            GetDynamicRouterPrefixLength.IpDynamicRouterPrefixLength.PrefixLength);
          DEBUG((DEBUG_INFO, " IPMI : Ipv6 Dynamic Router Info Prefix Length string: %S \n", CharPtrString2));
        }

        // Wait until Set In Progress field is cleared.
        Status = IpmiWaitSetInProgressClear(
          gIpmiTransport,
          LanChannelNumber);
        if (EFI_ERROR(Status)) {
          return;
        }

        // Get IPV6 Dynamic Router Prefix Value.
        GetParamCommand.LanChannel.ChannelNumber = LanChannelNumber;
        GetParamCommand.LanChannel.GetParam = 0;
        GetParamCommand.LanChannel.Reserved = 0;
        GetParamCommand.ParameterSelector = AmiIpmiIpv6DynamicRouterInfoPrefixValue;
        GetParamCommand.BlockSelector = AMI_IPMI_SELECTOR_NONE;
        GetParamCommand.SetSelector = AMI_IPMI_SELECTOR_NONE;
        ResponseDataSize = sizeof(GetDynamicRouterPrefixValue);

        Status = gIpmiTransport->SendIpmiCommand(
          gIpmiTransport,
          IPMI_NETFN_TRANSPORT,
          AMI_BMC_LUN,
          IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
          (UINT8*)&GetParamCommand,
          sizeof(GetParamCommand),
          (UINT8*)&GetDynamicRouterPrefixValue,
          &ResponseDataSize);
        DEBUG((DEBUG_INFO, " IPMI : Get Ipv6 Dynamic Router Info Prefix value Status: %r\n", Status));
        if (EFI_ERROR(Status)) {
          return;
        }

        CharPtrString3 = AllocateZeroPool(STRING_BUFFER_LENGTH);
        if (CharPtrString3 != NULL) {
          NetLibIp6ToStr(
            &GetDynamicRouterPrefixValue.IpDynamicRouterPrefixValue.Ipv6Address,
            CharPtrString3,
            STRING_BUFFER_LENGTH);
          DEBUG((DEBUG_INFO, " IPMI : Ipv6 Dynamic Router Info Prefix Value string: %S \n", CharPtrString3));
        }
      }
      else {
        CharPtrString = HiiGetString(
          HiiHandle,
          STRING_TOKEN(STR_IPSOURCE_UNSUPPORTED_ROUTER_INFO_SET),
          NULL);

        CharPtrString1 = HiiGetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_ADDRESS_VAL),
          NULL); \

          CharPtrString2 = HiiGetString(
            HiiHandle,
            STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_LENGTH_VAL),
            NULL);

        CharPtrString3 = HiiGetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_VALUE),
          NULL);
      }
    }

    if (LanChannelNumber == gChannelNumberBuffer[0]) {
      if (CharPtrString != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_IP6_CURR_ROUTER_LANCAS1_VAL),
          CharPtrString,
          NULL);
      }

      if (CharPtrString1 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_ADDRESS1_VAL),
          CharPtrString1,
          NULL);
      }

      if (CharPtrString2 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_LENGTH1_VAL),
          CharPtrString2,
          NULL);
      }

      if (CharPtrString3 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_VALUE1),
          CharPtrString3,
          NULL);
      }
    }

    if ((BMC_LAN_COUNT == 2) && (LanChannelNumber == gChannelNumberBuffer[1])) {
      if (CharPtrString != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_IP6_CURR_ROUTER_LANCAS2_VAL),
          CharPtrString,
          NULL);
      }

      if (CharPtrString1 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_ADDRESS2_VAL),
          CharPtrString1,
          NULL);
      }

      if (CharPtrString2 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_LENGTH2_VAL),
          CharPtrString2,
          NULL);
      }
      if (CharPtrString3 != NULL) {
        HiiSetString(
          HiiHandle,
          STRING_TOKEN(STR_STATION_IP6_ROUTER_PREFIX_VALUE2),
          CharPtrString3,
          NULL);
      }
    }

    if (CharPtrString != NULL)
      FreePool(CharPtrString);

    if (CharPtrString1 != NULL)
      FreePool(CharPtrString1);

    if (CharPtrString2 != NULL)
      FreePool(CharPtrString2);

    if (CharPtrString3 != NULL)
      FreePool(CharPtrString3);
  }
}

/**
    Reads the Ipv4 and Ipv6 parameters from BMC and updates in Setup

    @param  HiiHandle - Handle to HII database
    @param  Class - Indicates the setup class

    @retval VOID

**/
#if (BmcLanConfig_Live_status == 0)
EFI_STATUS
BmcLanParamSetupDisplay(
  IN EFI_HII_HANDLE HiiHandle)
{

  EFI_STATUS                         Status;
  SERVER_MGMT_SETUP_CONFIGURATION     ServerMgmtConfiguration;
  UINTN                              Size;
  UINT32                             Attributes;

  // Check for valid Lan channel.
  if (!gChannelNumberBuffer[0]) {
    Status = AmiInitgVariables();

    if (EFI_ERROR(Status))
      return EFI_UNSUPPORTED;
  }
  //Updates Ipv4 Parameters

  UpdateIp4Parameters(HiiHandle);

  //
  // Get ServerSetup Variable to find if Ipv6 supported or not. 
  //
  Size = sizeof(ServerMgmtConfiguration);
  Status = gRT->GetVariable(
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    &gEfiServerMgmtSetupVariableGuid,
    &Attributes,
    &Size,
    &ServerMgmtConfiguration);
  DEBUG((DEBUG_INFO, "IPMI : Status of ServerSetup GetVariable() = %r\n", Status));

  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  if (gChannelNumberBuffer[0] != 0) {
    if (ServerMgmtConfiguration.Ip6Support1 == IPV6_SUPPORTED) {
      //Updates Ipv6 Parameters
      UpdateIp6Parameters(
        HiiHandle,
        gChannelNumberBuffer[0]);

      UpdateIp6RouterParameters(
        HiiHandle,
        gChannelNumberBuffer[0]);
    }
  }

#if BMC_LAN_COUNT == 2
  if (gChannelNumberBuffer[1] != 0) {
    if (ServerMgmtConfiguration.Ip6Support2 == IPV6_SUPPORTED) {
      UpdateIp6Parameters(
        HiiHandle,
        gChannelNumberBuffer[1]);

      UpdateIp6RouterParameters(
        HiiHandle,
        gChannelNumberBuffer[1]);
    }
  }
#endif

  DEBUG((DEBUG_INFO, "%a Exiting...\n", __FUNCTION__));
  return EFI_SUCCESS;
}
#endif
/**
    Reads Ipv6 parameters from BMC and displays in setup.

    @param[in] HiiHandle    Handle to HII database.
    @param[in] Class        Formset Class.
    @param[in] SubClass     Formset Subclass.
    @param[in] Key          Formset Key.

    @retval EFI_UNSUPPORTED     Protocol error or unable to get setup
                                variable.
    @retval EFI_SUCCESS         IPv6 configuration details are updated.

**/
#if (BmcLanConfig_Live_status)
EFI_STATUS
BmcLanParamSetupDisplayCallback(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  EFI_STATUS                          Status;
  SERVER_MGMT_SETUP_CONFIGURATION      ServerMgmtConfiguration;
  UINTN                               Size;
  UINT32                              Attributes;

  // Get the call back parameters and verify the action.
  if ((CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGING) && (CallbackParameters->Action != EFI_BROWSER_ACTION_RETRIEVE)) {
    DEBUG((DEBUG_INFO, "IPMI: CallbackParameters->Action != EFI_BROWSER_ACTION_CHANGING and CallbackParameters->Action != EFI_BROWSER_ACTION_RETRIEVE ...so return EFI_UNSUPPORTED\n"));
    return  EFI_UNSUPPORTED;
  }

  // Check for valid Lan channel.
  if (!gChannelNumberBuffer[0]) {
    Status = AmiInitgVariables();

    if (EFI_ERROR(Status))
      return EFI_UNSUPPORTED;
  }
  //Updates Ipv4 Parameters

  UpdateIp4Parameters(HiiHandle);

  // Get ServerSetup Variable to find if Ipv6 supported or not.
  Size = sizeof(ServerMgmtConfiguration);
  Status = gRT->GetVariable(
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    &gEfiServerMgmtSetupVariableGuid,
    &Attributes,
    &Size,
    &ServerMgmtConfiguration);

  DEBUG((DEBUG_INFO, "IPMI : Status of ServerSetup GetVariable() = %r\n", Status));

  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  if (ServerMgmtConfiguration.Ip6Support1 == IPV6_SUPPORTED) {
    UpdateIp6Parameters(
      HiiHandle,
      gChannelNumberBuffer[0]);

    UpdateIp6RouterParameters(
      HiiHandle,
      gChannelNumberBuffer[0]);
  }
#if (BMC_LAN_COUNT == 2) 
  if (ServerMgmtConfiguration.Ip6Support2 == IPV6_SUPPORTED) {
    UpdateIp6Parameters(
      HiiHandle,
      gChannelNumberBuffer[1]);

    UpdateIp6RouterParameters(
      HiiHandle,
      gChannelNumberBuffer[1]);
  }
#endif

  DEBUG((DEBUG_INFO, "%a Exiting...\n", __FUNCTION__));
  return EFI_SUCCESS;
}
#endif

/**
    This function will validate the static data for both IPv6 and IPv4 configuration
    and display the Error message, if the user didn't set the static data.

    @param  HiiHandle A handle that was previously registered in the
                      HII Database.
    @param  Class     Form set Class of the Form Callback Protocol passed in

    @param  SubClass  Form set sub Class of the Form Callback Protocol passed in
    @param  Key       Form set Key of the Form Callback Protocol passed in

    @return EFI_STATUS Return Status

**/

EFI_STATUS
BmcLanIPValidation(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters)
{
  EFI_STRING                          PopUpString;
  EFI_STRING                          IPv4DefaultIPStr;
  EFI_STRING                          IPv6DefaultIPStr;
  EFI_STRING                          IPv4DefaultSubnetMaskStr;
  BOOLEAN                             BrowserDataStatus;
  SERVER_MGMT_SETUP_CONFIGURATION      ServerMgmtConfiguration;
  CHAR16                              OutputString[EFI_PAGE_SIZE];
  UINTN                               SelectionBufferSize = sizeof(SERVER_MGMT_SETUP_CONFIGURATION);


  // Get the call back parameters and verify the action
  DEBUG((DEBUG_INFO, "%a Entered with Key: %x  \n", __FUNCTION__, Key));
  if (CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    DEBUG((DEBUG_ERROR, "\n IPMI: CallbackParameters->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD... so load the defaults \n"));
    return EFI_UNSUPPORTED;
  }
  else if ((CallbackParameters->Action != EFI_BROWSER_ACTION_FORM_OPEN) && (CallbackParameters->Action != EFI_BROWSER_ACTION_FORM_CLOSE)) {
    DEBUG((DEBUG_INFO, "\n IPMI:CallbackParameters->Action != EFI_BROWSER_ACTION_FORM_OPEN\n"));
    return EFI_SUCCESS;
  }

  BrowserDataStatus = HiiGetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration);

  if (!BrowserDataStatus) {
    DEBUG((DEBUG_ERROR, "\nStatus of HiiGetBrowserData() = %d\n", BrowserDataStatus));
    return EFI_UNSUPPORTED;
  }

  ZeroMem(OutputString, EFI_PAGE_SIZE);

  IPv4DefaultIPStr = HiiGetString(
    HiiHandle,
    STRING_TOKEN(STR_STATION_IP_ADDRESS_DEFAULT),
    NULL);

  IPv4DefaultSubnetMaskStr = HiiGetString(
    HiiHandle,
    STRING_TOKEN(STR_SUBNET_MASK_DEFAULT),
    NULL);

  IPv6DefaultIPStr = HiiGetString(
    HiiHandle,
    STRING_TOKEN(STR_STATION_IP6_ADDRESS_DEFAULT),
    NULL);

  // IPv4 channel 1 validation.
  if (ServerMgmtConfiguration.BmcLan1 == IpmiStaticAddrsss) {

    // Checking for IPv4 station IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StationIp1, (CHAR8*)IPv4DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVAID_LAN1_IPV4_STATION_IP),
        NULL);
      StrCpyS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);

      ServerMgmtConfiguration.BmcLan1 = IpmiUnspecified;
      FreePool(PopUpString);
    }
    else

    // Checking for Sub-Net Mask
    if ((AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.Subnet1, (CHAR8*)IPv4DefaultSubnetMaskStr) == 0)) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_LAN1_SUBNETMASK),
        NULL);
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);

      ServerMgmtConfiguration.BmcLan1 = IpmiUnspecified;
      FreePool(PopUpString);
    }
  }

  // IPv6 Channel 1 Validation
  if ((ServerMgmtConfiguration.Ip6Support1 == IPV6_SUPPORTED) &&
    (ServerMgmtConfiguration.Ip6BmcLan1 == IpmiStaticAddrsss)) {

    // Checking for IPv6 Station IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StaticIp6Lan1, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN1_STATION_IP),
        NULL);

      ServerMgmtConfiguration.Ip6BmcLan1 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }
  }

  if ((ServerMgmtConfiguration.Ip6Support1 == IPV6_SUPPORTED) &&
    (ServerMgmtConfiguration.Ip6RouterLan1 == IpmiStaticAddrsss)) {

    // Checking for IPv6 Router IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StaticIp6Lan1Router1Ip, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN1_ROUTER_IP),
        NULL);

      ServerMgmtConfiguration.Ip6RouterLan1 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }

    // Checking for IPv6 Router Prefix Value.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.RouterPrefixValue1, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN1_ROUTER_PREFIX_VALUE),
        NULL);

      ServerMgmtConfiguration.Ip6RouterLan1 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }
  }

#if (BMC_LAN_COUNT == 2)  

  // IPv4 channel 2 Validation.
  if (ServerMgmtConfiguration.BmcLan2 == IpmiStaticAddrsss) {
    // Checking for IPv4 IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StationIp2, (CHAR8*)IPv4DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_LAN2_IPV4_STATION_IP),
        NULL);
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);

      ServerMgmtConfiguration.BmcLan2 = IpmiUnspecified;
      FreePool(PopUpString);
    }
    // Checking for Sub-Net Mask
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.Subnet2, (CHAR8*)IPv4DefaultSubnetMaskStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_LAN2_SUBNETMASK),
        NULL);

      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      ServerMgmtConfiguration.BmcLan2 = IpmiUnspecified;
      FreePool(PopUpString);
    }
  }

  // IPv6 channel 2 Validation.
  if ((ServerMgmtConfiguration.Ip6Support2 == IPV6_SUPPORTED) &&
    (ServerMgmtConfiguration.Ip6BmcLan2 == IpmiStaticAddrsss)) {

    // Checking for IPv6 Station IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StaticIp6Lan2, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN1_STATION_IP),
        NULL);

      ServerMgmtConfiguration.Ip6BmcLan2 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }
  }

  if ((ServerMgmtConfiguration.Ip6Support2 == IPV6_SUPPORTED) &&
    (ServerMgmtConfiguration.Ip6RouterLan2 == IpmiStaticAddrsss)) {

    // Checking for IPv6 Router IP address.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.StaticIp6Lan2Router1Ip, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN2_ROUTER_IP),
        NULL);

      ServerMgmtConfiguration.Ip6RouterLan2 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }

    // Checking for IPv6 Router Prefix Value.
    if (AsciiStrCmp((CHAR8*)ServerMgmtConfiguration.RouterPrefixValue2, (CHAR8*)IPv6DefaultIPStr) == 0) {

      PopUpString = HiiGetString(
        HiiHandle,
        STRING_TOKEN(STR_INVALID_IPV6_LAN2_ROUTER_PREFIX_VALUE),
        NULL);

      ServerMgmtConfiguration.Ip6RouterLan2 = IpmiUnspecified;
      StrCatS(OutputString, sizeof(OutputString) / sizeof(CHAR16), PopUpString);
      FreePool(PopUpString);
    }
  }
#endif

  FreePool(IPv4DefaultIPStr);
  FreePool(IPv6DefaultIPStr);
  FreePool(IPv4DefaultSubnetMaskStr);

  if ((*OutputString) == 0) {
    return EFI_SUCCESS;
  }

  // POPUP

  CHAR16* ContinueString = HiiGetString(
    HiiHandle,
    STRING_TOKEN(STR_PRESS_ENTER_TO_CONTINUE),
    NULL);
  CHAR16* CharPtrString = HiiGetString(
    HiiHandle,
    STRING_TOKEN(STR_BMCLAN_ERROR_INFO),
    NULL);
  if ((OutputString != NULL) && (CharPtrString != NULL)) {
    PrintPopup(
      CharPtrString,
      OutputString,
      ContinueString
    );
    gBS->FreePool(CharPtrString);
    gBS->FreePool(ContinueString);
  }

  // Set browser data.
  BrowserDataStatus = HiiSetBrowserData(
    &gEfiServerMgmtSetupVariableGuid,
    SERVER_MGMT_SETUP_VARIABLE_NAME,
    SelectionBufferSize,
    (UINT8*)&ServerMgmtConfiguration,
    NULL);
  DEBUG((DEBUG_INFO, "\nIPMI: Status of HiiSetBrowserData() = %d\n", BrowserDataStatus));
  if (!BrowserDataStatus) {
    return EFI_UNSUPPORTED;
  }

  DEBUG((DEBUG_INFO, "%a Exit \n", __FUNCTION__));
  return EFI_SUCCESS;
}
