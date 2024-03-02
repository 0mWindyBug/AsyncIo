#pragma once
#include <ntddk.h>
#include <wdmsec.h>

#define IOCTL_GET_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define TAG 'pend'


typedef struct _WRKCONTEXT
{
	PIRP Irp;
}WRKCONTEXT, * PWRKCONTEXT;