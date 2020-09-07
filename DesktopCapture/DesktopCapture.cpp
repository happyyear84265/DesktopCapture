#include "stdafx.h"
using namespace std;

#define APP_PIPE_NAME		L"\\\\.\\pipe\\PipeCapture"
#define APP_MUTEX_NAME		L"MUTEX-CAPTURE"
#define APP_TITLE_NAME		L"Capture"
#define APP_CLASS_NAME		L"Capture_Class"

vector<float>		MonitorScaleList;
UINT				width,height;
int					orgX = 0, orgY = 0,endx = 0, endy = 0;
int					SPACResult, ImagesbecomeVideoInterval, ImageQualityResult, processcount,VideoQualityResult;
bool				gbEnable = false, gbByApp = false, gbDeleteDupImage = false, gbVideoEnable = false;
CLSID				clsid;
EncoderParameters	encoderParameters;
thread*				trd;
bool				gostop = true;
CString				wsPolicyFile;
HINSTANCE			hInst;										// current instance
TCHAR				szTitle[MAX_PATH];							// The title bar text
TCHAR				szWindowClass[MAX_PATH];					// the main window class name
int					nCount = 0;									//紀錄擷取桌面次數
int					firstnum = 1;								//記錄檔開頭添加編碼判別
CString				caputre_LOGsave_path;						//記錄檔位置
CAviFile*			avi = nullptr;								//定義avi指標
HBITMAP				hDesktopCompatibleBitmap = NULL;
HDC					hDesktopCompatibleDC = NULL;
HDC					hDesktopDC = GetDC(GetDesktopWindow());
HWND				AllHwnd;
UINT_PTR			TimeEvent1, TimeEvent2, TimeEvent3;
HANDLE				g_hTrdNamePipe = NULL;
HBITMAP				HBitmapRight;
BITMAPINFO			BitmapInfoRight = { 0 };

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);												//註冊表單
BOOL				InitInstance(HINSTANCE, int);														//初始化
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);												//呼叫用function
void				SaveBitmap(wstring szFilename, HBITMAP hBitmap);										//儲存BITMAP function
void				CreateDirectoryFile();
bool				CompareBitmaps(HBITMAP HBitmapLeft);							//比對前一張bitmap function
bool				DirectoryRecursive(CString token);													//刪除檔案資料夾
BOOL				LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP* phBitmap, HPALETTE* phPalette);	//讀取bmp圖檔
BOOL				Savedatatologfile(CString lpdata, BOOL deleteyn);					//記錄檔function
int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);								//JPEG 格式設定
void				savejpeg(wstring szFilename, HBITMAP hBitmap);		//儲存JPEG function
BOOL				EnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
void				changeMonitorRect();
BOOL				GetAllMonitorInfo(MonData* data);
BOOL				CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
wstring				GetProfileSettingStr(wstring wsSection, wstring wsKey, wstring wsDefStr);
bool				GetProfileSettingBool(wstring wsSection, wstring wsKey, wstring wsDefStr);
int					GetProfileSettingInt(wstring wsSection, wstring wsKey, wstring wsDefStr);

CString				ImagePath;															//是否啟動合併影片
vector<CString>		processlist;
vector<CString>		processtitlelist;
TCHAR				NAME[255];
DWORD				processid;
TCHAR				szProcessName[1024];
CString				testaa;

#define		CAPTURE_SPACING_RUNTIME		SPACResult						//擷取桌面定時間格時間
#define		VIDEO_SPACING_RUNTIME		ImagesbecomeVideoInterval		//錄製桌面定時間格時間
#define		VIDEO_FPS					1								//錄製桌面FPS(現為1秒1張)
#define		ImageQuality				ImageQualityResult				//圖片壓縮率
#define		CAPTURE_CYCLE_START			3012
#define		VIDEO_CYCLE_START			3013

VOID capture_desktop()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString capture_timbuf;
	capture_timbuf.AppendFormat(L"%d-%d-%d-%d-%d%d%d", st.wYear, st.wMonth, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CString caputre_LOGsave_filename;
	CString szBMPFileName, szJPGFileName;
	szBMPFileName.AppendFormat(L"%s\\screendata\\%sscreenshot.bmp", ImagePath, capture_timbuf);
	szJPGFileName.AppendFormat(L"%s\\jpgpicture\\%sscreenshot.jpeg", ImagePath, capture_timbuf);

	caputre_LOGsave_filename.Append(capture_timbuf);
	caputre_LOGsave_filename.Append(L"screenshot.jpeg");

	HDC hBmpFileDC = CreateCompatibleDC(hDesktopDC);
	HBITMAP	hBmpFileBitmap = CreateCompatibleBitmap(hDesktopDC, width, height);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBmpFileDC, hBmpFileBitmap);
	BitBlt(hBmpFileDC, 0, 0, width, height, hDesktopDC, orgX, orgY, SRCCOPY | CAPTUREBLT);
	SelectObject(hBmpFileDC, hOldBitmap);

	savejpeg(szJPGFileName.GetBuffer(), hBmpFileBitmap);					//呼叫儲存JPEG function
	if(gbVideoEnable)
		SaveBitmap(szBMPFileName.GetBuffer(), hBmpFileBitmap);						//呼叫儲存bitmap function
	bool deleteyn = true;
	if (gbDeleteDupImage)
	{
		deleteyn = CompareBitmaps(hBmpFileBitmap);
		if (deleteyn)
		{
			DeleteFile(szBMPFileName);
			DeleteFile(szJPGFileName);
		}
	}
	Savedatatologfile(caputre_LOGsave_filename, deleteyn);

	DeleteDC(hBmpFileDC);										//刪除桌面畫面暫存
	DeleteObject(hBmpFileBitmap);								//刪除桌面bitmap暫存
}

VOID capture_window(HDC test,RECT ttt)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString capture_timbuf;
	capture_timbuf.AppendFormat(L"%d-%d-%d-%d-%d%d%d", st.wYear, st.wMonth, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CString szBMPFileName, szJPGFileName;
	szBMPFileName.AppendFormat(L"%s\\screendata\\%sscreenshot.bmp", ImagePath, capture_timbuf);
	szJPGFileName.AppendFormat(L"%s\\jpgpicture\\%sscreenshot.jpeg", ImagePath, capture_timbuf);

	CString capture_LOGsave_filename;
	capture_LOGsave_filename.Append(capture_timbuf);
	capture_LOGsave_filename.Append(L"screenshot.jpeg");

	HDC hBmpFileDC = CreateCompatibleDC(test);
	HBITMAP	hBmpFileBitmap = CreateCompatibleBitmap(test, ttt.right-ttt.left, ttt.bottom-ttt.top);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBmpFileDC, hBmpFileBitmap);
	BitBlt(hBmpFileDC, 0, 0, ttt.right - ttt.left, ttt.bottom - ttt.top, test, orgX, orgY, SRCCOPY | CAPTUREBLT);
	SelectObject(hBmpFileDC, hOldBitmap);

	if (gbVideoEnable)
		SaveBitmap(szBMPFileName.GetBuffer(), hBmpFileBitmap);						//呼叫儲存bitmap function
	savejpeg(szJPGFileName.GetBuffer(), hBmpFileBitmap);					//呼叫儲存JPEG function

	bool deleteyn = false;
	if (gbDeleteDupImage)
	{
		deleteyn = CompareBitmaps(hBmpFileBitmap);
		if (deleteyn)
		{
			DeleteFile(szBMPFileName);
			DeleteFile(szJPGFileName);
		}
	}
	Savedatatologfile(capture_LOGsave_filename, deleteyn);
	DeleteDC(hBmpFileDC);										//刪除桌面畫面暫存
	DeleteObject(hBmpFileBitmap);								//刪除桌面bitmap暫存
}


BOOL find_main_window()
{
	while (gostop)
	{
		EnumWindows(EnumWindowsProc, NULL);
		Sleep(CAPTURE_SPACING_RUNTIME *1000);
	}
	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (GetWindow(hwnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hwnd))
	{
		(LC_ALL, "");

		GetWindowTextW(hwnd, NAME, 255);
		if (_tcscmp(NAME, L"") == 0) return true;

		GetWindowThreadProcessId(hwnd, &processid);
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processid);
		if (NULL != hProcess)
		{
			GetProcessImageFileName(hProcess, szProcessName, MAX_PATH);
			testaa = szProcessName;
			for (int q = 0; q < processcount; q++)
			{
				if (testaa.Find(processlist[q]) >= 0)
				{
					HDC test = GetDC(hwnd);
					RECT my_rect;
					GetWindowRect(hwnd, &my_rect);
					capture_window(test,my_rect);
					break;
				}
			}
		}
		CloseHandle(hProcess);
	}
	return true;
}

VOID Video_Create()
{
	SYSTEMTIME st_v;
	GetLocalTime(&st_v);
	CString capture_timbuf;
	capture_timbuf.AppendFormat(L"%d-%d-%d-%d-%d%d%d", st_v.wYear, st_v.wMonth, st_v.wMonth, st_v.wDay, st_v.wHour, st_v.wMinute, st_v.wSecond);
	WIN32_FIND_DATA FindFileData;
	WCHAR vdFullPath[MAX_PATH];
	CString vdVideoName;
	vdVideoName.AppendFormat(L"%s\\jpgpicture\\%sOutput.avi", ImagePath, capture_timbuf);
	CString vdFilePath;
	vdFilePath.AppendFormat(L"%s\\screendata\\*.bmp", ImagePath);
	CString	video_BMPload_path;
	video_BMPload_path.AppendFormat(L"%s\\screendata\\",ImagePath);			//檔案名稱複製到szVideoName

	HANDLE hFile = FindFirstFile(vdFilePath, &FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile error\n");
	}
	else
	{
		avi = new CAviFile(vdVideoName, mmioFOURCC('x', 'v', 'i', 'd'), VIDEO_FPS);
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0)
				{
					continue;
				}
			}
			HBITMAP       hBitmap, hOldBitmap;
			HPALETTE      hPalette, hOldPalette;
			HDC           hMemDC;
			BITMAP        bm;
			wsprintf(vdFullPath, L"%s\\%s", video_BMPload_path, FindFileData.cFileName);
			if (LoadBitmapFromBMPFile(vdFullPath, &hBitmap, &hPalette))
			{
				GetObject(hBitmap, sizeof(BITMAP), &bm);
				hMemDC = CreateCompatibleDC(NULL);
				hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
				hOldPalette = SelectPalette(NULL, hPalette, FALSE);
				//RealizePalette(hDC);
				BitBlt(NULL, 0, 0, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldBitmap);
				if (FAILED(avi->AppendNewFrame(hBitmap)))	//avi.AppendNewFrame(320, 240, pBits, 32)))		//執行avi添加新畫面
				{
					MessageBox(AllHwnd, avi->GetLastErrorMessage(), _T("Error Occured"), MB_OK | MB_ICONERROR);
					DestroyWindow(AllHwnd);
				}
				DeleteFile(vdFullPath);
				DeleteDC(hMemDC);
				DeleteObject(hPalette);
				DeleteObject(hBitmap);
			}
		} while (FindNextFile(hFile, &FindFileData));
		FindClose(hFile);
		if (nullptr != avi)
		{
			delete avi;
			avi = nullptr;
		}
	}
}

void CreateDirectoryFile()
{
	CString findpath;
	findpath = ImagePath.Left(ImagePath.ReverseFind('\\'));
	if (_waccess(findpath, 0) == -1)
	{
		if (!CreateDirectory(findpath, NULL))
		{
			
		}
	}
	if (_waccess(ImagePath, 0) == -1)
	{
		if (!CreateDirectory(ImagePath, NULL))
		{

		}
	}
	CString screenBMPimagePath;
	screenBMPimagePath.Append(ImagePath);
	screenBMPimagePath.Append(L"\\screendata");
	if (_waccess(screenBMPimagePath, 0) >= 0)
	{
		DirectoryRecursive(screenBMPimagePath);
	}
	CreateDirectory(screenBMPimagePath, 0);											//創建執行檔位置下的screendata資料夾

	CString screenJPGimagePath;
	screenJPGimagePath.Append(ImagePath);
	screenJPGimagePath.Append(L"\\jpgpicture");
	if (_waccess(screenJPGimagePath, 0) == -1)
	{
		CreateDirectory(screenJPGimagePath, 0);										//創建執行檔位置下的jpgpicture資料夾
	}
}

void ProfileInitial() 
{
	WCHAR ownPth[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// Use GetModuleFileName() with module handle to get the path
		GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
		PathRemoveFileSpec(ownPth);
		wsPolicyFile.AppendFormat(L"%s\\DesktopCapture.ini", ownPth);
	}

	gbEnable = GetProfileSettingBool(L"ScreenCapture", L"Enable", L"N");
	gbByApp = GetProfileSettingBool(L"ScreenCapture", L"CaptureWhenApp",L"N");	//偵測程序啟動擷圖(Y)
	gbDeleteDupImage = GetProfileSettingBool(L"ScreenCapture", L"DeleteduplicateImage", L"N");
	gbVideoEnable = GetProfileSettingBool(L"ScreenCapture", L"EnableVideo", L"N");
	ImagePath.Append(GetProfileSettingStr(L"ScreenCapture", L"ImagePath", L"").c_str());
	if (ImagePath.IsEmpty())
	{
		ImagePath = ownPth;
	}
	SPACResult = GetProfileSettingInt(L"ScreenCapture", L"CaptureInterval", L"0");
	ImagesbecomeVideoInterval = GetProfileSettingInt(L"ScreenCapture", L"ImagesbecomeVideoInterval", L"0");
	ImageQualityResult = GetProfileSettingInt(L"ScreenCapture", L"ImageQuality", L"0");
	processcount = GetProfileSettingInt(L"ScreenCapture", L"AppWindowsCount", L"0");
	VideoQualityResult = GetProfileSettingInt(L"ScreenCapture", L"videoQuality", L"0");

	if (processcount != NULL) 
	{
		for (int i = 0; i < processcount; i++) 
		{
			//... ProcessName ...
			CString processname, queryname;
			queryname.AppendFormat(L"AppProcess%d",i);
			processname = GetProfileSettingStr(L"ScreenCapture", queryname.GetBuffer(), L"").c_str();
			if (processname != L"")
			{
				processlist.push_back(processname);
			}
			//... CaptionName ...
			CString processtitle, querytitle;
			querytitle.AppendFormat(L"AppWindows%d", i);
			processtitle = GetProfileSettingStr(L"ScreenCapture", querytitle.GetBuffer(), L"").c_str();
			if (processtitle != L"") 
			{
				processtitlelist.push_back(processtitle);
			}
		}
	}
	else
	{
		if (gbByApp)
		{
			
		}
	}

	//Gdi進行初始化,不進行存不了圖
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	GetEncoderClsid(L"image/jpeg", &clsid);

	//圖片影像畫質設定
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &ImageQuality;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//確認是否多開
	HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, APP_MUTEX_NAME);
	if (hMutex == NULL) 
	{
		hMutex = CreateMutex(NULL, true, APP_MUTEX_NAME);
	}
	else
	{
		return FALSE;
	}
	
	//處理傳入參數
	bool bIsOffline = false;
	if (!(wcsstr(lpCmdLine, L"keepout"))) return 0;

	//處理Policy 使用參數
	ProfileInitial();
	if (gbEnable == false) {
		return FALSE;
	}
	//創建圖片儲存目錄
	CreateDirectoryFile();
	//找螢幕最大尺寸
	changeMonitorRect();
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_PATH);
	LoadString(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_PATH);

	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SCREENCAPTURE);

	if (gbByApp)
	{
		trd = new thread(&find_main_window);
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = (LPCTSTR)IDR_MENU1;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON1);;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,szWindowClass, szTitle, WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE, 100, 400, 80, 60, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	AllHwnd = hWnd;
	if (gbByApp)																							//判別是否啟動依照程序視窗截圖
		trd = new thread(&find_main_window);
	else
	{
		TimeEvent2 = SetTimer(hWnd, CAPTURE_CYCLE_START, CAPTURE_SPACING_RUNTIME * 1000, (TIMERPROC)WndProc);								//啟動定時截圖定時器
		{	if (gbVideoEnable)
			TimeEvent3 = SetTimer(hWnd, VIDEO_CYCLE_START, VIDEO_SPACING_RUNTIME * 60 * 1000, (TIMERPROC)WndProc);
		}
	}

	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:													//視窗MENU的執行動作設定
	{
		// Parse the menu selections:
		switch (wParam)
		{
		case IDM_EXIT:											//MENU>檔案>EXIT
		{
			DestroyWindow(hWnd);								//關閉視窗
			PostQuitMessage(0);									//結束程式
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_TIMER:															//循環設定
	{
		switch (wParam)
		{
		case CAPTURE_CYCLE_START:											//擷取桌面循環
		{
			nCount++;													//計算擷取次數
			capture_desktop();
			break;
		}
		case VIDEO_CYCLE_START:
		{
			Video_Create();
			break;
		}
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void SaveBitmap(wstring szFilename, HBITMAP hBitmap)							//儲存BITMAP function
{
	HDC					hdc = NULL;
	FILE* fp = NULL;
	LPVOID				pBuf = NULL;
	BITMAPINFO			bmpInfo;
	BITMAPFILEHEADER	bmpFileHeader;

	do {
		hdc = GetDC(NULL);
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		GetDIBits(hdc, hBitmap, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);

		if (bmpInfo.bmiHeader.biSizeImage <= 0)
			bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth * abs(bmpInfo.bmiHeader.biHeight) * (bmpInfo.bmiHeader.biBitCount + 7) / 8;

		if ((pBuf = malloc(bmpInfo.bmiHeader.biSizeImage)) == NULL)
		{
			MessageBox(NULL, _T("Unable to Allocate Bitmap Memory"), _T("Error"), MB_OK | MB_ICONERROR);
			break;
		}

		bmpInfo.bmiHeader.biCompression = BI_RGB;
		GetDIBits(hdc, hBitmap, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);
		if ((fp = _wfopen(szFilename.c_str(), _T("wb"))) == NULL)
		{
			CString screenBMPimagePath;
			screenBMPimagePath.Append(ImagePath);
			screenBMPimagePath.Append(L"\\screendata");
			CreateDirectory(screenBMPimagePath, 0);
		}

		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpInfo.bmiHeader.biSizeImage;
		bmpFileHeader.bfType = 'MB';
		bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, fp);
		fwrite(pBuf, bmpInfo.bmiHeader.biSizeImage, 1, fp);

	} while (false);
	if (hdc)
		ReleaseDC(NULL, hdc);
	if (pBuf)
		free(pBuf);
	if (fp)
		fclose(fp);
}

bool CompareBitmaps(HBITMAP HBitmapLeft)
{
	if (HBitmapLeft == HBitmapRight)
	{
		return true;
	}

	if (NULL == HBitmapLeft || NULL == HBitmapRight)
	{
		HBitmapRight = HBitmapLeft;
		return false;
	}
	bool bSame = false;

	HDC hdc = GetDC(NULL);
	BITMAPINFO BitmapInfoLeft = { 0 };

	BitmapInfoLeft.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoRight.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	if (0 != GetDIBits(hdc, HBitmapLeft, 0, 0, NULL, &BitmapInfoLeft, DIB_RGB_COLORS))
	{
		// Compare the BITMAPINFOHEADERs of the two bitmaps

		if (0 == memcmp(&BitmapInfoLeft.bmiHeader, &BitmapInfoRight.bmiHeader,
			sizeof(BITMAPINFOHEADER)))
		{
			// The BITMAPINFOHEADERs are the same so now compare the actual bitmap bits

			DWORD* pLeftBits = new DWORD(BitmapInfoLeft.bmiHeader.biSizeImage);
			DWORD* pRightBits = new DWORD(BitmapInfoRight.bmiHeader.biSizeImage);
			BYTE* pByteLeft = NULL;
			BYTE* pByteRight = NULL;

			PBITMAPINFO pBitmapInfoLeft = &BitmapInfoLeft;
			PBITMAPINFO pBitmapInfoRight = &BitmapInfoRight;

			// calculate the size in BYTEs of the additional

			// memory needed for the bmiColor table

			int AdditionalMemory = 0;
			switch (BitmapInfoLeft.bmiHeader.biBitCount)
			{
			case 1:
				AdditionalMemory = 1 * sizeof(RGBQUAD);
				break;
			case 4:
				AdditionalMemory = 15 * sizeof(RGBQUAD);
				break;
			case 8:
				AdditionalMemory = 255 * sizeof(RGBQUAD);
				break;
			case 16:
			case 32:
				AdditionalMemory = 2 * sizeof(RGBQUAD);
			}

			if (AdditionalMemory)
			{
				// we have to allocate room for the bmiColor table that will be

				// attached to our BITMAPINFO variables

				pByteLeft = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
				if (pByteLeft)
				{
					memset(pByteLeft, 0, sizeof(BITMAPINFO) + AdditionalMemory);
					memcpy(pByteLeft, pBitmapInfoLeft, sizeof(BITMAPINFO));
					pBitmapInfoLeft = (PBITMAPINFO)pByteLeft;
				}

				pByteRight = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
				if (pByteRight)
				{
					memset(pByteRight, 0, sizeof(BITMAPINFO) + AdditionalMemory);
					memcpy(pByteRight, pBitmapInfoRight, sizeof(BITMAPINFO));
					pBitmapInfoRight = (PBITMAPINFO)pByteRight;
				}
			}

			if (pLeftBits && pRightBits && pBitmapInfoLeft && pBitmapInfoRight)
			{
				// zero out the bitmap bit buffers

				//memset(pLeftBits, 0, BitmapInfoLeft.bmiHeader.biSizeImage);
				//memset(pRightBits, 0, BitmapInfoRight.bmiHeader.biSizeImage);

				// fill the bit buffers with the actual bitmap bits

				if (0 != GetDIBits(hdc, HBitmapLeft, 0,
					0, pLeftBits, pBitmapInfoLeft,
					DIB_RGB_COLORS) )
				{
					// compare the actual bitmap bits of the two bitmaps

					bSame = ( 0 == memcmp(pLeftBits, pRightBits,
						pBitmapInfoLeft->bmiHeader.biSizeImage));
				}
			}

			// clean up
			delete pLeftBits;
			delete pRightBits;
			delete[] pByteLeft;
			delete[] pByteRight;
		}
	}
	ReleaseDC(NULL, hdc);

	HBitmapRight = HBitmapLeft;
	BitmapInfoRight = BitmapInfoLeft;

	return bSame;
}

BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP* phBitmap, HPALETTE* phPalette)
{
	BITMAP  bm;
	*phBitmap = NULL;
	*phPalette = NULL;

	// Use LoadImage() to get the image loaded into a DIBSection
	*phBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (*phBitmap == NULL)
		return FALSE;

	// Get the color depth of the DIBSection
	GetObject(*phBitmap, sizeof(BITMAP), &bm);
	// If the DIBSection is 256 color or less, it has a color table
	if ((bm.bmBitsPixel * bm.bmPlanes) <= 8)
	{
		HDC           hMemDC;
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;
		WORD          i;

		// Create a memory DC and select the DIBSection into it
		hMemDC = CreateCompatibleDC(NULL);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, *phBitmap);
		// Get the DIBSection's color table
		GetDIBColorTable(hMemDC, 0, 256, rgb);
		// Create a palette from the color tabl
		pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + (256 * sizeof(PALETTEENTRY)));
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = 256;
		for (i = 0; i < 256; i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		*phPalette = CreatePalette(pLogPal);
		// Clean up
		free(pLogPal);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
	}
	else   // It has no color table, so use a halftone palette
	{
		HDC    hRefDC;
		hRefDC = GetDC(NULL);
		*phPalette = CreateHalftonePalette(hRefDC);
		ReleaseDC(NULL, hRefDC);
	}
	return TRUE;
}

bool DirectoryRecursive(CString token)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	DWORD Attributes;
	WCHAR str[MAX_PATH] = { 0 };
	lstrcpy(str, token);
	lstrcat(str, _T("\\*.*"));

	//List files
	hFind = FindFirstFile(str, &FindFileData);
	do {
		if (lstrcmp(FindFileData.cFileName, _T(".")) != 0 && lstrcmp(FindFileData.cFileName, _T("..")) != 0)
		{
			//Str append Example
			lstrcpy(str, token);
			lstrcat(str, _T("\\"));
			lstrcat(str, FindFileData.cFileName);
			Attributes = GetFileAttributes(str);
			if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//is directory
				DirectoryRecursive(str);
			}
			else
			{
				//not directory
				_wremove(str);
			}
		}
	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
	RemoveDirectory(token);
	return true;
}

BOOL Savedatatologfile(CString lpdata, BOOL deleteyn)
{
	WCHAR wnd_title[MAX_PATH] = { 0 };
	HWND hwnd = GetForegroundWindow(); // get handle of currently active window
	GetWindowText(hwnd, wnd_title, sizeof(wnd_title));

	DWORD PID;
	HWND targetWnd = FindWindow(NULL, wnd_title);
	GetWindowThreadProcessId(targetWnd, &PID);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
	WCHAR nameProc[1024] = { 0 };
	if (!GetProcessImageFileName(hProcess, nameProc, sizeof(nameProc)))
	{
		wcscpy(nameProc, L"system.exe ,cannot find");
	}
	CString output_yn;
	if (deleteyn == 1)
		output_yn = "delete";
	else
		output_yn = "exist";

	CloseHandle(hProcess);

	return true;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure
	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}
	free(pImageCodecInfo);
	return 0;
}

void savejpeg(wstring szFilename, HBITMAP hBitmap)
{
	Gdiplus::Bitmap bitmap(hBitmap, NULL);
	bitmap.Save(szFilename.c_str(), &clsid, &encoderParameters);
}

BOOL GetAllMonitorInfo(MonData* data)
{
	MonitorScaleList.clear();
	EnumDisplayMonitors(hDesktopDC, NULL, (MONITORENUMPROC)(&EnumProc), (LPARAM)(data));

	return true;
}

BOOL EnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MonData* data = (MonData*)dwData;
	data->info[data->current].cbSize = sizeof(MONITORINFOEXW);
	float Scale = (float)GetDeviceCaps(hdcMonitor, DESKTOPHORZRES) / (float)GetDeviceCaps(hdcMonitor, HORZRES);
	MonitorScaleList.push_back(Scale);
	return GetMonitorInfoW(hMonitor, &(data->info[data->current++]));
}

void changeMonitorRect()
{
	int cMonitors = GetSystemMetrics(SM_CMONITORS);
	MonData data;
	data.current = 0;
	data.info = (MONITORINFOEXW*)calloc(cMonitors, sizeof(MONITORINFOEXW));
	GetAllMonitorInfo(&data);

	for (int i = 0; i < cMonitors; i++)
	{
		if (data.info[i].rcWork.left < orgX)
			orgX = (int)(data.info[i].rcWork.left * MonitorScaleList[i]);
		if (data.info[i].rcWork.top < orgY)
			orgY = (int)(data.info[i].rcWork.top * MonitorScaleList[i]);
		if (data.info[i].rcWork.right > endx)
			endx = (int)(data.info[i].rcWork.right * MonitorScaleList[i]);
		if (data.info[i].rcWork.bottom > endy)
			endy = (int)(data.info[i].rcWork.bottom * MonitorScaleList[i]);
	}
	width = endx - orgX;
	height = endy - orgY;
}

wstring GetProfileSettingStr(wstring wsSection, wstring wsKey, wstring wsDefStr)
{
	TCHAR tmp_buf[1024] = { 0 };
	int iRtn = GetPrivateProfileString(wsSection.c_str(), wsKey.c_str(), wsDefStr.c_str(), tmp_buf, 1000, wsPolicyFile);
	wstring rt = tmp_buf;
	return rt;
}

bool GetProfileSettingBool(wstring wsSection, wstring wsKey, wstring wsDefStr)
{
	wstring rt = GetProfileSettingStr(wsSection, wsKey, wsDefStr);
	if (rt == L"Y") return true; else return false;
}

int GetProfileSettingInt(wstring wsSection, wstring wsKey, wstring wsDefStr)
{
	wstring rt = GetProfileSettingStr(wsSection, wsKey, wsDefStr);
	return _wtoi(rt.c_str());
}