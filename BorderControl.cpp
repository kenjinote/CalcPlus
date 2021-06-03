#include <windows.h>
#include "BorderControl.h"
#define SZCECOWIZBUTTONPROC TEXT("BorderProc")

/* BorderControl コンストラクタ */
BorderControl::BorderControl()
{
}

/* BorderControl デストラクタ */
BorderControl::~BorderControl()
{
}

/* ボーダーコントロール作成 */
HWND BorderControl::CreateBorder(
	int nHeight,
	HWND hWndParent,     // 親ウィンドウまたはオーナーウィンドウのハンドル
	HINSTANCE hInstance // アプリケーションインスタンスのハンドル
	)
{
	m_nHeight = nHeight;
	const HWND hWnd = CreateWindowEx(0, L"STATIC", 0, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, nHeight, 640, 1, hWndParent, 0, hInstance, NULL);
	if (!hWnd) return NULL;
	SetClassLongPtr(hWnd, GCL_STYLE, GetClassLongPtr(hWnd, GCL_STYLE) & ~CS_DBLCLKS);
	SetProp(hWnd, SZCECOWIZBUTTONPROC, this); // ウィンドウハンドルのプロパテにオブジェクトのポインタを関連付ける
	m_DefBorderProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)GlobalButtonProc);
	return hWnd;
}

/* グローバル（静的）のウィンドウプロシージャ ※ローカル変数へのアクセス不可 */
LRESULT CALLBACK BorderControl::GlobalButtonProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	BorderControl *pBorderControl = (BorderControl*)GetProp(hWnd, SZCECOWIZBUTTONPROC);
	if (pBorderControl)
	{
		return pBorderControl->LocalButtonProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/* ローカル（動的）のウィンドウプロシージャ */
LRESULT BorderControl::LocalButtonProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (msg)
	{
	case WM_PRINTCLIENT: // これを処理しないとAnimateWindow()が動作しない
	case WM_ERASEBKGND: // 背景を描画
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		COLORREF clrPrev = SetBkColor((HDC)wParam, RGB(171, 171, 171));
		ExtTextOut((HDC)wParam, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
		SetBkColor((HDC)wParam, clrPrev);
	}
		return 1;
	case WM_DESTROY:
		RemoveProp(hWnd, SZCECOWIZBUTTONPROC);
		break;
	}
	return CallWindowProc(m_DefBorderProc, hWnd, msg, wParam, lParam);
}