#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>

void variadiccleanup(const char *fmt, ...) {
	int i = 0;
	va_list args;

	va_start(args, fmt);
	while (fmt[i]) {
		switch (fmt[i]) {
			//handle
		case 'h':
		case 'H': {
			HANDLE h = va_arg(args, HANDLE);
			if (h && h != INVALID_HANDLE_VALUE)
				CloseHandle(h);
		} break;
			//stream
		case 'f':
		case 'F': {
			FILE* f = va_arg(args, FILE*);
			if (f)
				fclose(f);
		} break;
			//view of file mapping
		case 'v':
		case 'V': {
			LPVOID v = va_arg(args, LPVOID);
			if (v)
				UnmapViewOfFile(v);
		} break;
		}
		i++;
	}
	va_end(args);
}

int main(int argc, char *argv[]) {
	FILE* ofile = stdout;

	if (argc < 2) {
		printf(
			"Usage: bin2hexarr <path_to_input_file> [<path_to_output_file>]\n"
			"If output file is not specified stdout is assumed\n"
			"Outputs C array byte sequence for given file, doesn't include array definition\n"
		);
		return EXIT_SUCCESS;
	}

	if (argc > 2) {
		ofile = freopen(argv[2], "w+", stdout);
		if (!ofile)
			fprintf(stderr, "can't redirect output to %s\n", argv[2]);
	}
	
	SYSTEM_INFO sysinfo;
	HANDLE hfile, hmapping;
	DWORD fileszlow, fileszhigh, buffersz, diff, colsz = 16, colcnt = 0;
	PUCHAR filedata;
	ULONGLONG filesz, fileoffset = 0;

	hfile = CreateFile(
		argv[1], 
		GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
	);

	if (hfile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "unable to open file %s\n", argv[1]);
		variadiccleanup("f", ofile);
		return EXIT_FAILURE;
	}
	GetSystemInfo(&sysinfo);

	//mapping offset must be a multiple of granularity
	buffersz = sysinfo.dwAllocationGranularity;

	fileszlow = GetFileSize(hfile, &fileszhigh);
	filesz = (ULONGLONG)fileszhigh << 32 | fileszlow;
	if (!filesz) {
		fprintf(stderr, "input file is empty\n");
		variadiccleanup("hf", hfile, ofile);
		return EXIT_FAILURE;
	}
	hmapping = CreateFileMapping(
		hfile,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
	);
	if (!hmapping) {
		fprintf(stderr, "can't create mapping object for input file\n");
		variadiccleanup("hf", hfile, ofile);
		return EXIT_FAILURE;
	}
	do {
		diff = filesz - fileoffset > buffersz ? buffersz : filesz - fileoffset;
		filedata = (PUCHAR)MapViewOfFile(
			hmapping,
			FILE_MAP_READ,
			fileoffset >> 32,
			fileoffset & 0xffffffff,
			diff
		);
		if (!filedata) {
			fprintf(stderr, "can't map view of file mapping into address space\n");
			variadiccleanup("hhf", hmapping, hfile, ofile);
			return EXIT_FAILURE;
		}
		if (fileoffset)
			printf(", ");
		for (int i = 0; i < diff; i++) {
			if (i)
				printf(", ");

			if (colcnt >= colsz) {
				printf("\n");
				colcnt = 0;
			}
			printf("0x%.2x", filedata[i]);
			colcnt++;
		}
		fileoffset += diff;
		UnmapViewOfFile(filedata);
	} while (filesz - fileoffset != 0);

	variadiccleanup("hhf", hmapping, hfile, ofile);
}