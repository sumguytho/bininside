#include <fstream>
#include <iostream>
#include <cstring>

#include "PEImage.hpp"

PEImage::PEImage(const char filename[]) {
	parse(filename);
}

void PEImage::parse(const char filename[]) {
	inputFile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!inputFile.is_open())
		return;

	// DOS header
	inputFile.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));

	// DOS stub
	dosDataSize = dosHeader.e_lfanew - sizeof(dosHeader);
	dosDataPtr = std::make_unique<unsigned char[]>(dosDataSize);
	inputFile.read(reinterpret_cast<char*>(dosDataPtr.get()), dosDataSize);
	
	// PE signature, COFF file header, (not) optional (in the image) header
	inputFile.read(reinterpret_cast<char*>(&PEHeaders), sizeof(PEHeaders));
	
	sections.resize(PEHeaders.FileHeader.NumberOfSections);

	unsigned int sectionSize, prevPos;
	for (int i = 0; i < PEHeaders.FileHeader.NumberOfSections; i++) {
		// section header table
		inputFile.read(reinterpret_cast<char*>(&sections[i].header), sizeof(sections[i].header));

		// section data
		/*sectionSize = max(sections[i].header.SizeOfRawData, sections[i].header.Misc.VirtualSize);
		sections[i].dataPtr = std::make_unique<unsigned char[]>(sectionSize);
		prevPos = inputFile.tellg();
		inputFile.seekg(sections[i].header.PointerToRawData, std::ios::beg);
		inputFile.read(reinterpret_cast<char*>(sections[i].dataPtr.get()), sectionSize);

		inputFile.seekg(prevPos, std::ios::beg);*/
	}
	fileLoaded = true;
}

void PEImage::readwrite(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize, bool read) {
	if (!isLoaded()) return;

	auto section = sectionByName(sectionName);
	if (!section) return;

	RVA += section->header.PointerToRawData;
	
	if (read) {
		inputFile.seekg(RVA, std::ios::beg);
		inputFile.read(reinterpret_cast<char*>(data), dataSize);
	}
	else {
		inputFile.seekp(RVA, std::ios::beg);
		inputFile.write(reinterpret_cast<char*>(data), dataSize);
	}
}

void PEImage::write(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize) {
	readwrite(sectionName, RVA, data, dataSize, false);
}

void PEImage::read(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize) {
	readwrite(sectionName, RVA, data, dataSize, true);
}

PEImage::PESection *PEImage::sectionByName(const char* name) {
	if (!isLoaded()) return nullptr;

	char buf[9];
	for (int i = 0; i < PEHeaders.FileHeader.NumberOfSections; i++) {
		memset(buf, 0, _countof(buf));
		memcpy(buf, sections[i].header.Name, 8);

		if (!strcmp(buf, name))
			return &sections[i];
	}
	return nullptr;
}

void PEImage::release() {
	inputFile.close();
	fileLoaded = false;
}

bool PEImage::isLoaded() {
	return fileLoaded;
}
