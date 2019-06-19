// Zetta!.cpp : Defines the entry point for the application.
//

#include "pch.h"

#include "Zetta!.h"

#include "resource.h"
#include "MainWnd.h"
#include "Patch.h"

void GetFileVersion(LPCTSTR fname, LPTSTR ver) {
    auto vsize = GetFileVersionInfoSize(fname, NULL);
    LPVOID data = malloc(vsize);
    GetFileVersionInfo(fname, 0, vsize, data);
    LPBYTE buffer;
    UINT size;
    if (VerQueryValue(data, _T("\\"), (VOID FAR* FAR*)&buffer, &size)) {
        if (size) {
            VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)buffer;
            if (verInfo->dwSignature == 0xfeef04bd) {
                wsprintf(ver, _T("%d.%d.%d.%d"),
                    (verInfo->dwFileVersionMS >> 16) & 0xffff,
                    (verInfo->dwFileVersionMS >> 0) & 0xffff,
                    (verInfo->dwFileVersionLS >> 16) & 0xffff,
                    (verInfo->dwFileVersionLS >> 0) & 0xffff
                );
            }
        }
    }
    free(data);
}

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT) {
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);
    CMainWnd wndMain;
    if (wndMain.Create(NULL, CWindow::rcDefault, _T("Zetta!"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, WS_EX_OVERLAPPEDWINDOW) == NULL) {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }
    wndMain.ShowWindow(nCmdShow);
    wndMain.UpdateWindow();
    int nRet = theLoop.Run();
    _Module.RemoveMessageLoop();
    return nRet;
}

int CALLBACK _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow) {
    gPatch.Load("data.json");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#if (_WIN32_IE >= 0x0300)
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(iccx);
    iccx.dwICC = ICC_BAR_CLASSES;	// change to support other controls
    ::InitCommonControlsEx(&iccx);
#else
    ::InitCommonControls();
#endif

    _Module.Init(NULL, hInstance);
    int nRet = Run(lpCmdLine, nCmdShow);
    _Module.Term();
    WSACleanup();

    return 0;
}
