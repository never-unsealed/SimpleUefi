#include "config.h"
#include "util.h"
#include "strings.h"
#include "commands.h"

//Process config file
void EFIAPI SuProcessConfigFile(PSU_GLOBAL_CONFIG pGlobalConfig)
{
	CHAR8 *pFileContent = NULL;
	CHAR16 *cmd = NULL;
	UINT64 fileSize, currentPos = 0;

	if (!(pFileContent = SuReadConfigFile(&fileSize)))
	{
		Print(L"No config file found\n");
		goto Done;
	}
	
	if (!fileSize)
		goto Done;

	if (!(cmd = AllocateZeroPool((fileSize + 1) * sizeof(CHAR16))))
		goto Done;
	
	for (UINT64 i = 0; i < fileSize; i++)
	{
		if (pFileContent[i] == '\r')
			pFileContent[i] = '\0';

		if (pFileContent[i] == '\n')
		{
			SuCmdController(cmd, pGlobalConfig);
			ZeroMem(cmd, fileSize * sizeof(CHAR16));
			currentPos = 0;
		}
		else
		{
			cmd[currentPos] = pFileContent[i];
			currentPos++;
		}

		if (!(i + 1 < fileSize))
		{
			SuCmdController(cmd, pGlobalConfig);
		}
	}

Done:

	if (cmd)
		FreePool(cmd);

	if (pFileContent)
		FreePool(pFileContent);

	return;
}

//Inizialize config
BOOLEAN EFIAPI SuInitializeConfig(PSU_GLOBAL_CONFIG pGlobalConfig)
{
	EFI_STATUS obtainSuGuid = EFI_NOT_FOUND;
	EFI_GUID stdGuid = { 0xd009f5f3, 0x7625 , 0x44a6, { 0xb7, 0x15, 0x85, 0x36, 0x66, 0xbf, 0x9e, 0x47} };
	UINT32 attrs = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
	UINT64 dataSize = 0;

	CopyMem(&pGlobalConfig->EntryStore, &stdGuid, sizeof(EFI_GUID));
	SuProcessConfigFile(pGlobalConfig);

	return TRUE;
}
