//***********************************************************************
//*                                                                     *
//*   Copyright (c) 1985-2020, American Megatrends International LLC.   *
//*                                                                     *
//*      All rights reserved. Subject to AMI licensing agreement.       *
//*                                                                     *
//***********************************************************************


/** @file BmcLanConfigCommon.c
    Contains common functions for BMC network configuration.

*/

//---------------------------------------------------------------------------

#include "BmcLanConfigCommon.h"

//---------------------------------------------------------------------------

/**
    It will return the String length of the unicode array.

    @param[in] String   It holds the base address of the array.

    @return UINTN   Returns length of the char array.

**/
UINTN
EfiStrLen (
  IN CHAR16   *String )
{
    UINTN Length = 0;

    while (*String++) {
        Length++;
    }

    return Length;
}

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
  IN  CHAR16    *Source )
{
    INTN       Index;
    INT8       LookingIp = 3;
    UINT8      SumIp = 0;
    UINT8      IpDigit = 1;
    UINT8      DigitCount = 0;
    UINT8      IsIpValid = 1;
    UINT8      GotTheFirstDigit = 0;
    UINT16     TotalSumIp = 0;

    Index = (INTN) EfiStrLen(Source);

    /* Assigning index = 15 because it is not returning 15 as strlen if ip
       is like xxx.xxx.xxx.xxx this.*/
    if (Index > 15) {
        Index = 15;
    }

    for (Index = Index - 1; Index >= 0; Index--) {

        // Check if the value should be between 0 and 9.
        if ((Source[Index] <= 57) && (Source[Index] >= 48)) {

            GotTheFirstDigit = 1;

            // Check if DigitCount should not be more than 3 (including 0). Else Ip is invalid.
            if (DigitCount > 2) {
                IsIpValid = 0;
                break;
            }

            // DigitCount is 3.
            if (DigitCount == 2) {
                /* Check if the 3rd digit should not be more than 2. Else Ip is invalid.
                   Example: 255. 2 is the 3rd digit.*/
                if (Source[Index] - 48 >2) {
                    IsIpValid = 0;
                    break;
                }

                // Check if the Ip address part is not having value more than 255. Else Ip is invalid.
                if (Source[Index] - 48 == 2 && ((Source[Index + 1] - 48 > 5) || \
                    (Source[Index + 1] - 48 == 5 && Source[Index + 2] - 48 > 5))) {
                    IsIpValid = 0;
                    break;
                }
            }

            SumIp = SumIp + ((UINT8)Source[Index] - 48) * IpDigit;
            IpDigit = IpDigit * 10;
            DigitCount = DigitCount + 1;
        } else if (Source[Index] == 46) { // Check for '.'

            // If the first digit in Ip address part is '.', then Ip is invalid.
            if ((GotTheFirstDigit == 0) || (Source[Index + 1] == 46)) {
                IsIpValid = 0;
                break;
            }
            // Else store the valid Ip address part done till now.
            Destination[LookingIp] = SumIp;
            TotalSumIp = TotalSumIp + SumIp;
            
            LookingIp--;
            
            // If the Ip address have more than 4 Ip address parts then Ip is invalid.
            if(LookingIp < 0) {
                IsIpValid = 0;
                break;
            }
            SumIp = 0;
            IpDigit = 1;
            DigitCount = 0;
        } else if ((Source[Index] != 0) || \
                    ((Source[Index] == 0) && \
                    (GotTheFirstDigit == 1))) {
            /* Check if Ip address part has values other than 0 to 9, '.' and after first digit of Ip
               part has 0 (NULL), then Ip is invalid*/
            IsIpValid = 0;
            break;
        }
    }

    // Store the final Ip address part.
    if (LookingIp == 0) {
        Destination[LookingIp] = SumIp;
        TotalSumIp = TotalSumIp + SumIp;
    }

    // Check for invalid Ip and return.
    if((LookingIp != 0) || (IsIpValid == 0) || (TotalSumIp == 0)) {
        Destination[0] = 0;
        Destination[1] = 0;
        Destination[2] = 0;
        Destination[3] = 0;
        return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}

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
  IN  CHAR16    *Source )
{
    INT8       Index;
    INT8       LookingMac = 5;
    UINT8      SumMac = 0;
    UINT8      MacDigit = 1;
    UINT8      DigitCount = 0;
    UINT8      IsMacValid = 1;
    UINT8      GotTheFirstDigit = 0;
    UINT8      TmpValue;
    UINT16     TotalSumMac = 0;

    for (Index = 16; Index >= 0; Index--) {
    
        /* Check if the Mac address part has values 0 to 9, A to F or a to f.*/
        if (((Source[Index] <= 57) && (Source[Index] >= 48)) ||
            ((Source[Index] >= 65) && (Source[Index] <= 70)) ||
            ((Source[Index] >= 97) && (Source[Index] <= 102))) {

            GotTheFirstDigit = 1;

            // Check if DigitCount should not be more than 2 (including 0). Else Mac is invalid.
            if (DigitCount > 1) {
                IsMacValid = 0;
                break;
            }

            TmpValue = (UINT8)Source[Index];
            // Check if the Mac address part has values a to f.
            if ((TmpValue >= 97) && (TmpValue <= 102)) {
                TmpValue = TmpValue - 32;
            }
            TmpValue = TmpValue - 48;

            if (TmpValue > 9) {
                TmpValue = TmpValue - 7;
            }

            SumMac = SumMac + (TmpValue * MacDigit);
            MacDigit = MacDigit * 16;
            DigitCount = DigitCount + 1;

        } else if (Source[Index] == 45) {  // Check for '-'

            // If the first digit in Mac address part is '-' then Mac is invalid.
            if ((GotTheFirstDigit == 0) || (Source[Index + 1] == 45)) {
                IsMacValid = 0;
                break;
            }
            Destination[LookingMac] = SumMac;
            TotalSumMac = TotalSumMac + SumMac;
            LookingMac--;

            // If the Mac address have more than 6 parts then Mac is invalid.
            if(LookingMac < 0) {
                IsMacValid = 0;
                break;
            }
            SumMac = 0;
            MacDigit = 1;
            DigitCount = 0;
        } else {
            IsMacValid = 0;
            break;
        }
    }

    // Store the final Mac address part.
    if (LookingMac == 0) {
        Destination[LookingMac] = SumMac;
        TotalSumMac = TotalSumMac + SumMac;
    }

    // Check for invalid Mac and return.
    if ((LookingMac != 0) || (IsMacValid == 0) || (TotalSumMac == 0)) {
        Destination[0] = 0;
        Destination[1] = 0;
        Destination[2] = 0;
        Destination[3] = 0;
        Destination[4] = 0;
        Destination[5] = 0;
        return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}
/** @internal
    Set IP address to ... to memory pointed by ZeroIp.

    @param[out] ZeroIp  Pointer to IP address in BIOS setup variable.

    @return VOID        Nothing.

**/
VOID
SetDefaultIp (
  OUT CHAR16    *ZeroIp )
{
    ZeroIp[0] = 46;
    ZeroIp[1] = 46;
    ZeroIp[2] = 46;
    ZeroIp[3] = 0;
    ZeroIp[4] = 0;
    ZeroIp[5] = 0;
    ZeroIp[6] = 0;
    ZeroIp[7] = 0;
    ZeroIp[8] = 0;
    ZeroIp[9] = 0;
    ZeroIp[10] = 0;
    ZeroIp[11] = 0;
    ZeroIp[12] = 0;
    ZeroIp[13] = 0;
    ZeroIp[14] = 0;
}

/** @internal
    Set IP address to 0.0.0.0 to memory pointed by ZeroIp.

    @param[out] ZeroIp  Pointer to IP address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroIp (
  OUT CHAR16    *ZeroIp )
{
    ZeroIp[0] = 48;
    ZeroIp[1] = 46;
    ZeroIp[2] = 48;
    ZeroIp[3] = 46;
    ZeroIp[4] = 48;
    ZeroIp[5] = 46;
    ZeroIp[6] = 48;
    ZeroIp[7] = 0;
    ZeroIp[8] = 0;
    ZeroIp[9] = 0;
    ZeroIp[10] = 0;
    ZeroIp[11] = 0;
    ZeroIp[12] = 0;
    ZeroIp[13] = 0;
    ZeroIp[14] = 0;
}

/** @internal
    Set MAC address to 00-00-00-00-00-00 to memory pointed by ZeroMac.

    @param[out] ZeroMac     Pointer to MAC address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroMac (
  OUT CHAR16    *ZeroMac )
{
    ZeroMac[0] = 48;
    ZeroMac[1] = 48;
    ZeroMac[2] = 45;
    ZeroMac[3] = 48;
    ZeroMac[4] = 48;
    ZeroMac[5] = 45;
    ZeroMac[6] = 48;
    ZeroMac[7] = 48;
    ZeroMac[8] = 45;
    ZeroMac[9] = 48;
    ZeroMac[10] = 48;
    ZeroMac[11] = 45;
    ZeroMac[12] = 48;
    ZeroMac[13] = 48;
    ZeroMac[14] = 45;
    ZeroMac[15] = 48;
    ZeroMac[16] = 48;
}

/** @internal
    Set IP address to ::::::: for memory pointed by ZeroIp.

    @param[out] ZeroIp  Pointer to Ip address in BIOS setup variable.

    @return VOID    Nothing.

**/
VOID
SetZeroIp6 (
  OUT CHAR16    *ZeroIp6 )
{
    ZeroMem (
        (UINT8*)&ZeroIp6[0],
        40 * 2 );
    ZeroIp6[0] = ':';
    ZeroIp6[1] = ':';
    ZeroIp6[2] = ':';
    ZeroIp6[3] = ':';
    ZeroIp6[4] = ':';
    ZeroIp6[5] = ':';
    ZeroIp6[6] = ':';
}

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
  IN UINT8                          LanChannelNumber )
{
    EFI_STATUS                                          Status;
    UINT8                                               ResponseDataSize;
    UINT8                                               Retries;
    AMI_GET_LAN_CONFIG_PARAM_REQUEST                    GetLanParamCmd;
    AMI_GET_LAN_CONFIG_PARAM_SET_IN_PROGRESS_RESPONSE   GetSetInProgResponse;

    GetLanParamCmd.LanChannel.ChannelNumber     = LanChannelNumber;
    GetLanParamCmd.LanChannel.Reserved          = 0;
    GetLanParamCmd.LanChannel.GetParam          = 0; 
    GetLanParamCmd.ParameterSelector            = AMI_IPMI_SELECTOR_NONE;
    GetLanParamCmd.SetSelector                  = AMI_IPMI_SELECTOR_NONE;
    GetLanParamCmd.BlockSelector                = AMI_IPMI_SELECTOR_NONE;

    // Wait for Set-In progress bit to clear.
    for (Retries = 0; Retries < AMI_IPMI_GET_SET_IN_PROGRESS_RETRY_COUNT; Retries++) {
        ResponseDataSize = sizeof (GetSetInProgResponse);
        Status = IpmiTransport->SendIpmiCommand (
                                    IpmiTransport,
                                    IPMI_NETFN_TRANSPORT,
                                    AMI_BMC_LUN,
                                    IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS,
                                    (UINT8 *)&GetLanParamCmd,
                                    sizeof (GetLanParamCmd),
                                    (UINT8 *)&GetSetInProgResponse,
                                    &ResponseDataSize );
        
        if (EFI_ERROR (Status)) {
            return Status;
        }

        if (GetSetInProgResponse.Param0.SetInProgress == AMI_IPMI_BIT_CLEAR) {
            return EFI_SUCCESS;
        }
        gBS->Stall (AMI_IPMI_STALL);
    }

    return EFI_TIMEOUT;
}


