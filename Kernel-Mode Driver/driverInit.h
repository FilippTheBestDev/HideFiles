#ifndef DRIVER_INIT_H
#define DRIVER_INIT_H

#include "ntifs.h"
#include "hideFileAlgorithm.h"
#include "processIoCtlCodes.h"

extern UNICODE_STRING gDeviceName;
extern PCWSTR gDeviceNameStr;
extern UNICODE_STRING gSymbolicLinkName;
extern PCWSTR gSymbolicLinkNameStr;
extern PDEVICE_OBJECT gDeviceObject;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS Initialize();
void Uninitialize();
NTSTATUS DeviceControl(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS DeviceOpen(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS DeviceClose(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
void DriverUnload(IN PDRIVER_OBJECT DriverObject);

#endif