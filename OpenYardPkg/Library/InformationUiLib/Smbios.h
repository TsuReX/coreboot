#include "Info.h"


VOID
_ConvertMemorySizeToString(
  IN  UINT32  MemorySize,
  OUT CHAR16** String
);

CHAR16*
_GetStringById(
  IN  EFI_STRING_ID  Id
);

EFI_STATUS
_GetOptionalStringByIndex(
  IN      CHAR8* OptionalStrStart,
  IN      UINT8   Index,
  OUT     CHAR16** String
);

VOID
_ConvertProcessorToString(
  IN  UINT16  ProcessorFrequency,
  IN  UINT16  Base10Exponent,
  OUT CHAR16** String
);
