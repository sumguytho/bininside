#include "framework.h"

someStruct someData;

static unsigned char simple_pe_editor_data[] = {
		#include "simple-pe-editor.hexarr"
};

void SyncWithPID(DWORD pid) {
	HANDLE syncProc = OpenProcess(SYNCHRONIZE, false, pid);
	if (syncProc != NULL && syncProc != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(syncProc, INFINITE);
		CloseHandle(syncProc);
	}
}

bool IndependentProcess(TCHAR* cmdLine) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess(
		NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return true;
	}
	return false;
}

void HandleCleanupRequest(wchar_t exePath[], unsigned bufSize) {
	LPCWSTR cmdLine = GetCommandLine();
	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(cmdLine, &nArgs);

	if (!szArglist) {
		wchar_t buf[1024];
		swprintf_s(buf, L"Unable to make argv for %s", cmdLine);
		MessageBox(NULL, buf, L"", MB_OK);
		return;
	}

	int i;
	for (i = 0; i < nArgs; i++)
		if (!wcscmp(szArglist[i], L"cleanup"))
			break;

	wcscpy_s(exePath, bufSize, szArglist[0]);

	wchar_t buf[1024];
	bool cleanupHandled = false;
	// if i equals nArgs cleanup option hasn't been specified and messageboxes needn't be displayed
	if (i < nArgs) {
		if (nArgs > i + 2) {
			cleanupHandled = true;
			DWORD pid;
			if (swscanf_s(szArglist[i + 1], L"%d", &pid)) {
				SyncWithPID(pid);

				std::ifstream ifile(szArglist[i + 2], std::ios::in | std::ios::binary);
				// do not test the stack
				static unsigned char fileContents[_countof(simple_pe_editor_data)];
				if (ifile.is_open()) {
					ifile.read(reinterpret_cast<char*>(fileContents), sizeof(fileContents));
					if (!memcmp(fileContents, simple_pe_editor_data, sizeof(simple_pe_editor_data))) {
						ifile.close();
						if (_wremove(szArglist[i + 2])) {
							swprintf_s(buf, L"Unable to remove %s", szArglist[i + 2]);
							MessageBox(NULL, buf, L"", MB_OK);
						}
					}
					else {
						swprintf_s(buf, L"%s is not a valid copy of simple-pe-editor. Cleanup failed.", szArglist[i + 2]);
						MessageBox(NULL, buf, L"", MB_OK);
					}
				}
				else {
					swprintf_s(buf, L"Unable to access %s", szArglist[i + 2]);
					MessageBox(NULL, buf, L"", MB_OK);
				}
			}
			else {
				swprintf_s(buf, L"Can't parse pid %s", szArglist[i + 1]);
				MessageBox(NULL, buf, L"", MB_OK);
			}
		}
		else
			MessageBox(NULL, L"Cleanup should be followed by syncPID and target", L"", MB_OK);
	}
		
	LocalFree(szArglist);

	if (cleanupHandled) exit(0);
}

DWORD GetProcessByName(const wchar_t* process_name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process = { 0 };
	process.dwSize = sizeof(PROCESSENTRY32);
	DWORD proc_id = 0;
	if (Process32First(snapshot, &process))
	{
		while (Process32Next(snapshot, &process))
		{
			if (!_wcsicmp(process.szExeFile, process_name) && process.th32ProcessID != GetCurrentProcessId())
			{
				proc_id = process.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	auto sa = GetLastError();
	return proc_id;
}

// actually, this is no longer a concern but i'll just let this stay here
void LookForCopies(wchar_t* exeName) {
	if(GetProcessByName(exeName))
		MessageBox(NULL,	L"Running multiple instances of this image\r"
							L"may cause a deadlock or smth, idk. \r"
							L"Running different copies of this image is fine tho.", L"", MB_OK);
}

void TheEpicPart(wchar_t* exePath) {
	wchar_t tempPath[MAX_PATH], fileFmt[MAX_PATH] = L"%s\\malicious-software#%d.exe", fileName[MAX_PATH];
	unsigned fileNum = 0, maxAttempts = 100;
	std::ofstream ofile;
	std::ifstream ifile;

	if (GetTempPath(_countof(tempPath), tempPath)) {
		bool ifileOpened, ofileOpened;
		do {
			swprintf_s(fileName, fileFmt, tempPath, fileNum);
			fileNum++;
			
			ifile.open(fileName, std::ios::in);
			ifileOpened = ifile.is_open();
			ifile.close();
			ifile.clear();
			
			ofile.open(fileName, std::ios::out | std::ios::app);
			ofileOpened = ofile.is_open();
			ofile.close();
			ofile.clear();
			// doesn't exist but can be created
		} while ((ifileOpened || !ofileOpened) && fileNum < maxAttempts);

		ofile.open(fileName, std::ios::out | std::ios::binary);
		if (ofile.is_open()) {
			ofile.write(reinterpret_cast<const char*>(simple_pe_editor_data), sizeof(simple_pe_editor_data));
			ofile.close();
			
			// converting section name to wide char string
			wchar_t sectionw[_countof(/*someData.*/section)];
			memset(sectionw, 0, sizeof(section));
			for (int i = 0; i < _countof(section) - 1; i++) {
				sectionw[i] = btowc(/*someData.*/section[i]);
			}

			wchar_t cmdLine[1024];
			// <this_app> <target> <section> <RVA> <syncPID> <cleaner>
			if (swprintf_s(cmdLine, L"\"%s\" \"%s\" %s %x %d \"%s\"",
				fileName, exePath, sectionw, someData.RVA, GetCurrentProcessId(), exePath) != -1) {
				if (!IndependentProcess(cmdLine)) {
					wchar_t errStr[1024];
					swprintf_s(errStr, L"Can't start new process for %s (left hanging), error code: %d",
						fileName, GetLastError());
					MessageBox(NULL, errStr, L"", MB_OK);
				}
			}
			else 
				MessageBox(NULL, L"Can't prepare cmdline for new process.", L"", MB_OK);
		}
		else
			MessageBox(NULL, L"Can't access temporary path after 100 attempts.", L"", MB_OK);
	}
	else
		MessageBox(NULL, L"Can't get temporary path.", L"", MB_OK);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
	wchar_t exePath[MAX_PATH];

	HandleCleanupRequest(exePath, MAX_PATH);

	LookForCopies(wcsrchr(exePath, L'\\') + 1);

    int retval = DoFancyStuff(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	TheEpicPart(exePath);

    return retval;
}