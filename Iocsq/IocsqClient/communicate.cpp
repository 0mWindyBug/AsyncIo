#include <thread>
#include "communicate.h"


bool communicate::wait_for_event(HANDLE hDevice)
{
    while (true)
    {
        OVERLAPPED* overlapped = new OVERLAPPED();
        if (!overlapped)
            return false;

        overlapped->hEvent = CreateEventW(nullptr, true, false, nullptr);
        if (!overlapped->hEvent)
        {
            std::cout << "[*] Failed to create event: " << GetLastError() << std::endl;
            delete overlapped;
            return false;
        }

        DWORD bytes = 0;
        BOOL status = DeviceIoControl(hDevice, IOCTL_IOCSQ_WAIT_FOR_EVENT, nullptr, 0, nullptr, 0, &bytes, overlapped);
        if (!status && GetLastError() != ERROR_IO_PENDING)
        {
            std::cout << "[*] Failed in DeviceIoControl: " << GetLastError() << std::endl;
            CloseHandle(overlapped->hEvent);
            delete overlapped;
            return false;
        }

        std::cout << "[*] Polling for events from driver..." << std::endl;

        DWORD waitStatus = WaitForSingleObject(overlapped->hEvent, INFINITE);
        if (waitStatus == WAIT_OBJECT_0)
        {
            std::cout << "[*] Event signaled, processing..." << std::endl;

            // std::thread set_user_dialog_thread(communicate::send_response, hDevice);
            // set_user_dialog_thread.detach();
        }
        else
        {
            std::cout << "[*] Wait error: " << GetLastError() << std::endl;
            CloseHandle(overlapped->hEvent);
            delete overlapped;
            break;
        }

        CloseHandle(overlapped->hEvent);
        delete overlapped;
    }

    return true;
}


bool communicate::trigger_event(HANDLE hDevice)
{
    OVERLAPPED* overlapped = new OVERLAPPED();
    if (!overlapped)
        return false;

    overlapped->hEvent = CreateEventW(nullptr, true, false, nullptr);
    if (!overlapped->hEvent)
    {
        std::cout << "[*] Failed to create event: " << GetLastError() << std::endl;
        delete overlapped;
        return false;
    }

    DWORD bytes = 0;
    BOOL status = DeviceIoControl(hDevice, IOCTL_IOCSQ_TRIGGER_EVENT, nullptr, 0, nullptr, 0, &bytes, overlapped);
    if (!status && GetLastError() != ERROR_IO_PENDING)
    {
        std::cout << "[*] Failed in DeviceIoControl: " << GetLastError() << std::endl;
        CloseHandle(overlapped->hEvent);
        delete overlapped;
        return false;
    }

    std::cout << "[*] waiting for driver to process event..." << std::endl;

    WaitForSingleObject(overlapped->hEvent, INFINITE);
    
    std::cout << "[*] event processed by driver : )" << std::endl;

    CloseHandle(overlapped->hEvent);
    delete overlapped;

    return true;
}

bool communicate::send_response(HANDLE hDevice)
{
    int input_value;
    std::cout << "Enter 1 to send a positive response or 0 for a negative response: ";
    std::cin >> input_value;

    DWORD bytes = 0;
    BOOL status = DeviceIoControl(
        hDevice,
        IOCTL_IOCSQ_SEND_DIALOG_RESULT,
        &input_value,           
        sizeof(input_value),    
        nullptr,
        0,
        &bytes,
        nullptr
    );

    if (!status)
    {
        std::cout << "[*] Failed to send response. Error: " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "[*] Response sent successfully with value: " << input_value
        << ". Driver should complete the pending IOCTL now." << std::endl;
    return true;
}