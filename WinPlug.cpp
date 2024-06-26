﻿#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <iostream>

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        return TRUE;

        // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        Beep(600, 200);
        printf("Ctrl-Close event\n\n");
        return TRUE;

        // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
        Beep(900, 200);
        printf("Ctrl-Break event\n\n");
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        Beep(1000, 200);
        printf("Ctrl-Logoff event\n\n");
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        Beep(750, 500);
        printf("Ctrl-Shutdown event\n\n");
        return FALSE;

    default:
        return FALSE;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_DEVICECHANGE: {
        std::cout << "hpt plug " << std::endl;
        return true;
    }
    case WM_QUERYENDSESSION:
    {
        // Check `lParam` for which system shutdown function and handle events.
        // See https://learn.microsoft.com/windows/win32/shutdown/wm-queryendsession
        return TRUE; // Respect user's intent and allow shutdown.
    }
    case WM_ENDSESSION:
    {
        // Check `lParam` for which system shutdown function and handle events.
        // See https://learn.microsoft.com/windows/win32/shutdown/wm-endsession
        return 0; // We have handled this message.
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int main(void)
{
    WNDCLASS sampleClass{ 0 };
    sampleClass.lpszClassName = TEXT("CtrlHandlerSampleClass");
    sampleClass.lpfnWndProc = WindowProc;

    if (!RegisterClass(&sampleClass))
    {
        printf("\nERROR: Could not register window class");
        return 2;
    }

    HWND hwnd = CreateWindowEx(
        0,
        sampleClass.lpszClassName,
        TEXT("Console Control Handler Sample"),
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    if (!hwnd)
    {
        printf("\nERROR: Could not create window");
        return 3;
    }

    ShowWindow(hwnd, SW_HIDE);

    if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        printf("\nThe Control Handler is installed.\n");
        printf("\n -- Now try pressing Ctrl+C or Ctrl+Break, or");
        printf("\n    try logging off or closing the console...\n");
        printf("\n(...waiting in a loop for events...)\n\n");

        // Pump message loop for the window we created.
        MSG msg{};
        while (GetMessage(&msg, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return 0;
    }
    else
    {
        printf("\nERROR: Could not set control handler");
        return 1;
    }
}