#include "common.h"

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {

	NTSTATUS		STATUS			= STATUS_SUCCESS;	// Stores NTSTATUS information
	PDEVICE_OBJECT	pDeviceObject	= NULL;				// Stores pointer to the DEVICE_OBJECT

	// Unreference unused parameter
	UNREFERENCED_PARAMETER(RegistryPath);

	// Execute UnloadDriver on driver unload
	DriverObject->DriverUnload = UnloadDriver;

	// Define the device name
	UNICODE_STRING  usDevName = RTL_CONSTANT_STRING(DRIVER_DEVICENAME);

	// Create device object
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatedevice
	STATUS = IoCreateDevice(
		DriverObject,					// Pointer to the driver object for the caller.
		0,								// Can be null
		&usDevName,						// Name of device object
		FILE_DEVICE_UNKNOWN,			// Specify UNKNOWN device type
		FILE_DEVICE_SECURE_OPEN,		// Most drivers specify FILE_DEVICE_SECURE_OPEN for this parameter.
		FALSE,							// Most drivers set this value to FALSE.
		&pDeviceObject					// Output pointer to device object
	);
	if (!NT_SUCCESS(STATUS)) {
		error("IoCreateDevice - Driver failed to create device (0x%08X)", STATUS);
		return STATUS;
	}
	info("IoCreateDevice - Device object %wZ created", &usDevName);

	// Register process creation kernell callback
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/nf-ntddk-pssetcreateprocessnotifyroutineex
	STATUS = PsSetCreateProcessNotifyRoutineEx(
		ProcessNotifyCallback,  // Routine to be executed on process creation
		FALSE					// Register callback
	);
	if (!NT_SUCCESS(STATUS)) {
		error("PsSetCreateProcessNotifyRoutineEx - Failed to register callback (0x%08X)", STATUS);
	}
	else {
		okay("PsSetCreateProcessNotifyRoutineEx - Registered process creation callback");
	}

	return STATUS_SUCCESS;

}

// Driver unload function
VOID UnloadDriver(IN PDRIVER_OBJECT DriverObject) {

	UNICODE_STRING usDevName = RTL_CONSTANT_STRING(DRIVER_DEVICENAME);

	// Delete the device object
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iodeletedevice
	IoDeleteDevice(DriverObject->DeviceObject);
	info("IoDeleteDevice - Driver %wZ unloaded", &usDevName);

	// Unregister process creation kernell callback
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/nf-ntddk-pssetcreateprocessnotifyroutineex
	PsSetCreateProcessNotifyRoutineEx(
		ProcessNotifyCallback,  // Routine to be executed on process creation
		TRUE					// Remove (unregister) callback
	);

}