#pragma once
#include <Windows.h>
#include <iostream>

#define IOCTL_IOCSQ_WAIT_FOR_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA) // Simulates the event that puts IRP in queue
#define IOCTL_IOCSQ_TRIGGER_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)          // Simulates an event 
#define IOCTL_IOCSQ_SEND_DIALOG_RESULT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA) // Send response to driver (Allow/Deny)

namespace communicate
{
	bool wait_for_event(HANDLE hDevice);
	bool trigger_event(HANDLE hDevice);
	bool send_response(HANDLE hDevice);
}