//***********************************************************************
//*                                                                     *
//*   Copyright (c) 1985-2020, American Megatrends International LLC.   *
//*                                                                     *
//*      All rights reserved. Subject to AMI licensing agreement.       *
//*                                                                     *
//***********************************************************************


/** @file BmcLanConfigCommon.h
    Contains headers and declarations for BMC network configuration.

*/

#ifndef _BMC_LAN_CONFIG_COMMON_H_
#define _BMC_LAN_CONFIG_COMMON_H_

//---------------------------------------------------------------------------

#include <Uefi.h>
#include <AmiIpmiNetFnTransportDefinitions.h>
#include <Guid/ServerMgmtSetupVariable.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/IpmiTransportProtocol.h>
#include <Library/AmiIpmiLib.h>

//---------------------------------------------------------------------------

#pragma pack(1)
/** @internal
    Bmc IPv4 bios settings.
*/
typedef struct {
    /// Bmc lan channel
    UINT8     BmcLan;
    UINT8     StationIp[4];     ///< Station Ip address.
    UINT8     Subnet[4];        ///< Subnet address.
    UINT8     RouterIp[4];      ///< Router Ip address.
    UINT8     RouterMac[6];     ///< Router Mac address.
} BMC_IP_BIOS_SETTINGS;
#pragma pack()

//---------------------------------------------------------------------------

/**
    It will return the String length of the unicode array.

    @param[in] String   It holds the base address of the array.

    @return UINTN   Returns length of the char array.

**/
UINTN
EfiStrLen (
  IN CHAR16   *String
);

/** @internal
    Validate the IP address, and also convert the string IP to decimal
    value. If the IP is invalid then 0 IP is entered.

    The Source string is parsed from right to left with
    following rules:
    1) No characters other than numeral and dot is allowed.
    2) The first right most letter should not be a dot.
    3) No consecutive dot allowed.
    4) Values greater than 255 not allowed.
    5) Not more than four parts allowed.

    @param[out] Destination  Pointer contains validated IP address in
                             decimal system.
    @param[in] Source        Pointer to string IP to be validated.

    @retval EFI_SUCCESS             IP address is validated and converted to
                                    decimal successfully.
    @retval EFI_INVALID_PARAMETER   Given IP address is invalid.

    @note  Function variables description.

        Index           - Counter for for loop and store the length
                          of the source array initially.
        LookingIp       - IP address is taken into four parts, one
                          part by one will be extracted from the
                          string and saved to Destination variable.
                          LookingIp variable contains which IP part
                          currently we are extracting.
        SumIp           - Contains sum of the digit of an IP address
                          multiplied by its base power. SumIp=
                          (unit digit * 1) + (Tenth digit * 10) +
                          (hundredth digit * 100).
        IpDigit         - Base power of the digit we are extracting
        DigitCount      - Digit number - 1 we are extracting in a 
                          IP part.
        IsIpValid       - Flag to set if an invalid IP is entered
                          and break the loop.
        GotTheFirstDigit- Flag is set when the first decimal value is
                          found in the string,So if a dot is 
                          encountered first, immediately the loop can
                          be terminated.
        TotalSumIp      - Sum of all 4 SumIp part of an IP address.
                          this variable is used to avoid 
                          000.000.000.000 IP case in the
                          BmcLanConfiguration.intial value is set to
                          zero.if sum of all 4 SumIp part of an IP
                          address is zero then that is invalid
                          input.

**/
EFI_STATUS
ConvertToIpAddress (
  OUT UINT8     *Destination,
  IN  CHAR16    *Source
);

/** @internal
    Validate the MAC address and convert the string MAC to decimal value.
    If the MAC is invalid then 0 MAC is entered.

    The Source string is parsed from right to left with following rules.

    1) No characters other than numeral, alphabets a-f, A-F and
       - is allowed.
    2) The first right most letter should not be a -.
    3) No consecutive - allowed.
    4) Not more than six parts allowed.

    @param[out] Destination     Pointer contains validated MAC address in
                                decimal system.
    @param[in]  Source          Pointer to string MAC to be validated.

    @retval EFI_SUCCESS             MAC address is validated and converted
                                    to decimal successfully.
    @retval EFI_INVALID_PARAMETER   Given MAC address is invalid.

    @note  Function variables description.
  
        Index           - Counter for loop.
        LookingMac      - MAC address is taken into six parts, one part
                          by one will be extracted from the string and
                          saved to Destination variable.
                          LookingMac variable contains which MAC part
                          currently we are extracting.
        SumMac          - Contains sum of the digit of an MAC address
                          multiplied by its base power. SumMac=
                          (unit digit * 1) + (Tenth digit * 16)
        MacDigit        - Base power of the digit we are extracting
        DigitCount      - Digit number - 1 we are extracting in a MAC
                          part.
        IsMacValid      - Flag to set if an invalid MAC is entered and
                          break the loop.
        GotTheFirstDigit- Flag is set when the first hex value is
                          found in the string,So if a - is 
                          encountered first,immediately the loop
                          can be terminated.
        TmpValue        - Used to convert all small letters to capital
                          letters.
        TotalSumMac     - Sum of all 6 SumMac part of an MAC address.
                          this variable is used to avoid 
                          00-00-00-00-00-00 MAC case in the
                          BmcLanConfiguration.intial value is set to 
                          zero.if sum of all 6 SumMac is zero then 
                          that is invalid MAC.

**/
EFI_STATUS
ConvertToMacAddress (
  OUT UINT8     *Destination,
  IN  CHAR16    *Source
);

/** @internal
    Set IP address to ... to memory pointed by ZeroIp.

    @param[out] DefautIp  Pointer to IP address in BIOS setup variable.

    @return VOID          Nothing.

**/
VOID
SetDefaultIp (
  OUT CHAR16    *ZeroIp
);

/** @internal
    Set IP address to 0.0.0.0 to memory pointed by ZeroIp.

    @param[out] ZeroIp  Pointer to IP address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroIp (
  OUT CHAR16    *ZeroIp
);

/** @internal
    Set MAC address to 00-00-00-00-00-00 to memory pointed by ZeroMac.

    @param[out] ZeroMac     Pointer to MAC address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroMac (
  OUT CHAR16    *ZeroMac
);

/** @internal
    Set IP address to ::::::: for memory pointed by ZeroIp.

    @param[out] ZeroIp  Pointer to Ip address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroIp6 (
  OUT CHAR16    *ZeroIp6
);

/** @internal
    Checks for Set-In Progress Bit and Wait to get it Clear.

    @param[in] IpmiTransport        Pointer to Ipmi transport instance.
    @param[in] LanChannelNumber     Channel Number of LAN.

    @retval EFI_SUCCESS     Set-In Progress Bit is Cleared.
    @retval EFI_TIMEOUT     Specified Time out over.
    @retval Others          Error status returned by send ipmi command.

**/
EFI_STATUS
IpmiWaitSetInProgressClear (
  IN AMI_IPMI_TRANSPORT_INSTANCE    *IpmiTransport,
  IN UINT8                          LanChannelNumber
);
#endif


