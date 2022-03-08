#include "framework.h"

someStruct someData;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    
    int nArgs;
    LPWSTR* szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);

    int i;
    for(i = 0; i < nArgs; i++)
        if(!wcscmp(szArglist[i], L"cleanup"))
            break;
    if (i < nArgs)
        if (i + 2 >= nArgs)
            MessageBox(NULL, L"Cleanup should be followed by syncPID and target", L"", MB_OK);
        else
            MessageBox(NULL, L"Hmmmm... It can be arranged", L"", MB_OK);

    LocalFree(szArglist);

    int retval = DoFancyStuff(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    return retval;
}