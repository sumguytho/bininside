#include <iostream>

#include "PEImage.hpp"

int main(int argc, char *argv[]) {
	// [0] target section RVA syncPID caller

	if (argc < 6) {
		std::cout << "5 arguments are expected";
		return !0;
	}

	unsigned syncPID = 0;
	if (!sscanf_s(argv[4], "%d", &syncPID, 4)) {
		std::cout << "Can't resolve syncPID " << argv[3] << std::endl;
		return !0;
	}

	// wait until the process of the image is closed
	HANDLE syncProc = OpenProcess(SYNCHRONIZE, false, syncPID);
	if (syncProc != NULL && syncProc != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(syncProc, INFINITE);
		CloseHandle(syncProc);
	}
	
	PEImage application(argv[1]);
	if (!application.isLoaded()) {
		std::cout << "Can't open " << argv[1] << std::endl;
		return !0;
	}
	
	unsigned RVA = 0;
	if (!sscanf_s(argv[3], "0x%x", &RVA, 4) && !sscanf_s(argv[3], "%x", &RVA, 4)) {
		std::cout << "Can't resolve RVA " << argv[3] << std::endl;
		return !0;
	}

	// modify the image
	unsigned num;
	application.read(argv[2], RVA, reinterpret_cast<unsigned char*>(&num), 4);
	num++;
	// !!!
	application.write(argv[2], RVA, reinterpret_cast<unsigned char*>(&num), 4);

	char buf[1024];
	unsigned curPID = GetCurrentProcessId();
	if (sprintf_s(buf, "\"%s\" cleanup %d", argv[5], curPID) == -1) {
		std::cout << "Can't prepare command line." << std::endl;
		return !0;
	}
	
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	application.release();

	// signal the image for cleanup
	// obviously, the image should implement cleanup request
	if (!CreateProcessA(
		NULL,
		buf,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)) {
		std::cout << "Can't create process with cmdline " << buf << std::endl;
		return !0;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}