#include <iostream>
#include <fstream>

#include "PEImage.hpp"

void SyncWithPID(DWORD pid) {
	HANDLE syncProc = OpenProcess(SYNCHRONIZE, false, pid);
	if (syncProc != NULL && syncProc != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(syncProc, INFINITE);
		CloseHandle(syncProc);
	}
}

bool IndependentProcess(char* cmdLine) {
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessA(
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

int main(int argc, char *argv[]) {
	if (argc < 6) {
		std::cout << "Usage: <this_app> <target> <section> <RVA> <syncPID> <cleaner>" << std::endl;
		return !0;
	}

	std::cout << std::endl << "argv:" << std::endl;
	for (int i = 0; i < argc; i++)
		std::cout << "[" << i << "] = " << argv[i] << std::endl;

	unsigned syncPID = 0;
	if (!sscanf_s(argv[4], "%d", &syncPID)) {
		std::cout << std::endl << "Can't resolve syncPID " << argv[3] << std::endl;
		return !0;
	}

	// wait until the process of the image is closed
	SyncWithPID(syncPID);
	
	char buf[1024];
	PEImage application(argv[1]);
	if (application) {
		unsigned RVA = 0;
		if (!sscanf_s(argv[3], "0x%x", &RVA) && !sscanf_s(argv[3], "%x", &RVA)) {
			std::cout << std::endl << "Can't resolve RVA " << argv[3] << std::endl;
			return !0;
		}

		// check integrity, somewhat
		unsigned char someStruct[12];	// should be 21 but adding string to a structure
										// breaks its layout
		// it's a cop out but it's gonna be fine, probably
		unsigned bytesToCheck = 0x100;
		bool bypassCheck = false;
		if (RVA == 0xcccccccc) {
			auto prevFl = std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout.setf(std::ios::showbase);
			std::cout << std::endl << "RVA is set to 0xcccccccc, traversing first " << bytesToCheck
				<< " bytes of specified section for a 12 byte site match." << std::endl;
			int i = 0;
			for (; i < bytesToCheck; i += 4) {
				application.read(argv[2], i, someStruct, _countof(someStruct));
				if (*(unsigned*)someStruct == 0xfadedbee &&
					*(unsigned*)(someStruct + 8) == RVA) {
					std::cout << "Found a match at " << argv[2] << '+'
						<< i << ", incrementing " << argv[2] << '+' << i + 4
						<< std::endl;
					// next check expects RVA of launch number, not site start
					RVA = i + 4;
					bypassCheck = true;
					break;
				}
			}
			std::cout.setf(prevFl);
		}

		application.read(argv[2], RVA - 4, someStruct, _countof(someStruct));
		if (bypassCheck ||
			*(unsigned*)someStruct == 0xfadedbee &&
			*(unsigned*)(someStruct + 8) == RVA /*&&
			!strcmp((char*)someStruct + 12, argv[2])*/) { 
			// modify the image
			unsigned num = *(unsigned*)(someStruct + 4);
			num++;
			// !!!
			application.write(argv[2], RVA, reinterpret_cast<unsigned char*>(&num), 4);
		}
		else {
			sprintf_s(buf,	"The 12 byte site is invalid:\n"
							"signature\n"
							"expected: 0x%x real: 0x%x\n"
							"RVA\n"
							"expected: 0x%x real: 0x%x\n"
							/*"section\n"
							"expected: %s real: %s.8\n"*/
							"\nNo modification performed\n",	0xfadedbee,	*(unsigned*)someStruct,
																RVA ,		*(unsigned*)(someStruct + 8)/*, 
																argv[2],	someStruct + 12*/);
			std::cout << std::endl << buf;
		}
	}
	else {
		std::ifstream target;
		target.open(argv[1]);
		if (!target.is_open()) {
			sprintf_s(buf, "Can't access %s. \r%s is left hanging.", argv[1], argv[0]);
			MessageBox(NULL, buf, "", MB_OK);
			return !0;
		}
		target.close();
		std::cout << std::endl << "Can't open " << argv[1] << " for modification." << std::endl;
	}
	application.release();

	unsigned curPID = GetCurrentProcessId();
	// [0] cleanup syncPID cleaner
	if (sprintf_s(buf, "\"%s\" cleanup %d \"%s\"", argv[5], curPID, argv[0]) == -1) {
		std::cout << std::endl << "Can't prepare cleanup command line." << std::endl;
		return !0;
	}

	std::cout << std::endl << "Executing: " << buf << std::endl;

	// signal the image for cleanup
	// obviously, the image should implement cleanup request handler
	if (!IndependentProcess(buf)) {
		std::cout << std::endl << "Can't create new process, error code: " << GetLastError() << std::endl;
	}
}