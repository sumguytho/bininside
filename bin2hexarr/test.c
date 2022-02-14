#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

unsigned char bin2hexarr[] = {
#include "bin2hexarr.hexarr"
};

int main() {
	FILE *newf = fopen("bin2hexarrexport.exe", "w+b");
	unsigned long long arrsize = sizeof(bin2hexarr);
	fwrite(bin2hexarr, arrsize, 1, newf);
	fclose(newf);
}