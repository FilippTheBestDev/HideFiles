#include "driverInit.h"

UNICODE_STRING gDeviceName;
PCWSTR gDeviceNameStr = L"\\Device\\HideFiles";
UNICODE_STRING gSymbolicLinkName;
PCWSTR gSymbolicLinkNameStr = L"\\DosDevices\\HideFiles";
PDEVICE_OBJECT gDeviceObject = NULL;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
	DbgPrint("DriverEntry()\n");

	RtlInitUnicodeString(&gDeviceName, gDeviceNameStr);
	RtlInitUnicodeString(&gSymbolicLinkName, gSymbolicLinkNameStr);

	NTSTATUS status;
	status = IoCreateDevice(
		DriverObject,     // вказiвник на об'єкт драйверу
		0,                // додатковий розмiр (DeviceExtensionSize)
		&gDeviceName,     // вказiвник на Unicode-iм'я
		FILE_DEVICE_NULL, // тип пристрiю
		0,
		FALSE,
		&gDeviceObject    // вказiвник на об'єкт пристрiю
	);
	if (status != STATUS_SUCCESS)
		return STATUS_FAILED_DRIVER_ENTRY;

	// символiчне посилання, щоб полегшити звертання до драйверу з режиму користувача
	status = IoCreateSymbolicLink(&gSymbolicLinkName, &gDeviceName);
	if (status != STATUS_SUCCESS)
		return STATUS_FAILED_DRIVER_ENTRY;

	/* Регiстрацiя обробникiв IRP */
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceOpen;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;

	return Initialize();
}
NTSTATUS Initialize() {
	ModifyNtQuery();
	return STATUS_SUCCESS;
}
void Uninitialize() {
	RestoreNtQuery();
}
// DeviceControl обробляє виклики DeviceIoControl із режиму користувача (User Mode).
NTSTATUS DeviceControl(IN PDEVICE_OBJECT fdo, IN PIRP pIrp) {
	return ProcessIrp(pIrp);
}
// виклик IRP_MJ_CREATE
NTSTATUS DeviceOpen(IN PDEVICE_OBJECT fdo, IN PIRP Irp) {
	DbgPrint("DeviceOpen()\n");

	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}
// виклик IRP_MJ_CLOSE
NTSTATUS DeviceClose(IN PDEVICE_OBJECT fdo, IN PIRP Irp) {
	DbgPrint("DeviceClose()\n");

	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}
void DriverUnload(IN PDRIVER_OBJECT DriverObject) {
	Uninitialize();

	IoDeleteSymbolicLink(&gSymbolicLinkName);
	IoDeleteDevice(gDeviceObject);

	DbgPrint("DriverUnload()\n");

	return;
}
