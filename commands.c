#include "commands.h"
#include "strings.h"
#include "base64.h"
#include "util.h"

//Tested
void EFIAPI SuCommandRestart()
{
	RT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
	Print(L"Failed to reboot\n");
}

//Tested
void EFIAPI SuCommandShutdown()
{
	RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
	Print(L"Failed to shutdown\n");
}

//Tested
void EFIAPI SuCommandClear()
{
	ST->ConOut->ClearScreen(ST->ConOut);
}

//Tested
void EFIAPI SuCommandHelp()
{
	Print(L"List of available commands:\n");
	Print(L"LoadEntry [Name] - Load into an OS entry\n");
	Print(L"ListEntry - List all available OS entries\n");
	Print(L"AddEntry [Name] [ESP Path] - Add bootable OS entry\n");
	Print(L"RemoveEntry [Name] - Remove bootable OS entry\n");
	Print(L"ValidateEntry [Name] - Make sure OS entry is not manipulated\n");
	Print(L"Export [Name] - Export OS entry device path\n");
	Print(L"Import [Name] [Content] - Import OS entry device path\n");
	Print(L"Set [VarName] [Value] - Set configuration value\n");
	Print(L"Clear - Clear screen\n");
	Print(L"Restart - Restart system\n");
	Print(L"Shutdown - Shutdown system\n");
	Print(L"Info - Print version info\n");
	Print(L"Help - List available commands\n");
}

//Tested
void EFIAPI SuCommandAddEntry(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *entryName = NULL, *filePathName = NULL;
	EFI_DEVICE_PATH *pDevicePath = NULL;
	EFI_LOADED_IMAGE *pLoadedImage;
	UINT64 dataSize = 0;
	UINT32 attrs = 0;

	if (!(entryName = SuIsolateToken(pCommand, 1)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		Print(L"Invalid entry name\n");
		goto Done;
	}

	if (!(filePathName = SuIsolateToken(pCommand, 2)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(filePathName) < 4)
	{
		Print(L"Invalid file path\n");
		goto Done;
	}

	if (RT->GetVariable(entryName, &pConfig->EntryStore, &attrs, &dataSize, NULL) != EFI_NOT_FOUND)
	{
		Print(L"Entry already exists\n");
		goto Done;
	}

	if (EFI_ERROR(BS->HandleProtocol(LibImageHandle, &gEfiLoadedImageProtocolGuid, &pLoadedImage)))
	{
		Print(L"Failed to obtain device handle\n");
		goto Done;
	}

	if (!(pDevicePath = FileDevicePath(pLoadedImage->DeviceHandle, filePathName)))
	{
		Print(L"Failed to obtain device path\n");
		goto Done;
	}

	if (EFI_ERROR(RT->SetVariable(entryName, &pConfig->EntryStore, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE, DevicePathSize(pDevicePath), pDevicePath)))
	{
		Print(L"Failed to enroll new entry\n");
		goto Done;
	}

Done:

	if (pDevicePath)
		FreePool(pDevicePath);

	if (filePathName)
		FreePool(filePathName);

	if (entryName)
		FreePool(entryName);

	return;
}

//Tested
BOOLEAN EFIAPI SuCommandValidateEntry(BOOLEAN output, CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	BOOLEAN retVal = FALSE;
	CHAR16 *entryName = FALSE;
	UINT64 dataSize = 0;
	UINT32 attrs = 0;

	if (!(entryName = SuIsolateToken(pCommand, !output ? 2 : 1)))
	{
		output ? Print(L"Invalid syntax\n") : 0;
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		output ? Print(L"Invalid entry name\n") : 0;
		goto Done;
	}

	RT->GetVariable(entryName, &pConfig->EntryStore, &attrs, &dataSize, NULL);

	if (!dataSize)
	{
		output ? Print(L"Entry not found\n") : 0;
		goto Done;
	}

	if (attrs & EFI_VARIABLE_RUNTIME_ACCESS)
	{
		output ? Print(L"Entry is NOT valid!\n") : 0;
		goto Done;
	}
	else
	{
		output ? Print(L"Entry is VALID!\n") : 0;
		retVal = TRUE;
	}

Done:

	if (entryName)
		FreePool(entryName);

	return retVal;
}

//Test
BOOLEAN EFIAPI SuCommandLoadEntry(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	BOOLEAN retVal = FALSE;
	CHAR16 *entryName = NULL;
	EFI_DEVICE_PATH *pDevicePath = NULL;
	EFI_STATUS loadFile;
	EFI_HANDLE loadHandle = NULL;
	UINT32 attributes = 0;
	UINT64 fileSize = 0;

	if (!(entryName = SuIsolateToken(pCommand, 1)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		Print(L"Invalid entry name\n");
		goto Done;
	}

	RT->GetVariable(entryName, &pConfig->EntryStore, &attributes, &fileSize, NULL);

	if (!fileSize)
	{
		Print(L"Invalid entry name\n");
		goto Done;
	}

	if (attributes & EFI_VARIABLE_RUNTIME_ACCESS)
	{
		Print(L"Entry is not genuine\n");
		goto Done;
	}

	if (!(pDevicePath = AllocatePool(fileSize + sizeof(EFI_DEVICE_PATH) * 2)))
	{
		Print(L"Failed to allocate device path\n");
		goto Done;
	}

	if (EFI_ERROR(RT->GetVariable(entryName, &pConfig->EntryStore, &attributes, &fileSize, pDevicePath)))
	{
		Print(L"Failed to retrieve device path\n");
		goto Done;
	}

	if (EFI_ERROR(loadFile = BS->LoadImage(FALSE, LibImageHandle, pDevicePath, NULL, 0, &loadHandle)))
	{
		if (loadFile == EFI_SECURITY_VIOLATION)
		{
			Print(L"Image is not trusted\n");
			goto Done;
		}
		else
		{
			Print(L"Failed to load file\n");
			goto Done;
		}
	}

	if (EFI_ERROR(BS->StartImage(loadHandle, 0, NULL)))
	{
		Print(L"Failed to start image\n");
		goto Done;
	}

Done:

	if (loadHandle)
		BS->UnloadImage(loadHandle);

	if (pDevicePath)
		FreePool(pDevicePath);

	if (entryName)
		FreePool(entryName);

	return retVal;
}

//Tested
void EFIAPI SuCommandRemoveEntry(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *entryName = NULL;
	UINT64 dataSize = 0;
	UINT32 attrs = 0;

	if (!(entryName = SuIsolateToken(pCommand, 1)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		Print(L"Invalid entry name\n");
		goto Done;
	}

	if (EFI_ERROR(RT->SetVariable(entryName, &pConfig->EntryStore, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE, 0, NULL)))
	{
		Print(L"Failed to delete entry. Is the name valid?\n");
	}
	else
	{
		Print(L"Entry deleted\n");
	}

Done:

	if (entryName)
		FreePool(entryName);

	return;
}

//Tested
void EFIAPI SuCommandListEntry(PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *pVariableName = AllocateZeroPool(500);
	UINT64 bufferSize = 500;
	UINT32 entryCount = 0;
	EFI_STATUS nextVar;
	EFI_GUID compare;

	if (!pVariableName)
	{
		Print(L"Error: Failed allocation\n");
		goto Done;
	}

	while ((nextVar = RT->GetNextVariableName(&bufferSize, pVariableName, &compare)) != EFI_NOT_FOUND)
	{
		if (EFI_ERROR(nextVar))
		{
			Print(L"GetNextVariableName Error\n");
			goto Done;
		}

		if (!RtCompareGuid(&pConfig->EntryStore, &compare))
		{
			entryCount++;
			Print(L"%d Entry: %s\n", entryCount, pVariableName);
		}

		bufferSize = 500;
	}

	if (!entryCount)
		Print(L"No boot entries found\n");

Done:

	if (pVariableName)
		FreePool(pVariableName);

	return;
}

//Tested
void EFIAPI SuCommandInfo()
{
	Print(L"Running SimpleUefi Version %d - %d\n", MAIN_VERSION, SUB_VERSION);
}

//Tested
void EFIAPI SuCommandExport(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *entryName = NULL;
	CHAR8 *base64Data = NULL, *output = NULL;
	UINTN dataSize = 0;
	UINT32 attrs;

	if (!(entryName = SuIsolateToken(pCommand, 1)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (!SuCommandValidateEntry(TRUE, pCommand, pConfig))
	{
		Print(L"Invalid entry\n");
		goto Done;
	}

	RT->GetVariable(entryName, &pConfig->EntryStore, &attrs, &dataSize, NULL);

	if (!dataSize)
	{
		Print(L"Error\n");
		goto Done;
	}

	if(!(base64Data = AllocatePool(dataSize)))
	{
		Print(L"Error\n");
		goto Done;
	}

	if (EFI_ERROR(RT->GetVariable(entryName, &pConfig->EntryStore, &attrs, &dataSize, base64Data)))
	{
		Print(L"GetVariable failed\n");
		goto Done;
	}

	if (!(output = base64Encode(base64Data, dataSize)))
	{
		Print(L"Encoding failed\n");
		goto Done;
	}

	if (!SuWriteExport(entryName, output))
	{
		Print(L"Failed to write export to disk\n");
	}
	else
	{
		Print(L"Export file written to ESP\n");
	}

Done:

	if (output)
		FreePool(output);

	if (base64Data)
		FreePool(base64Data);

	if (entryName)
		FreePool(entryName);

	return;
}

//Tested
void EFIAPI SuCommandImport(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *entryName = NULL, *entryValue = NULL;
	CHAR8 *asciiStr = NULL, *data = NULL;
	UINT64 dataSize = 0;
	UINT32 attrs = 0;

	if (!(entryName = SuIsolateToken(pCommand, 1)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (!(entryValue = SuIsolateToken(pCommand, 2)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (StrLen(entryName) < 4)
	{
		Print(L"Error\n");
		goto Done;
	}

	if (RT->GetVariable(entryName, &pConfig->EntryStore, NULL, &dataSize, NULL) != EFI_NOT_FOUND)
	{
		Print(L"Entry already exists\n");
		goto Done;
	}

	if (!(asciiStr = SuUnicodeToAscii(entryValue)))
	{
		Print(L"Error\n");
		goto Done;
	}

	if(!(dataSize = base64DecodedSize(asciiStr)))
	{
		Print(L"Invalid input value\n");
		goto Done;
	}

	if (!(data = AllocatePool(dataSize)))
	{
		Print(L"Error\n");
		goto Done;
	}

	if (!base64Decode(asciiStr, data, dataSize))
	{
		Print(L"Decoding failed\n");
		goto Done;
	}

	if (EFI_ERROR(RT->SetVariable(entryName, &pConfig->EntryStore, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE, dataSize, data))) 
	{
		Print(L"Import failed\n");
		goto Done;
	}

Done:

	if (data)
		FreePool(data);

	if (asciiStr)
		FreePool(asciiStr);

	if (entryValue)
		FreePool(entryValue);

	if (entryName)
		FreePool(entryName);

	return;
}

//Tested
void EFIAPI SuCommandSet(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	CHAR16 *valueInput = NULL;
	UINT64 dataSize = 0;
	UINT32 attrs = 0;
	BOOLEAN nonFree = FALSE;

	if (!(valueInput = SuIsolateToken(pCommand, 2)))
	{
		Print(L"Invalid syntax\n");
		goto Done;
	}

	if (SuCompareToken(pCommand, L"AutoLoadPrimary", 1))
	{
		if (StrLen(valueInput) == 1 && (valueInput[0] == '0' || valueInput[0] == '1'))
		{
			if (valueInput[0] == '0')
				pConfig->AutoLoadPrimary = FALSE;
			else
				pConfig->AutoLoadPrimary = TRUE;
		}
		else
		{
			Print(L"Invalid syntax\n");
		}
	}
	else if (SuCompareToken(pCommand, L"AutoLoadDelay", 1))
	{
		if (StrLen(valueInput) < 4 && SuIsNumerical(valueInput))
		{
			pConfig->AutoLoadDelay = (UINT32)Atoi(valueInput);
		}
		else
		{
			Print(L"Invalid value\n");
		}
	}
	else if (SuCompareToken(pCommand, L"Primary", 1))
	{
		if (SuCommandValidateEntry(FALSE, pCommand, pConfig))
		{
			pConfig->PrimaryHash = SuHashString(valueInput);
			pConfig->PrimaryName = valueInput;
			nonFree = TRUE;
		}
		else
		{
			Print(L"Invalid entry\n");
		}
	}
	else if (SuCompareToken(pCommand, L"Profile", 1))
	{
		if (SuVerifiyGUID(valueInput))
		{
			SuStringToGuid(&pConfig->EntryStore, valueInput);
		}
		else
		{
			Print(L"Invalid profile\n");
		}
	}
	else
	{
		Print(L"Unknown config value\n");
	}

Done:

	if (valueInput && !nonFree)
		FreePool(valueInput);

	return;
}

//Vaildate and execute commands
void EFIAPI SuCmdController(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig)
{
	if (!SuValidateCommand(pCommand))
	{
		Print(L"Invalid command\n");
		goto Done;
	}

	if (SuCompareToken(pCommand, L"Restart", 0))
	{
		SuCommandRestart();
	}
	else if (SuCompareToken(pCommand, L"AddEntry", 0))
	{
		SuCommandAddEntry(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"RemoveEntry", 0))
	{
		SuCommandRemoveEntry(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"Export", 0))
	{
		SuCommandExport(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"Import", 0))
	{
		SuCommandImport(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"LoadEntry", 0))
	{
		SuCommandLoadEntry(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"Shutdown", 0))
	{
		SuCommandShutdown();
	}
	else if (SuCompareToken(pCommand, L"ValidateEntry", 0))
	{
		SuCommandValidateEntry(TRUE, pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"Help", 0))
	{
		SuCommandHelp();
	}
	else if (SuCompareToken(pCommand, L"Info", 0))
	{
		SuCommandInfo();
	}
	else if (SuCompareToken(pCommand, L"ListEntry", 0))
	{
		SuCommandListEntry(pConfig);
	}
	else if (SuCompareToken(pCommand, L"Set", 0))
	{
		SuCommandSet(pCommand, pConfig);
	}
	else if (SuCompareToken(pCommand, L"Clear", 0))
	{
		SuCommandClear(pCommand, pConfig);
	}
	else
	{
		Print(L"Unknown command! Try \"Help\"\n");
	}

Done:

	return;
}
