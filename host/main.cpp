// ReSharper disable CppClangTidyPerformanceAvoidEndl

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdlib>

#include <iostream>

#include "gamepad.h"


int main()
{
    // Set a console control handler to stop the application
    static bool s_running = true;
    if (!SetConsoleCtrlHandler([](DWORD) { s_running = false; return TRUE; }, TRUE))
    {
        std::cerr << "Failed to set console control handler" << std::endl;
        return EXIT_FAILURE;
    }

    // Create a gamepad and add a controller
    remote_hid::gamepad gamepad;
    auto const id = gamepad.add_controller();

    while (s_running)  // NOLINT(bugprone-infinite-loop)
    {
        // Read the report from the console
        XUSB_REPORT report{};
        std::cout << "Buttons(Hex) LeftTrigger RightTrigger LeftThumbX LeftThumbY RightThumbX RightThumbY" << std::endl;
        std::cin >> std::hex >> report.wButtons 
                 >> std::dec >> report.bLeftTrigger
                 >> report.bRightTrigger
                 >> report.sThumbLX
                 >> report.sThumbLY
                 >> report.sThumbRX
                 >> report.sThumbRY;

        // Update the target with the report
        gamepad.update_controller(id, report);
    }
}