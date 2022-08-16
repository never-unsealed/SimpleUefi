#pragma once
#include "simple_uefi.h"

CHAR16 *EFIAPI SuIsolateToken(CHAR16 *pCommand, UINT32 tokenNr);
BOOLEAN EFIAPI SuCompareToken(CHAR16 *pCmd, CHAR16 *pToken, UINT32 tokenNr);
BOOLEAN EFIAPI SuValidateCommand(CHAR16 *pCommand);
UINT32 EFIAPI SuHashString(CHAR16 *str);
BOOLEAN EFIAPI SuIsNumerical(CHAR16 *str);
CHAR8 *EFIAPI SuUnicodeToAscii(CHAR16 *str);

unsigned long EFIAPI strtoulc(const char *nptr, char **endptr, register int base);