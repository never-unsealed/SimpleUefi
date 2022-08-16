#pragma once
#include "simple_uefi.h"

void EFIAPI SuCommandInfo();
void EFIAPI SuCommandHelp();
void EFIAPI SuCommandShutdown();
void EFIAPI SuCommandRestart();
void EFIAPI SuCmdController(CHAR16 *pCommand, PSU_GLOBAL_CONFIG pConfig);