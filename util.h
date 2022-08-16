#pragma once
#include "simple_uefi.h"

void EFIAPI SuStringToGuid(EFI_GUID *pGuid, CHAR16 *pStr);
void *EFIAPI SuReadConfigFile(UINT64 *pFileSize);
BOOLEAN EFIAPI SuWriteExport(CHAR16 *pEntryName, CHAR8 *pBase64);
BOOLEAN EFIAPI SuVerifiyGUID(CHAR16 *pStr);
