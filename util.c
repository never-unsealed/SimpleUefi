#include "util.h"
#include "strings.h"

//Write to disk
BOOLEAN EFIAPI SuWriteExport(CHAR16 *pEntryName, CHAR8 *pBase64)
{
	BOOLEAN retVal = FALSE;
	EFI_LOADED_IMAGE *pLoadedImage;
	EFI_FILE_IO_INTERFACE *pDrive;
	EFI_FILE *pRoot, *pExportFile = NULL;
	UINTN baseLen = strlena(pBase64);
	CHAR16 fullPath[255];

	if (StrLen(pEntryName) > 200)
		goto Done;

	StrCpy(fullPath, L"EFI\\export_");
	StrCat(fullPath, pEntryName);

	if (EFI_ERROR(BS->HandleProtocol(LibImageHandle, &LoadedImageProtocol, &pLoadedImage)))
		goto Done;

	if (EFI_ERROR(BS->HandleProtocol(pLoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, &pDrive)))
		goto Done;

	if (EFI_ERROR(pDrive->OpenVolume(pDrive, &pRoot)))
		goto Done;

	if (EFI_ERROR(pRoot->Open(pRoot, &pExportFile, fullPath, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0)))
		goto Done;

	if (EFI_ERROR(pExportFile->Write(pExportFile, &baseLen, pBase64)))
		goto Done;

	if (EFI_ERROR(pExportFile->Close(pExportFile)))
		goto Done;

	retVal = TRUE;

Done:

	return retVal;
}

//Read config file
void *EFIAPI SuReadConfigFile(UINT64 *pFileSize)
{
	void *retVal = NULL;
	EFI_LOADED_IMAGE *pLoadedImage;
	EFI_FILE_IO_INTERFACE *pDrive;
	EFI_FILE_INFO *pFileInfo = NULL;
	EFI_FILE *pRoot, *pConfig = NULL;
	UINT64 bufSize = 0;

	if (EFI_ERROR(BS->HandleProtocol(LibImageHandle, &LoadedImageProtocol, &pLoadedImage)))
		goto Done;

	if (EFI_ERROR(BS->HandleProtocol(pLoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, &pDrive)))
		goto Done;

	if (EFI_ERROR(pDrive->OpenVolume(pDrive, &pRoot)))
		goto Done;

	if(EFI_ERROR(pRoot->Open(pRoot, &pConfig, L"EFI\\s.cfg", EFI_FILE_MODE_READ, 0)))
		goto Done;

	pConfig->GetInfo(pConfig, &gEfiFileInfoGuid, &bufSize, &pFileInfo);

	if (!bufSize)
		goto Done;

	if (!(pFileInfo = AllocateZeroPool(bufSize)))
		goto Done;

	if (EFI_ERROR(pConfig->GetInfo(pConfig, &gEfiFileInfoGuid, &bufSize, pFileInfo)))
		goto Done;

	if (!(pFileInfo->FileSize))
		goto Done;

	if (!(retVal = AllocateZeroPool(pFileInfo->FileSize)))
		goto Done;

	if (EFI_ERROR(pConfig->Read(pConfig, &pFileInfo->FileSize, retVal)))
	{
		FreePool(retVal);
		retVal = NULL;
	}
	else
	{
		*pFileSize = pFileInfo->FileSize;
	}

Done:

	if (pFileInfo)
		FreePool(pFileInfo);

	return retVal;
}

//Verifiy GUID
BOOLEAN SuVerifiyGUID(CHAR16 *pStr)
{
	WCHAR g;

	if (StrLen(pStr) != 38)
		return FALSE;

	for (int i = 0; i < StrLen(pStr); ++i)
	{
		g = pStr[i];

		if (i == 0) 
		{
			if (g != '{') 
				return FALSE;
		}
		else if (i == 37) 
		{
			if (g != '}')
				return FALSE;
		}
		else if ((i == 9) || (i == 14) || (i == 19) || (i == 24)) 
		{
			if (g != '-') 
				return FALSE;
		}
		else 
		{
			if (!((g >= '0') && (g <= '9')) && !((g >= 'A') && (g <= 'F')) && !((g >= 'a') && (g <= 'f'))) 
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

//String to GUID
void EFIAPI SuStringToGuid(EFI_GUID *pGuid, CHAR16 *pStr)
{
	unsigned char guidSingleByte[39] = { 0 };
	char holder[3];
	holder[2] = 0;

	for (UINT32 i = 0; i < 38; i++)
		guidSingleByte[i] = (unsigned char)pStr[i];

	pGuid->Data1 = strtoulc(guidSingleByte + 1, NULL, 16);
	pGuid->Data2 = (UINT16)strtoulc(guidSingleByte + 10, NULL, 16);
	pGuid->Data3 = (UINT16)strtoulc(guidSingleByte + 15, NULL, 16);

	CopyMem(holder, guidSingleByte + 20, 2);
	pGuid->Data4[0] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 22, 2);
	pGuid->Data4[1] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 25, 2);
	pGuid->Data4[2] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 27, 2);
	pGuid->Data4[3] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 29, 2);
	pGuid->Data4[4] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 31, 2);
	pGuid->Data4[5] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 33, 2);
	pGuid->Data4[6] = (UINT8)strtoulc(holder, NULL, 16);

	CopyMem(holder, guidSingleByte + 35, 2);
	pGuid->Data4[7] = (UINT8)strtoulc(holder, NULL, 16);
}