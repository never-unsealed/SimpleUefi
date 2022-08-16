#include "simple_uefi.h"
#include "config.h"
#include "su_ui.h"

//Entry point
EFI_STATUS EFIAPI SuEntry(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *pSystemTable)
{
	SU_GLOBAL_CONFIG globalConfig = { 0 };

	InitializeLib(imageHandle, pSystemTable);
	globalConfig.AutoLoadDelay = 5;

	//1. Load/create config
	if (!SuInitializeConfig(&globalConfig))
		goto Done;

	//2. Load default
	if(globalConfig.AutoLoadPrimary && globalConfig.PrimaryName)
		SuLoadPrimary(&globalConfig);

	//3. Show command prompt
	Print(L"Enter cmd\n");
	SuCommandLine(&globalConfig);

Done:

	if (globalConfig.PrimaryName)
		FreePool(globalConfig.PrimaryName);

	Print(L"Sleeping 30 seconds before terminating\n");
	pSystemTable->BootServices->Stall(30000000);
	return EFI_SUCCESS;
}