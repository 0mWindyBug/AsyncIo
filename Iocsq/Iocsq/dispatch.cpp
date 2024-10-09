#include "dispatch.h"
#include "config.h"
#include "csq.h"

void dispatch::complete_request(NTSTATUS Status, PIRP Irp)
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
}

NTSTATUS dispatch::create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	dispatch::complete_request(STATUS_SUCCESS, Irp);

	return STATUS_SUCCESS;
}



NTSTATUS dispatch::device_control(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIRP EventIrp = nullptr;
	PIO_STACK_LOCATION irpStack;
	irpStack = IoGetCurrentIrpStackLocation(Irp);
	pClientDeviceExtension DevExt = reinterpret_cast<pClientDeviceExtension>(DeviceObject->DeviceExtension);

	switch (irpStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_IOCSQ_WAIT_FOR_EVENT:
		DbgPrint("[*} driver :: inserting irp to client queue!\n");

		// pend the request 
		// I/O manager will now wait on the UserEvent in case caller asked for synchronous I/O or return and allow caller to continue if it asked for asynchronous I/O 
		// in order to signal the said event , and because completion will occur in an arbitrary thread context we have to mark irp as pedning 
		// the I/O manager will check Irp->PendingReturned to determine if it needs to queue an APC and signal the UserEvent 

		// insert IRP to event queue, IoCsqInsertIrp marks the IRP as pending
		IoCsqInsertIrp(&DevExt->CancelSafeEventIrpQueue, Irp, nullptr);

		status = STATUS_PENDING;


		break;

	case IOCTL_IOCSQ_TRIGGER_EVENT:
		DbgPrint("[*] driver :: inserting irp to ks queue and completing pended irp from cleint queue!\n");

		EventIrp = IoCsqRemoveNextIrp(&DevExt->CancelSafeEventIrpQueue, nullptr);
		if (EventIrp)
		{
			DbgPrint("[*] driver :: completing event from queue!\n");
			dispatch::complete_request(STATUS_SUCCESS, EventIrp);
			status = STATUS_SUCCESS;
		}

		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;

	if (status == STATUS_PENDING)
		return STATUS_PENDING;

	DbgPrint("[*] driver :: completed IOCTL\n");

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;

}
