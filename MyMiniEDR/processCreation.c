#include "common.h"

// Function to get the ImageName from a PID
PUNICODE_STRING GetImageNameFromPID(IN HANDLE hPID) {

	NTSTATUS			STATUS			= STATUS_SUCCESS;
	PEPROCESS			pProcess		= NULL; // Resolved process object
	PUNICODE_STRING		pImageName		= NULL; // Image name

	// Retrieve the EPROCESS of fake parent
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-pslookupprocessbyprocessid
	STATUS = PsLookupProcessByProcessId(
		hPID,					// Process ID to get EPROCESS off
		&pProcess				// OUT pointer to EPROCESS
	);
	if (!NT_SUCCESS(STATUS)) {
		error("SeLocateProcessImageName - Failed to get image name (0x%08X)", STATUS);
		return NULL;
	}

	// Retrieve the image of fake parent
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-selocateprocessimagename
	STATUS = SeLocateProcessImageName(
		pProcess,				// Pointer to EPROCESS
		&pImageName				// Pointer to ImageName
	);
	if (!NT_SUCCESS(STATUS)) {
		error("SeLocateProcessImageName - Failed to get image name (0x%08X)", STATUS);
		return NULL;
	}

	// Clean up
	ObDereferenceObject(pProcess);

	return pImageName;
}

// Function to check if command line arguments contain keyword
BOOLEAN ContainsKeyword(IN PCUNICODE_STRING pCommandLine, IN PCWSTR pwszKeyword) {

	UNICODE_STRING	usKeywordStr	= { 0 }; // Holds the keyword as a UNICODE_STRING
	UNICODE_STRING	usSlice			= { 0 }; // Temporary slice from the command line
	USHORT			i				= { 0 }; // Loop index

	// Validate input command line
	if (pCommandLine == NULL || pCommandLine->Buffer == NULL || pCommandLine->Length == 0) {
		error("ContainsKeyword - Something is null");
		return FALSE;
	}

	// RtlInitUnicodeString - Initializes a UNICODE_STRING structure from a null-terminated wide string
	RtlInitUnicodeString(&usKeywordStr, pwszKeyword);

	// Validate the keyword length against the input command line
	if (usKeywordStr.Length == 0 || usKeywordStr.Length > pCommandLine->Length) {
		error("ContainsKeyword - Something is null");
		return FALSE;
	}

	// Loop through the command line buffer to compare slices with the keyword
	for (i = 0; i <= (pCommandLine->Length - usKeywordStr.Length) / sizeof(WCHAR); i++) {
		usSlice.Buffer = &pCommandLine->Buffer[i];
		usSlice.Length = usKeywordStr.Length;
		usSlice.MaximumLength = usKeywordStr.Length;

		// RtlEqualUnicodeString - Compares two UNICODE_STRING structures (case-insensitive)
		if (RtlEqualUnicodeString(&usSlice, &usKeywordStr, TRUE)) {
			return TRUE;
		}
	}

	// Keyword not found in the command line
	return FALSE;
}

// Called on every process creation and termination
// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/nc-ntddk-pcreate_process_notify_routine_ex
VOID ProcessNotifyCallback(
	IN OUT	PEPROCESS				Process,		// Pointer to the EPROCESS structure of the process
	IN		HANDLE					ProcessId,		// Unique ID of the process
	IN		PPS_CREATE_NOTIFY_INFO	CreateInfo		// Pointer to creation info struct, or NULL on process exit
) {

	PUNICODE_STRING		pParentImageFileName		= { 0 };
	PUNICODE_STRING		pCreatorImageFileName	= { 0 };

	UNREFERENCED_PARAMETER(Process);

	if (ProcessId) {
		info("Process created - PID: %lu", PtrToUlong(ProcessId));
	}

	if (CreateInfo) {
		
		// ImageFileName
		if (CreateInfo->ImageFileName) {
			info("    ImageFileName: %wZ", CreateInfo->ImageFileName);
		}
		else {
			info("    ImageFileName: <unknown>");
		}

		// Print the Parent ID
		if (CreateInfo->ParentProcessId) {
			// Claimed parent ID
			info("    ParentProcessId: %lu", PtrToUlong(CreateInfo->ParentProcessId));

			// Get the ImageName
			pParentImageFileName = GetImageNameFromPID(CreateInfo->ParentProcessId);
			info("    ParentProcessId ImageFileName: %wZ", pParentImageFileName);

		}
		if (CreateInfo->CreatingThreadId.UniqueProcess) {
			// Real parent ID
			info("    CreatorProcess: %lu", PtrToUlong(CreateInfo->CreatingThreadId.UniqueProcess));

			// Get the ImageName
			pCreatorImageFileName = GetImageNameFromPID(CreateInfo->CreatingThreadId.UniqueProcess);
			info("    CreatorProcess ImageFileName: %wZ", pCreatorImageFileName);

		}

		// Print the TID
		if (CreateInfo->CreatingThreadId.UniqueThread) {
			info("    UniqueThreadID: %lu", PtrToUlong(CreateInfo->CreatingThreadId.UniqueThread));
		}

		// Print the command line command
		if (CreateInfo->CommandLine) {
			info("    CommandLine: %wZ", CreateInfo->CommandLine);
		}

		//// Print the executable file object
		//if (CreateInfo->FileObject) {
		//	info("    CreateInfo->FileObject: %p", CreateInfo->FileObject);
		//}

		// Check MimiKatz commandline arguments
		if (ContainsKeyword(CreateInfo->CommandLine, L"sekurlsa::")			||
			ContainsKeyword(CreateInfo->CommandLine, L"lsadump::")			||
			ContainsKeyword(CreateInfo->CommandLine, L"privilege::debug")	||
			ContainsKeyword(CreateInfo->CommandLine, L"kerberos::")			||
			ContainsKeyword(CreateInfo->CommandLine, L"token::")			||
			ContainsKeyword(CreateInfo->CommandLine, L"crypto::"))
		{
			alert("Malicious commandLine args detected: Mimikatz: %wZ", CreateInfo->CommandLine);
			alert("    PID: %lu", PtrToUlong(ProcessId));
			alert("    ImageFileName: %wZ", CreateInfo->ImageFileName);
		}

		// Check for PPID Spoofing
		if (CreateInfo->CreatingThreadId.UniqueProcess &&
			CreateInfo->ParentProcessId &&
			CreateInfo->CreatingThreadId.UniqueProcess != CreateInfo->ParentProcessId)
		{
			alert("PPID Spoofing detected: Real PID %lu vs fake PID %lu", 
				PtrToUlong(CreateInfo->CreatingThreadId.UniqueProcess), 
				PtrToUlong(CreateInfo->ParentProcessId));
			alert("    Creator ImageFileName: %wZ", pCreatorImageFileName);
			alert("    Partent ImageFileName: %wZ (faked parent)", pParentImageFileName);
		}

	}
	else {
		// Process termination
		info("Process exited - PID: %lu", PtrToUlong(ProcessId));
	}

	// **** CLEANUP SECTION ****

	// Free allocated strings from SeLocateProcessImageName
	// The caller is responsible for freeing the buffer that pImageFileName points to, using the ExFreeXxx routine that corresponds to the routine used to allocate the buffer.
	if (pParentImageFileName) {
		ExFreePoolWithTag(pParentImageFileName, 'ipnS');
	}
	if (pCreatorImageFileName) {
		ExFreePoolWithTag(pCreatorImageFileName, 'ipnS');
	}

}