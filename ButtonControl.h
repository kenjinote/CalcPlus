#pragma once
/* ビットマップボタンのクラス */
class ButtonControl
{
public:
	ButtonControl();
	~ButtonControl();

	/* ButtonControl デストラクタ */
	HWND CreateButton(
		LPCWSTR lpszText,
		LPRECT lpRect,
		HWND hWndParent,     // 親ウィンドウまたはオーナーウィンドウのハンドル
		HMENU hMenu,         // メニューハンドルまたは子ウィンドウ ID
		HINSTANCE hInstance, // アプリケーションインスタンスのハンドル
		int nNorm,           // ノーマルのときのリソースビットマップID
		int nSel,            // 押したときのリソースビットマップID
		int nHover,          // ホバーのときのリソースビットマップID
		int nDis             // ディセーブルのときのリソースビットマップID
		);

private:
	int m_nWidth;			// ボタンの幅
	int m_nHeight;			// ボタンの高さ
	HBITMAP m_hBmpNorm;		// 通常のボタン画像
	HBITMAP m_hBmpSel;		// マウスダウン時のボタン画像
	HBITMAP m_hBmpHover;	// マウスオーバー時のボタン画像
	HBITMAP m_hBmpDis;		// 無効状態(ディセーブル）時のボタン画像
	WNDPROC m_DefBtnProc;	// ボタンのデフォルトウィンドウプロシージャを保持
	BOOL m_bHover;			// ホーバー状態かどうかのフラグ
	BOOL m_bPush;			// プッシュ状態かどうかのフラグ
	BOOL m_bDisable;		// 無効状態(ディセーブル）かどうかのフラグ

	virtual LRESULT ButtonControl::LocalButtonProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	static LRESULT CALLBACK ButtonControl::GlobalButtonProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};