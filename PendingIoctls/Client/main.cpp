#include <iostream>
#include <Windows.h>


#define IOCTL_GET_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)






int main()
{
   LPOVERLAPPED overlapped = new OVERLAPPED;
    if (!overlapped)
        return -1;

    overlapped->hEvent = CreateEventW(nullptr, true, false, nullptr);
    if (!overlapped->hEvent)
    {
        delete overlapped;
        return -1;
    }

    HANDLE device = CreateFileW(L"\\\\.\\PendingIoctls", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (device == INVALID_HANDLE_VALUE) {
        std::cout << "[*] Failed to obtain scorpion device handle" << std::endl;
        return -1;
    }
    WCHAR message[100];
    DWORD bytes;
    BOOL success = DeviceIoControl(device, IOCTL_GET_EVENT, nullptr, 0, message, 100 * sizeof(WCHAR), &bytes, overlapped);

    if (!success && GetLastError() != ERROR_IO_PENDING) {
        std::cout << "[*] Failed in DeviceIoControl: " << GetLastError() << std::endl;
        delete overlapped;
        CloseHandle(device);
        return -1;
    }
    std::cout << "[*] asynchronous ioctl sent!" << std::endl;

    WaitForSingleObject(overlapped->hEvent, INFINITE);

    std::cout << "[*] asynchronous ioctl completed by driver!" << std::endl;

    std::wcout << L"[*] received output : " << message << std::endl;


    delete overlapped;
    CloseHandle(device);

	return 0; 
}
