#include <ntddk.h>
#include <wdmsec.h>
#include "dispatch.h"
#include "csq.h"

PDEVICE_OBJECT DeviceObject;
UNICODE_STRING deviceName, symLinkName;

void IocsqUnload(PDRIVER_OBJECT DriverObject)
{
	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&symLinkName);
	DbgPrint("[*] driver unloaded\n");
}


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ssdl;
	pClientDeviceExtension ClientDeviceExt;

	RtlInitUnicodeString(&deviceName, L"\\Device\\IoCsq");
	RtlInitUnicodeString(&symLinkName, L"\\??\\IoCsq");

	RtlInitUnicodeString(&ssdl, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");
	status = IoCreateDeviceSecure(DriverObject, sizeof(ClientDeviceExtension), &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &ssdl, NULL, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] Failed to create device  (status = %lx)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLinkName, &deviceName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] failed to create symlink  (status = 0x%x)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	ClientDeviceExt = reinterpret_cast<pClientDeviceExtension>(DeviceObject->DeviceExtension);

	KeInitializeSpinLock(&ClientDeviceExt->QueueLock);

	InitializeListHead(&ClientDeviceExt->EventIrpQueue);

	status = IoCsqInitialize(&ClientDeviceExt->CancelSafeEventIrpQueue, csq::InsertIrp, csq::RemoveIrp, csq::PeekNextIrp, csq::AcquireLock, csq::ReleaseLock, csq::CompleteCanceledIrp);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] failed to initialize cancel safe queue  (status = 0x%x)\n", status);
		IoDeleteDevice(DeviceObject);
		IoDeleteSymbolicLink(&symLinkName);
		return status;
	}

	DriverObject->DriverUnload = IocsqUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = dispatch::create_close;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = dispatch::create_close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = dispatch::device_control;


	DbgPrint("[*] driver loaded!\n");



	return status;

}