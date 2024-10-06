
#include <windows.h>
#include <iostream>

#define MYDATA_STRING 100;

typedef struct {
    bool write_access;
    UINT8 bus;
    UINT8 dev;
    UINT8 func;
    UINT8 data;
    HWND target;
}PCI_ACCESS;

int main()
{
	HWND hWnd = FindWindow(0, L"WinService");
    if (hWnd != 0) {
        std::cout << " find WinServer" << std::endl;
         UINT16 a = 1;

        COPYDATASTRUCT cds;
        cds.dwData = MYDATA_STRING;
        cds.cbData = sizeof(UINT16);
        a++;
        cds.lpData = (void*)&a;
        LRESULT  ret= SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
        std::cout << ret << std::endl;
    }
    else {
        std::cout << " cannot find WinServer" << std::endl;
    }
    system("pause");
}




