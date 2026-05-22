#include "Smbios.h"

extern INFO_CALLBACK_DATA  gInfoPrivate;
/*
CHAR16* mDeviceTypeStr[] = {
  L"Legacy BEV",
  L"Legacy Floppy",
  L"Legacy Hard Drive",
  L"Legacy CD ROM",
  L"Legacy PCMCIA",
  L"Legacy USB",
  L"Legacy Embedded Network",
  L"Legacy Unknown Device"
};
*/

/**
  Convert Memory Size to a string.

  @param MemorySize      The size of the memory to process
  @param String          The string that is created

**/
VOID
_ConvertMemorySizeToString(
  IN  UINT32  MemorySize,
  OUT CHAR16** String
)
{
  CHAR16* StringBuffer;

  StringBuffer = AllocateZeroPool(0x24);
  ASSERT(StringBuffer != NULL);
  UnicodeValueToStringS(StringBuffer, 0x24, LEFT_JUSTIFY, MemorySize, 10);
  StrCatS(StringBuffer, 0x24 / sizeof(CHAR16), L" MB RAM");

  *String = (CHAR16*)StringBuffer;

  return;
}


/**
  Get string by string id from HII Interface


  @param Id              String ID.

  @retval  CHAR16 *  String from ID.
  @retval  NULL      If error occurs.

**/
CHAR16*
_GetStringById(
  IN  EFI_STRING_ID  Id
)
{
  return HiiGetString(gInfoPrivate.HiiHandle, Id, NULL);
}


/**

  Acquire the string associated with the Index from smbios structure and return it.
  The caller is responsible for free the string buffer.

  @param    OptionalStrStart  The start position to search the string
  @param    Index             The index of the string to extract
  @param    String            The string that is extracted

  @retval   EFI_SUCCESS       The function returns EFI_SUCCESS always.

**/
EFI_STATUS
_GetOptionalStringByIndex(
  IN      CHAR8* OptionalStrStart,
  IN      UINT8   Index,
  OUT     CHAR16** String
)
{
  UINTN  StrSize;

  if (Index == 0) {
    *String = AllocateZeroPool(sizeof(CHAR16));
    return EFI_SUCCESS;
  }

  StrSize = 0;
  do {
    Index--;
    OptionalStrStart += StrSize;
    StrSize = AsciiStrSize(OptionalStrStart);
  } while (OptionalStrStart[StrSize] != 0 && Index != 0);

  if ((Index != 0) || (StrSize == 1)) {
    //
    // Meet the end of strings set but Index is non-zero, or
    // Find an empty string
    //
    *String = _GetStringById(STRING_TOKEN(STR_MISSING_STRING));
  }
  else {
    *String = AllocatePool(StrSize * sizeof(CHAR16));
    AsciiStrToUnicodeStrS(OptionalStrStart, *String, StrSize);
  }

  return EFI_SUCCESS;
}


/**
  Convert Processor Frequency Data to a string.

  @param ProcessorFrequency The frequency data to process
  @param Base10Exponent     The exponent based on 10
  @param String             The string that is created

**/
VOID
_ConvertProcessorToString(
  IN  UINT16  ProcessorFrequency,
  IN  UINT16  Base10Exponent,
  OUT CHAR16** String
)
{
  CHAR16* StringBuffer;
  UINTN   Index;
  UINTN   DestMax;
  UINT32  FreqMhz;

  if (Base10Exponent >= 6) {
    FreqMhz = ProcessorFrequency;
    for (Index = 0; Index < (UINT32)Base10Exponent - 6; Index++) {
      FreqMhz *= 10;
    }
  }
  else {
    FreqMhz = 0;
  }

  DestMax = 0x20 / sizeof(CHAR16);
  StringBuffer = AllocateZeroPool(0x20);
  ASSERT(StringBuffer != NULL);
  UnicodeValueToStringS(StringBuffer, sizeof(CHAR16) * DestMax, LEFT_JUSTIFY, FreqMhz / 1000, 3);
  Index = StrnLenS(StringBuffer, DestMax);
  StrCatS(StringBuffer, DestMax, L".");
  UnicodeValueToStringS(
    StringBuffer + Index + 1,
    sizeof(CHAR16) * (DestMax - (Index + 1)),
    PREFIX_ZERO,
    (FreqMhz % 1000) / 10,
    2
  );
  StrCatS(StringBuffer, DestMax, L" GHz");
  *String = (CHAR16*)StringBuffer;
  return;
}
