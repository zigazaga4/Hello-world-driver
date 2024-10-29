#include <ntddk.h>

// Forward declaration of the timer routine and unload routine
extern "C" VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject);
VOID TimerRoutine(PKDPC DpcLocal, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2);

// Declare the timer and DPC (Deferred Procedure Call) objects globally
KTIMER Timer;
KDPC Dpc;

// Flag to track if the file has been written to
BOOLEAN FileWritten = FALSE;

// Function to write data to a file
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

    // Create or open the file
    status = ZwCreateFile(&fileHandle, GENERIC_WRITE, &objectAttributes,
        &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
        0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (NT_SUCCESS(status))
    {
        // Write to the file
        status = ZwWriteFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock,
            Buffer, BufferLength, NULL, NULL);
        // Close the file
        ZwClose(fileHandle);
    }

    return status;
}

// DriverEntry - Entry point of the driver
extern "C"
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    // Initialize the DPC object
    KeInitializeDpc(&Dpc, TimerRoutine, NULL);

    // Initialize the timer object
    KeInitializeTimer(&Timer);

    // Set the timer for 3 seconds and make it recurring
    LARGE_INTEGER interval;
    interval.QuadPart = -30000000; // 3 seconds in 100-nanosecond intervals (negative for relative time)
    KeSetTimerEx(&Timer, interval, 3000, &Dpc); // 3000 ms recurring timer

    // Log to the kernel debugger and write to the file (only once)
    if (!FileWritten)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Driver loaded successfully\n");
        char message[] = "Driver loaded successfully\n";
        WriteToFile(L"\\??\\C:\\driver_output.txt", message, sizeof(message));
        FileWritten = TRUE; // Ensure the file is written only once
    }

    // Set up the unload routine
    DriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}

// TimerRoutine - Called when the timer fires
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

    // Log "Hello World" to the kernel debugger every 3 seconds
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello World\n");
}

// Unload routine
VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

    // Cancel the timer if needed
    KeCancelTimer(&Timer);

    // Log to the kernel debugger and write to the file (only if it hasn't been written before)
    if (!FileWritten)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Driver unloaded\n");
        char message[] = "Driver unloaded\n";
        WriteToFile(L"\\??\\C:\\driver_output.txt", message, sizeof(message));
        FileWritten = TRUE; // Ensure the file is written only once
    }
}
