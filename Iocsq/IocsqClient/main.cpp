#include <iostream>
#include <Windows.h>
#include <thread>
#include <string>
#include "communicate.h"

int main()
{
  

    HANDLE hDevice = CreateFileW(L"\\\\.\\IoCsq", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) {
        std::cout << "[*] Failed to obtain device handle" << std::endl;
        return -1;
    }

    std::thread poll_events_thread(communicate::wait_for_event, hDevice);
    
    // DEBUG CODE 
    std::string input;
    std::cout << "Type anything: ";
    std::getline(std::cin, input);

    communicate::trigger_event(hDevice);

    // END DEBUG CODE 
    


    poll_events_thread.join();
  
    CloseHandle(hDevice);

    return 0;
}