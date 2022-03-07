#ifndef __PEIMAGE__H__

#define __PEIMAGE__H__

#include "Image.hpp"

#include <Windows.h>
#include <vector>
#include <fstream>

class PEImage :
    public Image
{
private:
    IMAGE_DOS_HEADER dosHeader;
    std::unique_ptr<unsigned char[]> dosDataPtr;
    unsigned int dosDataSize;
    IMAGE_NT_HEADERS32 PEHeaders;
    
    struct PESection {
        IMAGE_SECTION_HEADER header;
        std::unique_ptr<unsigned char[]> dataPtr;
    };
    std::vector<PESection> sections;

    std::fstream inputFile;
    bool fileLoaded;

    PESection *sectionByName(const char *name);
    void readwrite(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize, bool read);
public:
    PEImage() = default;
    PEImage(const char filename[]);

    void parse(const char filename[]);
    bool isLoaded();
    void release();
    void write(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize);
    void read(const char* sectionName, DWORD RVA, unsigned char* data, unsigned int dataSize);
};

#endif
