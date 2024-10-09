#include "csq.h"
#include "dispatch.h"

/*
	void InsertIrp(PIO_CSQ Csq, PIRP Irp);
	void RemoveIrp(PIO_CSQ Csq, PIRP Irp);
	PIRP PeekNextIrp(PIO_CSQ Csq, PIRP Irp, PVOID PeekContext);
	void AcquireLock(PIO_CSQ Csq, PKIRQL Irql);
	void ReleaseLock(PIO_CSQ Csq, KIRQL Irql);
	void CompleteCanceledIrp(PIO_CSQ Csq, PIRP Irp);
*/



void csq::InsertIrp(PIO_CSQ Csq, PIRP Irp)
{
    DbgPrint("[*] csq :: inserting irp\n");
	pClientDeviceExtension DevExt;

	DevExt = CONTAINING_RECORD(Csq, ClientDeviceExtension, CancelSafeEventIrpQueue);

	InsertTailList(&DevExt->EventIrpQueue, &Irp->Tail.Overlay.ListEntry);
}


void csq::RemoveIrp(PIO_CSQ Csq, PIRP Irp)
{
    UNREFERENCED_PARAMETER(Csq);
    DbgPrint("[*] csq :: removing irp\n");
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

PIRP csq::PeekNextIrp(PIO_CSQ Csq, PIRP Irp, PVOID PeekContext)
{
    pClientDeviceExtension      DevExt;
    PIRP                    NextIrp = nullptr;
    PLIST_ENTRY             NextEntry;
    PLIST_ENTRY             ListHead;
    PIO_STACK_LOCATION     IrpStack;

    DevExt = CONTAINING_RECORD(Csq, ClientDeviceExtension, CancelSafeEventIrpQueue);

    ListHead = &DevExt->EventIrpQueue;

    if (Irp == nullptr) 
        NextEntry = ListHead->Flink;
    
    else 
        NextEntry = Irp->Tail.Overlay.ListEntry.Flink;
    

    while (NextEntry != ListHead)
    {

        NextIrp = CONTAINING_RECORD(NextEntry, IRP, Tail.Overlay.ListEntry);

        IrpStack = IoGetCurrentIrpStackLocation(NextIrp);

        if (PeekContext)
        {
            if (IrpStack->FileObject == reinterpret_cast<PFILE_OBJECT>(PeekContext))
                break;

        }
        else
        {
            break;
        }

        NextIrp = nullptr;
        NextEntry = NextEntry->Flink;
    }

    return NextIrp;
}

void csq::AcquireLock(PIO_CSQ Csq, PKIRQL Irql)
{
    pClientDeviceExtension   DevExt;

    DevExt = CONTAINING_RECORD(Csq, ClientDeviceExtension, CancelSafeEventIrpQueue);

    KeAcquireSpinLock(&DevExt->QueueLock, Irql);
}


void csq::ReleaseLock(PIO_CSQ Csq, KIRQL Irql)
{
    pClientDeviceExtension   DevExt;

    DevExt = CONTAINING_RECORD(Csq, ClientDeviceExtension, CancelSafeEventIrpQueue);

    KeReleaseSpinLock(&DevExt->QueueLock, Irql);
}

void csq::CompleteCanceledIrp(PIO_CSQ Csq, PIRP Irp)
{
    UNREFERENCED_PARAMETER(Csq);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


