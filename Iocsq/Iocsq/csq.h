#pragma once
#include <ntddk.h>


namespace csq
{
	void InsertIrp(PIO_CSQ Csq, PIRP Irp);
	void RemoveIrp(PIO_CSQ Csq, PIRP Irp);
	PIRP PeekNextIrp(PIO_CSQ Csq, PIRP Irp, PVOID PeekContext);
	void AcquireLock(PIO_CSQ Csq, PKIRQL Irql);
	void ReleaseLock(PIO_CSQ Csq, KIRQL Irql);
	void CompleteCanceledIrp(PIO_CSQ Csq, PIRP Irp);

}