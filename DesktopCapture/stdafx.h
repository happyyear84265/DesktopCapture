// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ�
// �M�ױM�� Include �ɮ�
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
// TODO: �b���Ѧұz���{���һݭn����L���Y

using namespace Gdiplus;
#pragma comment( lib, "gdiplus" )

typedef struct tagMonData {
	int current;
	MONITORINFOEXW* info;
} MonData;
