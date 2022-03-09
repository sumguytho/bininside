// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <stdio.h>
#include <shellapi.h>
#include <TlHelp32.h>
#include <fstream>

#pragma pack(1)
struct someStruct {
    unsigned signature = 0xfadedbee;
    unsigned launchCount = 0;
    // specified manually, CFF explorer or smth
    // there might be a smart way to do that
    unsigned RVA = 0xcccccccc;
};

// including it in structure causes weird stuff to happen
const char section[9] = ".data";

extern someStruct someData;

int APIENTRY DoFancyStuff(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow);
