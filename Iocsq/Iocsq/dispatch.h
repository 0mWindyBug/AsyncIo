#pragma once
#include <ntddk.h>

#define IOCTL_IOCSQ_WAIT_FOR_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)		// pending IOCTL to wait for events 
#define IOCTL_IOCSQ_TRIGGER_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)       // Simulates an event 


namespace dispatch
{
	void complete_request(NTSTATUS Status, PIRP Irp);
	NTSTATUS create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp);
	NTSTATUS device_control(PDEVICE_OBJECT DeviceObject, PIRP Irp);

}

typedef struct _ClientDeviceExtension
{
	LIST_ENTRY EventIrpQueue;
	KSPIN_LOCK QueueLock;
	IO_CSQ CancelSafeEventIrpQueue;
}ClientDeviceExtension, * pClientDeviceExtension;