#pragma once
/* ビットマップボタンのクラス */
class BorderControl
{
public:
	BorderControl();
	~BorderControl();

	/* BorderControl デストラクタ */
	HWND CreateBorder(
		int nHeight,
		HWND hWndParent,     // 親ウィンドウまたはオーナーウィンドウのハンドル
		HINSTANCE hInstance
		);

private:
	int m_nHeight;			// ボタンの高さ
	WNDPROC m_DefBorderProc;	// ボタンのデフォルトウィンドウプロシージャを保持

	virtual LRESULT BorderControl::LocalButtonProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK BorderControl::GlobalButtonProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};