#pragma once

#include <efi.h>
#include <lib.h>
#include <intrin.h>
#include <ctype.h>

#define MAIN_VERSION 2
#define SUB_VERSION	 5

#define ULONG_MAX     0xffffffffUL
#define ERANGE        34


typedef struct _SU_GLOBAL_CONFIG
{
	BOOLEAN AutoLoadPrimary;
	UINT32 AutoLoadDelay;
	UINT32 PrimaryHash;
	CHAR16 *PrimaryName;
	EFI_GUID EntryStore;
}SU_GLOBAL_CONFIG, *PSU_GLOBAL_CONFIG;
