#include "su_ui.h"
#include "commands.h"

//Show primary loading dialog
BOOLEAN SuLoadPrimary(PSU_GLOBAL_CONFIG pConfig)
{
	EFI_INPUT_KEY key = { 0 };
	CHAR16 *cmdStr = NULL;

	ST->ConOut->ClearScreen(ST->ConOut);
	Print(L"Starting default entry (%s) in %d seconds. Press ESC to cancel\n", pConfig->PrimaryName, pConfig->AutoLoadDelay);

	for (UINT32 seconds = pConfig->AutoLoadDelay; seconds; seconds--)
	{
		ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
		ST->ConIn->Reset(ST->ConIn, FALSE);

		if (key.ScanCode == SCAN_ESC)
		{
			return FALSE;
		}
		else
		{
			BS->Stall(1000000);
		}
	}

	if (!(cmdStr = AllocatePool((StrLen(pConfig->PrimaryName) + 50) * sizeof(WCHAR))))
	{
		Print(L"Allocation failed\n");
		goto Done;
	}

	StrCpy(cmdStr, L"LoadEntry ");
	StrCat(cmdStr, pConfig->PrimaryName);
	SuCmdController(cmdStr, pConfig);

Done:

	if (cmdStr)
		FreePool(cmdStr);

	return TRUE;
}

//Read command line input
void SuCommandLine(PSU_GLOBAL_CONFIG pConfig)
{
	UINTN keyEvent = 0;
	EFI_INPUT_KEY key = { 0 };
	CHAR16 *cmd = NULL;
	UINT32 load = 0;

	if (!(cmd = AllocateZeroPool(8001 * sizeof(CHAR16))))
	{
		Print(L"Failed allocation\n");
		goto Done;
	}

	ST->ConOut->ClearScreen(ST->ConOut);
	SuCmdController(L"Info", pConfig);
	SuCmdController(L"ListEntry", pConfig);

	while (TRUE)
	{
		Print(L"\n>");
		ZeroMem(cmd, 8000 * sizeof(CHAR16));
		load = 0;

		while (load < 8000)
		{
			BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &keyEvent);
			ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
			ST->ConIn->Reset(ST->ConIn, FALSE);

			if (key.UnicodeChar == CHAR_BACKSPACE)
			{
				if (load)
				{
					load--;
					cmd[load] = '\0';
					Print(L"\b");
				}
			}
			else if (key.UnicodeChar == CHAR_CARRIAGE_RETURN)
			{
				Print(L"\n");
				SuCmdController(cmd, pConfig);
				break;
			}
			else if(!(key.UnicodeChar < 32 || key.UnicodeChar > 125))
			{
				cmd[load] = key.UnicodeChar;
				cmd[load + 1] = '\0';
				load++;
				Print(L"%c", key.UnicodeChar);
			}
		}
	}

Done:

	if (cmd)
		FreePool(cmd);

	return;
}