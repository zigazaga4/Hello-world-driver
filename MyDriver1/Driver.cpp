#include <ntddk.h>

extern "C" VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject);
VOID TimerRoutine(PKDPC DpcLocal, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2);

KTIMER Timer;
KDPC Dpc;

BOOLEAN FileWritten = FALSE;

NTSTATUS WriteToFile(PCWSTR FileName, PVOID Buffer, ULONG BufferLength)
{
    NTSTATUS status;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING uniName;
    RtlInitUnicodeString(&uniName, FileName);

    InitializeObjectAttributes(&objectAttributes, &uniName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);

        status = ZwCreateFile(&fileHandle, GENERIC_WRITE, &objectAttributes,
        &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
        0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (NT_SUCCESS(status))
    {
                status = ZwWriteFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock,
            Buffer, BufferLength, NULL, NULL);
                ZwClose(fileHandle);
    }

    return status;
}

extern "C"
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

        KeInitializeDpc(&Dpc, TimerRoutine, NULL);

        KeInitializeTimer(&Timer);

        LARGE_INTEGER interval;
    interval.QuadPart = -30000000;     KeSetTimerEx(&Timer, interval, 3000, &Dpc); 
        if (!FileWritten)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Driver loaded successfully\n");
        char message[] = "Driver loaded successfully\n";
        WriteToFile(L"\\??\\C:\\driver_output.txt", message, sizeof(message));
        FileWritten = TRUE;     }

        DriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}

VOID TimerRoutine(
    PKDPC DpcLocal,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
)
{
    UNREFERENCED_PARAMETER(DpcLocal);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello World\n");
}

VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

        KeCancelTimer(&Timer);

        if (!FileWritten)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Driver unloaded\n");
        char message[] = "Driver unloaded\n";
        WriteToFile(L"\\??\\C:\\driver_output.txt", message, sizeof(message));
        FileWritten = TRUE;     }
}
