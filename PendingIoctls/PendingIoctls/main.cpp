#include <main.h>



PDEVICE_OBJECT DeviceObject;
UNICODE_STRING deviceName, symLinkName;



void Worker(PDEVICE_OBJECT DeviceObject, PVOID Context) {
	PWRKCONTEXT Contx = (PWRKCONTEXT)Context;
	PIRP Irp = Contx->Irp;
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

	// set message to return 
	PWCHAR data = L"message from driver";
	size_t data_length = wcslen(data) * sizeof(WCHAR);
	size_t outbuf_length = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	
	size_t bytes_to_copy = (outbuf_length < data_length?outbuf_length:data_length);

	RtlCopyBytes(Irp->AssociatedIrp.SystemBuffer, data, bytes_to_copy);

	ExFreePoolWithTag(Context, TAG);

	Irp->IoStatus.Information = bytes_to_copy;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
}



NTSTATUS CreateCloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS IoctlHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpStack;
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_GET_EVENT)
	{
		// allocate context 
		PWRKCONTEXT Context = (PWRKCONTEXT)ExAllocatePoolWithTag(NonPagedPool, sizeof(WRKCONTEXT), TAG);
		if (!Context)
		{
			DbgPrint("[*] failed to allocate context\n");
			Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		Context->Irp = Irp;

		// allocate work item 
		PIO_WORKITEM WorkItem = IoAllocateWorkItem(DeviceObject);
		if (!WorkItem)
		{
			ExFreePoolWithTag(Context, TAG);
			DbgPrint("[*] failed to allocate work item\n");
			Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		// pend the request 
	// I/O manager will now wait on the UserEvent in case caller asked for synchronous I/O or return and allow caller to continue if it asked for asynchronous I/O 
	// in order to signal the said event , and because completion will occur in an arbitrary thread context we have to mark irp as pedning 
	// the I/O manager will check Irp->PendingReturned to determine if it needs to queue an APC and signal the UserEvent 

		IoMarkIrpPending(Irp);
		Irp->IoStatus.Status = STATUS_PENDING;

		IoQueueWorkItem(WorkItem, (PIO_WORKITEM_ROUTINE)Worker, DelayedWorkQueue, Context);

		return STATUS_PENDING;
	}
	else
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
	}


}


void DriverUnload(
	_In_ PDRIVER_OBJECT DriverObject
)
{

	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&symLinkName);
	DbgPrint("[*] driver unloaded\n");
}


EXTERN_C NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ssdl;

	RtlInitUnicodeString(&deviceName, L"\\Device\\PendingIoctls");
	RtlInitUnicodeString(&symLinkName, L"\\??\\PendingIoctls");

	RtlInitUnicodeString(&ssdl, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"); 
	status = IoCreateDeviceSecure(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &ssdl, NULL, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] Failed to create device  (status = %lx)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLinkName, &deviceName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] Failed to create symlink  (status = %lx)\n", status);
		return status;
	}

	DriverObject->DriverUnload = DriverUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlHandler;


	DbgPrint("[*] driver loading!\n");



	return status;

}