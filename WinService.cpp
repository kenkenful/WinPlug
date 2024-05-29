#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>

#define BEEPSERVICE_NAME "BeepServices"

SERVICE_STATUS_HANDLE g_ssh = NULL;
HANDLE g_event = NULL;
HANDLE g_event2 = NULL;
DWORD g_state = SERVICE_STOPPED;

void WINAPI BeepServiceMain(DWORD, LPTSTR*);
void WINAPI BeepServiceHandler(DWORD);

void WriteLog(const std::string& message) {
	std::ofstream logFile("C:\\log.txt", std::ios::app);
	if (!logFile.is_open()) {
		std::cerr << "can not open logfile" << std::endl;
		return;
	}
	logFile << message << std::endl;
	logFile.close();
}


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
		WriteLog("Ctrl-C event\n\n");
		Beep(750, 300);
		return TRUE;

		// CTRL-CLOSE: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		Beep(600, 200);
		WriteLog("Ctrl-Close event\n\n");
		return TRUE;

		// Pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
		Beep(900, 200);
		WriteLog("Ctrl-Break event\n\n");
		return FALSE;

	case CTRL_LOGOFF_EVENT:
		Beep(1000, 200);
		WriteLog("Ctrl-Logoff event\n\n");
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		Beep(750, 500);
		WriteLog("Ctrl-Shutdown event\n\n");
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
		WriteLog("hpt plug ");
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
	char serviceName[MAX_PATH] = { 0 };
	strcpy(serviceName, "BeepService");

	SERVICE_TABLE_ENTRY services[] = {
		{serviceName, BeepServiceMain},
		{NULL, NULL}
	};

	if (!StartServiceCtrlDispatcher(services)) {
		MessageBox(NULL,
			"error occurred in BeepService.",
			"BeepService error",
			MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);
	}

	return 0;
}

void WINAPI BeepServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	// regsiter control handler
	g_ssh = RegisterServiceCtrlHandler(
		BEEPSERVICE_NAME, BeepServiceHandler);
	if (NULL == g_ssh) {
		MessageBox(NULL,
			"Couldn't start BeepService.",
			"BeepService error",
			MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);
		return;
	}

	// create event object
	g_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_event2 = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_state = SERVICE_RUNNING;

	// report initialize
	SERVICE_STATUS status = {
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_RUNNING,
		SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE,
		0, 0, 0, 0 };

	SetServiceStatus(g_ssh, &status);

	WNDCLASS sampleClass{ 0 };
	sampleClass.lpszClassName = TEXT("CtrlHandlerSampleClass");
	sampleClass.lpfnWndProc = WindowProc;

	if (!RegisterClass(&sampleClass))
	{
		WriteLog("\nERROR: Could not register window class");
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
		WriteLog("\nERROR: Could not create window");
	}

	ShowWindow(hwnd, SW_HIDE);

	if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
	{
		
		while(true)
		{
			if (SERVICE_RUNNING == g_state) {
				WriteLog("running in BeepServiceMain.\n");
			}
			else if (SERVICE_PAUSED == g_state) {
				WriteLog("paused in BeepServiceMain.\n");
				SetEvent(g_event2); 
				WaitForSingleObject(g_event, INFINITE);     
			}
			else if (SERVICE_STOPPED == g_state) {
				WriteLog("stopped in BeepServiceMain.\n");
				break;
			}

			MSG msg;

			while (g_state == SERVICE_RUNNING) {
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				Sleep(10);
			}

			WriteLog("escape loop.\n");
		}
	}
	else
	{
		WriteLog("\nERROR: Could not set control handler");
	}

	WriteLog("Terminated BeepService service.\n");
	
	SetEvent(g_event2);  // notify control handler.
}

void WINAPI BeepServiceHandler(DWORD fdwControl)
{
	char buf[256];

	SERVICE_STATUS status = {
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_RUNNING,
		SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE,
		0, 0, 0, 0 };

	switch (fdwControl) {
	case SERVICE_CONTROL_PAUSE:
		g_state = SERVICE_PAUSED;
		WriteLog("pausing in BeepServiceHandler.\n");
		
		// サービスハンドラに通知
		SetEvent(g_event);
		// サービスハンドラからの完了通知を待機
		WaitForSingleObject(g_event2, INFINITE);
		break;

	case SERVICE_CONTROL_CONTINUE:
		g_state = SERVICE_RUNNING;
		WriteLog("restarting in BeepServiceHandler.\n");
		// サービスハンドラに通知
		SetEvent(g_event);
		// サービスハンドラからの完了通知を待機
		WaitForSingleObject(g_event2, INFINITE);
		break;

	case SERVICE_CONTROL_STOP:
		g_state = SERVICE_STOPPED;
		WriteLog("stopped in BeepServiceHandler.\n");
	//	PostQuitMessage(0);

		// サービスハンドラに通知
		SetEvent(g_event);
		// サービスハンドラからの完了通知を待機
		WaitForSingleObject(g_event2, INFINITE);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		sprintf(buf, "Unknown control code: 0x%08lx\n", fdwControl);
		OutputDebugString(buf);
	}

	status.dwCurrentState = g_state;
	SetServiceStatus(g_ssh, &status);
	return;
}
