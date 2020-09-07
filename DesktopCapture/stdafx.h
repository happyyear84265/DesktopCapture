// stdafx.h : 可在此標頭檔中包含標準的系統 Include 檔，
// 或是經常使用卻很少變更的
// 專案專用 Include 檔案
//

#pragma once
#pragma comment(lib, "Gdiplus.lib")
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <Commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <Psapi.h>
#include <time.h>
#include <atlbase.h>
#include <atlenc.h>
#include <atlstr.h>
#include <io.h>
#include <assert.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>
#include <thread>

#include "resource.h"
#include "AviFile.h"
// TODO: 在此參考您的程式所需要的其他標頭

using namespace Gdiplus;
#pragma comment( lib, "gdiplus" )

typedef struct tagMonData {
	int current;
	MONITORINFOEXW* info;
} MonData;
