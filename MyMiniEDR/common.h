#pragma once

#include <ntifs.h>
#include "config.h"

// ***** HELPER FUNCTIONS FOR PRINTING ***** //
// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-dbgprintex
#define okay(msg, ...)    DbgPrintEx(0, DPFLTR_INFO_LEVEL,    "[+] [%s] " msg "\n", DRIVER_NAME, ##__VA_ARGS__)
#define info(msg, ...)    DbgPrintEx(0, DPFLTR_INFO_LEVEL,    "[i] [%s] " msg "\n", DRIVER_NAME, ##__VA_ARGS__)
#define error(msg, ...)   DbgPrintEx(0, DPFLTR_ERROR_LEVEL,   "[-] [%s] " msg "\n", DRIVER_NAME, ##__VA_ARGS__)
#define alert(msg, ...)   DbgPrintEx(0, DPFLTR_INFO_LEVEL,    "[!] [%s] " msg "\n", DRIVER_NAME, ##__VA_ARGS__)

// NT Macro for succes
#define NT_SUCCESS(status)	        (((NTSTATUS)(status)) >= 0) // https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-ntstatus-values

// ***** FUNCTION PROTOTYPES ***** //
// Function prototypes are needed so each source file is aware of the function's signature 
// (name, return type, and parameters) before the compiler encounters the function call.

// For functions in `main.c`
VOID UnloadDriver(IN PDRIVER_OBJECT DriverObject);

// For functions in `processCreation.c`
VOID ProcessNotifyCallback(IN OUT PEPROCESS Process, IN HANDLE ProcessId, IN PPS_CREATE_NOTIFY_INFO CreateInfo);
BOOLEAN ContainsKeyword(IN PCUNICODE_STRING pCommandLine, IN PCWSTR pwszKeyword);
PUNICODE_STRING GetImageNameFromPID(IN HANDLE hPID);
