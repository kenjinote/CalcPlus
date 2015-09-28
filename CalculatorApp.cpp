#pragma comment(lib,"imm32")

#include "CalculatorApp.h"
#include "CalculatorCore.h"
#include "Literals.h"
#include "Resource.h"

//
// エントリーポイント
//
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			CalculatorApp app;
			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}
	return 0;
}

//
// メンバー変数の初期化
//
CalculatorApp::CalculatorApp() :
m_hwnd(NULL),
m_pD2DFactory(NULL),
m_pDWriteFactory(NULL),
m_pRenderTarget(NULL),
m_pTextFormat(NULL),
m_pTextBrush(NULL),
m_pDispBrush(NULL),
m_bCapture(FALSE),
m_lpszText(NULL),
m_bInputOperand(FALSE),
m_bInputPoint(FALSE),
m_bEqualed(TRUE),
m_bError(FALSE),
m_pBitmap(NULL)
{
}

//
// リソースの解放
//
CalculatorApp::~CalculatorApp()
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pTextBrush);
	SafeRelease(&m_pDispBrush);
	SafeRelease(&m_pBitmap);
}

//
// 初期化とウィンドウの作成
// デバイスに依存しないリソース
//
HRESULT CalculatorApp::Initialize()
{
	HRESULT hr = CreateDeviceIndependentResources();
	if (SUCCEEDED(hr))
	{
		// ウィンドウクラスの登録
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_DROPSHADOW /*CS_HREDRAW | CS_VREDRAW*/;
		wcex.lpfnWndProc = CalculatorApp::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hIcon = LoadIcon(HINST_THISCOMPONENT, (LPCTSTR)IDI_ICON1);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = L"CalculatorClass";

		RegisterClassEx(&wcex);

		FLOAT dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

		RECT rect;
		SetRect(&rect, 0, 0, static_cast<UINT>(ceil(640.f * dpiX / 96.f)), static_cast<UINT>(ceil(280.f * dpiY / 96.f)));
		AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, 0);

		m_hwnd = CreateWindowEx(
			WS_EX_LAYERED,
			L"CalculatorClass",
			L"電卓+",
			WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_BORDER,
			CW_USEDEFAULT,
			0,
			rect.right - rect.left,
			rect.bottom - rect.top,
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			SetLayeredWindowAttributes(m_hwnd, RGB(255, 0, 0), 248, LWA_COLORKEY | LWA_ALPHA);

			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

//
// バインドされていないリソースの作成（フォントなど）
//
HRESULT CalculatorApp::CreateDeviceIndependentResources()
{
	static const WCHAR msc_fontName[] = L"Inconsolata";
	static const FLOAT msc_fontSize = 32;
	HRESULT hr;

	// Direct2D factory を作成
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	if (SUCCEEDED(hr))
	{
		// DirectWrite factory を作成
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
	}

	if (SUCCEEDED(hr))
	{
		// Create a DirectWrite text format object.
		hr = m_pDWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&m_pTextFormat
			);
	}

	if (SUCCEEDED(hr))
	{
		// Center the text horizontally and vertically.
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING); // 右寄せ
		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR); // 上寄せ
	}

	return hr;
}

//
//  デバイス固有のリソースを作成
//
HRESULT CalculatorApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{

		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
			);

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
			);

		if (SUCCEEDED(hr))
		{
			// Create a black brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				TEXTCOLOR,
				&m_pTextBrush
				);
			hr = m_pRenderTarget->CreateSolidColorBrush(
				DISPCOLOR,
				&m_pDispBrush
				);
		}

	}

	return hr;
}


//
//  デバイス固有のリソースを開放する
//
void CalculatorApp::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pTextBrush);
	SafeRelease(&m_pDispBrush);
}

//
// メインウィンドウのメッセージループ
//
void CalculatorApp::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


//
// 画面描画
//
HRESULT CalculatorApp::OnRender()
{
	HRESULT hr;

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		// Retrieve the size of the render target.
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(BACKCOLOR);

		//m_pRenderTarget->DrawLine(
		//	D2D1::Point2F(static_cast<FLOAT>(0), 28.0f),
		//	D2D1::Point2F(static_cast<FLOAT>(640), 28.0f),
		//	m_pTextBrush,
		//	20.0f
		//	);

		D2D1_RECT_F rectangle2 = D2D1::RectF(
			0,
			28,
			640,
			28 + 48
			);
		m_pRenderTarget->FillRectangle(&rectangle2, m_pDispBrush);

		m_pRenderTarget->DrawText(
			m_lpszText,
			lstrlen(m_lpszText),
			m_pTextFormat,
			D2D1::RectF(OUTOFFSET, OUTOFFSET * 4 + 4, renderTargetSize.width - 2 * OUTOFFSET, renderTargetSize.height - 8 * OUTOFFSET - 8),
			m_pTextBrush
			);

		hr = m_pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}

	return hr;
}

//
// ウィンドウがリサイズされたとき
//
void CalculatorApp::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;
		m_pRenderTarget->Resize(size);
	}
}

//
// ２点の距離を計算
//
double GetDis(const LPPOINT a, const LPPOINT b)
{
	const double Xab = a->x - b->x;
	const double Yab = a->y - b->y;
	return sqrt(Xab * Xab + Yab * Yab);
}

//
// ウィンドウメッセージハンドラー
//
LRESULT CALLBACK CalculatorApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		ImmAssociateContext(hwnd, 0);

		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		CalculatorApp *pCalculatorApp = (CalculatorApp *)pcs->lpCreateParams;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pCalculatorApp));

		pCalculatorApp->m_lpszText = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR)*(2));
		pCalculatorApp->m_lpszText[0] = TEXT('0');
		pCalculatorApp->m_lpszText[1] = 0;

		//// ボタンを作成
		for (int i = 0; i < BUTTON_MAX; i++)
		{
			if (i == BUTTON_CLOSE)
			{
				pCalculatorApp->m_ButtonList[i].CreateButton(button[i].szText, &button[i].rect, hwnd, (HMENU)button[i].id, ((LPCREATESTRUCT)(lParam))->hInstance, IDB_CLOSE_NORMAL, IDB_CLOSE_PRESS, IDB_CLOSE_CURSOR, IDB_CLOSE_NORMAL);
			}
			else
			{
				pCalculatorApp->m_ButtonList[i].CreateButton(button[i].szText, &button[i].rect, hwnd, (HMENU)button[i].id, ((LPCREATESTRUCT)(lParam))->hInstance, IDB_BITMAP1, IDB_BITMAP2, IDB_BITMAP3, IDB_BITMAP4);
			}
		}

		pCalculatorApp->m_Border1.CreateBorder(28, hwnd, ((LPCREATESTRUCT)(lParam))->hInstance);// button[i].szText, &button[i].rect, hwnd, (HMENU)button[i].id, ((LPCREATESTRUCT)(lParam))->hInstance, IDB_CLOSE_NORMAL, IDB_CLOSE_PRESS, IDB_CLOSE_CURSOR, IDB_CLOSE_NORMAL);
		pCalculatorApp->m_Border2.CreateBorder(28 + 48, hwnd, ((LPCREATESTRUCT)(lParam))->hInstance);// button[i].szText, &button[i].rect, hwnd, (HMENU)button[i].id, ((LPCREATESTRUCT)(lParam))->hInstance, IDB_CLOSE_NORMAL, IDB_CLOSE_PRESS, IDB_CLOSE_CURSOR, IDB_CLOSE_NORMAL);

		//SetTimer(hwnd, 0x1234, 1, NULL);

		result = 1;
	}
	else
	{
		CalculatorApp *pCalculatorApp = reinterpret_cast<CalculatorApp *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
		bool wasHandled = false;
		if (pCalculatorApp)
		{
			switch (message)
			{
				//case WM_TIMER:
				//	{
				//	//KillTimer(hwnd, 0x1234);
				//		POINT point1;
				//		GetCursorPos(&point1);
				//		RECT rect;
				//		GetWindowRect(hwnd, &rect);
				//		POINT point2 = { (rect.right + rect.left)/2.0, (rect.bottom + rect.top)/2.0 };
				//		const double dDistance= GetDis(&point1, &point2);
				//		if (dDistance > rect.right - rect.left)
				//		{
				//			SetLayeredWindowAttributes(hwnd, RGB(255, 0, 0), (BYTE)128, LWA_COLORKEY | LWA_ALPHA);
				//		}
				//		else
				//		{
				//			SetLayeredWindowAttributes(hwnd, RGB(255, 0, 0), (BYTE)(256.0 - dDistance / (rect.right - rect.left) * 128.0), LWA_COLORKEY | LWA_ALPHA);
				//		}
				//		//SetTimer(hwnd, 0x1234, 1, NULL);
				//	}
				//	break;

				//case WM_PRINTCLIENT:
				//	//hdc = (HDC)wp;
				//	//hBmp = (HBITMAP)LoadImage(hInst,
				//	//	"MYBMP",
				//	//	IMAGE_BITMAP,
				//	//	0, 0,
				//	//	LR_DEFAULTCOLOR);
				//	//GetObject(hBmp, (int)sizeof(BITMAP), &bmp_info);
				//	//wx = bmp_info.bmWidth;
				//	//wy = bmp_info.bmHeight;
				//	//hdc_mem = CreateCompatibleDC(hdc);
				//	//SelectObject(hdc_mem, hBmp);
				//	//BitBlt(hdc, 0, 0, wx, wy, hdc_mem, 0, 0, SRCCOPY);
				//	//DeleteObject(hBmp);
				//	//DeleteDC(hdc_mem);
				//	break;
			case WM_SIZE:
			{
				const UINT width = LOWORD(lParam);
				const UINT height = HIWORD(lParam);
				pCalculatorApp->OnResize(width, height);
			}
			wasHandled = true;
			result = 0;
			break;

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			wasHandled = true;
			result = 0;
			break;

			case WM_PAINT:
			{
				pCalculatorApp->OnRender();

				ValidateRect(hwnd, NULL);
			}
			wasHandled = true;
			result = 0;
			break;


			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case BUTTON_0:
				case BUTTON_1:
				case BUTTON_2:
				case BUTTON_3:
				case BUTTON_4:
				case BUTTON_5:
				case BUTTON_6:
				case BUTTON_7:
				case BUTTON_8:
				case BUTTON_9:
				{
					if (pCalculatorApp->m_bError || pCalculatorApp->m_bEqualed)
					{
						Reset(&pCalculatorApp->m_lpszText);
						pCalculatorApp->m_bEqualed = 0;
						pCalculatorApp->m_bError = 0;
					}
					AddText(&pCalculatorApp->m_lpszText, button[LOWORD(wParam)].szText);
					pCalculatorApp->m_bInputOperand = 0;
				}
				break;
				case BUTTON_LEFT:
				case BUTTON_RIGHT:
					if (!pCalculatorApp->m_bError)
					{
						if (pCalculatorApp->m_bEqualed)
						{
							Reset(&pCalculatorApp->m_lpszText);
							pCalculatorApp->m_bEqualed = 0;
						}
						AddText(&pCalculatorApp->m_lpszText, button[LOWORD(wParam)].szText);
						pCalculatorApp->m_bInputOperand = 0;
						pCalculatorApp->m_bInputPoint = 0;
					}
					break;

				case BUTTON_PLUS:
				case BUTTON_MINUS:
				case BUTTON_MULTIPLY:
				case BUTTON_DIVIDE:
					if (!pCalculatorApp->m_bError)
					{
						if (pCalculatorApp->m_bEqualed)pCalculatorApp->m_bEqualed = 0;
						if (pCalculatorApp->m_bInputOperand)
						{
							const DWORD dwSize = lstrlen(pCalculatorApp->m_lpszText);
							pCalculatorApp->m_lpszText[dwSize - 1] = button[LOWORD(wParam)].szText[0];
						}
						else
						{
							AddText(&pCalculatorApp->m_lpszText, button[LOWORD(wParam)].szText);
							pCalculatorApp->m_bInputOperand = 1;
						}
						pCalculatorApp->m_bInputPoint = 0;
					}
					break;

				case BUTTON_POINT:
					if (!pCalculatorApp->m_bError&&!pCalculatorApp->m_bInputPoint)
					{
						if (pCalculatorApp->m_bEqualed)
						{
							pCalculatorApp->m_lpszText = (LPTSTR)GlobalReAlloc(pCalculatorApp->m_lpszText, sizeof(TCHAR) * 2, GMEM_MOVEABLE);
							pCalculatorApp->m_lpszText[0] = TEXT('0');
							pCalculatorApp->m_lpszText[1] = 0;
							pCalculatorApp->m_bEqualed = 0;
						}
						AddText(&pCalculatorApp->m_lpszText, button[LOWORD(wParam)].szText);
						pCalculatorApp->m_bInputOperand = 0;
						pCalculatorApp->m_bInputPoint = 1;
					}
					break;

				case BUTTON_EQUAL:
					if (!pCalculatorApp->m_bError)
					{
						InitValue(pCalculatorApp->m_lpszText);
						const RR ans = expr();
						if (IsError())
						{
							pCalculatorApp->m_bError = 1;
							GlobalFree(pCalculatorApp->m_lpszText);
							pCalculatorApp->m_lpszText = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR) * 6);
							lstrcpy(pCalculatorApp->m_lpszText, TEXT("ERORR"));

						}
						else
						{
							GlobalFree(pCalculatorApp->m_lpszText);
							pCalculatorApp->m_lpszText = RRtoString(ans);
							pCalculatorApp->m_bEqualed = 1;
							pCalculatorApp->m_bInputOperand = 0;
							pCalculatorApp->m_bInputPoint = 0;
						}
					}
					break;

					//{ TEXT("√"), BUTTON_SQRT, { X1, Y5, X1 + BUTTON_WIDTH, Y5 + BUTTON_HEIGHT } },
					//{ TEXT("^"), BUTTON_POW, { X2, Y5, X2 + BUTTON_WIDTH, Y5 + BUTTON_HEIGHT } },
					//{ TEXT("Mod"), BUTTON_MOD, { X3, Y5, X3 + BUTTON_WIDTH, Y5 + BUTTON_HEIGHT } },
					//{ TEXT("Sin"), BUTTON_SIN, { X4, Y5, X4 + BUTTON_WIDTH, Y5 + BUTTON_HEIGHT } },
					//{ TEXT("Cos"), BUTTON_COS, { X5, Y5, X5 + BUTTON_WIDTH, Y5 + BUTTON_HEIGHT } },
					//{ TEXT("Tan"), BUTTON_TAN, { X1, Y6, X1 + BUTTON_WIDTH, Y6 + BUTTON_HEIGHT } },
					//{ TEXT("税金"), BUTTON_TAX, { X2, Y6, X2 + BUTTON_WIDTH, Y6 + BUTTON_HEIGHT } },

				case BUTTON_CLEAR:
				{
					pCalculatorApp->m_lpszText = (LPTSTR)GlobalReAlloc(pCalculatorApp->m_lpszText, sizeof(TCHAR) * 2, GMEM_MOVEABLE);
					pCalculatorApp->m_lpszText[0] = TEXT('0');
					pCalculatorApp->m_lpszText[1] = 0;
					pCalculatorApp->m_bEqualed = 1;
					pCalculatorApp->m_bInputOperand = 0;
					pCalculatorApp->m_bInputPoint = 0;
					pCalculatorApp->m_bError = 0;
				}
				break;
				case BUTTON_BACK:
				{
					const DWORD dwSize = GlobalSize(pCalculatorApp->m_lpszText);
					if (pCalculatorApp->m_bError || pCalculatorApp->m_bEqualed || dwSize / sizeof(TCHAR) == 2)
					{
						pCalculatorApp->m_lpszText = (LPTSTR)GlobalReAlloc(pCalculatorApp->m_lpszText, sizeof(TCHAR) * 2, GMEM_MOVEABLE);
						pCalculatorApp->m_lpszText[0] = TEXT('0');
						pCalculatorApp->m_lpszText[1] = 0;
						pCalculatorApp->m_bEqualed = 1;
						pCalculatorApp->m_bInputOperand = 0;
						pCalculatorApp->m_bInputPoint = 0;
						pCalculatorApp->m_bError = 0;
					}
					else
					{
						pCalculatorApp->m_lpszText = (LPTSTR)GlobalReAlloc(pCalculatorApp->m_lpszText, dwSize - sizeof(TCHAR), GMEM_MOVEABLE);
						pCalculatorApp->m_lpszText[dwSize / sizeof(TCHAR) - 2] = 0;
					}
				}
				break;
				case BUTTON_CLOSE:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				}
				SetFocus(hwnd); // キーボード入力を有効にするためメインウィンドウにフォーカスを戻す
				InvalidateRect(hwnd, 0 /*&rectDisplayText*/, 0);
				wasHandled = true;
				result = 0;
				break;

			case WM_NCHITTEST:
				wParam = DefWindowProc(hwnd, message, wParam, lParam);
				if (wParam == HTCLIENT)
					return HTCAPTION;
				else
					return wParam;

			case WM_KEYDOWN:
				if (pCalculatorApp->m_bCapture == 0)
				{
					int nButton = -1;
					switch (wParam)
					{
					case '0':
					case VK_NUMPAD0:
						nButton = BUTTON_0;
						break;
					case '1':
					case VK_NUMPAD1:
						nButton = BUTTON_1;
						break;
					case '2':
					case VK_NUMPAD2:
						nButton = BUTTON_2;
						break;
					case '3':
					case VK_NUMPAD3:
						nButton = BUTTON_3;
						break;
					case '4':
					case VK_NUMPAD4:
						nButton = BUTTON_4;
						break;
					case '5':
					case VK_NUMPAD5:
						nButton = BUTTON_5;
						break;
					case '6':
					case VK_NUMPAD6:
						nButton = BUTTON_6;
						break;
					case '7':
					case VK_NUMPAD7:
						nButton = BUTTON_7;
						break;
					case '8':
					case VK_NUMPAD8:
						nButton = BUTTON_8;
						break;
					case '9':
					case VK_NUMPAD9:
						nButton = BUTTON_9;
						break;
					case '+':
					case VK_ADD:
						nButton = BUTTON_PLUS;
						break;
					case VK_SUBTRACT:
						nButton = BUTTON_MINUS;
						break;
					case '*':
					case VK_MULTIPLY:
						nButton = BUTTON_MULTIPLY;
						break;
					case 191:
					case VK_DIVIDE:
						nButton = BUTTON_DIVIDE;
						break;
					case VK_DECIMAL:
						nButton = BUTTON_POINT;
						break;
					case '=':
					case VK_SEPARATOR:
					case VK_RETURN:
						nButton = BUTTON_EQUAL;
						break;
					case VK_DELETE:
					case VK_ESCAPE:
						nButton = BUTTON_CLEAR;
						break;
					case VK_BACK:
						nButton = BUTTON_BACK;
						break;
					case 219:
						nButton = BUTTON_LEFT;
						break;
					case 221:
						nButton = BUTTON_RIGHT;
						break;
					}

					if (nButton != -1)
					{
						//SendMessage(pCalculatorApp->m_ButtonList[nButton]., WM_COMMAND, MAKELONG(::GetDlgCtrlID(hwndCancel), BN_CLICKED), (LPARAM)hwndCancel);
						//SendMessage(pCalculatorApp->m_ButtonList[nButton],)
						SendMessage(hwnd, WM_COMMAND, nButton, 0);
					}
				}
				break;

			case WM_DESTROY:
				//KillTimer(hwnd, 0x1234);

				if (pCalculatorApp->m_lpszText)
				{
					GlobalFree(pCalculatorApp->m_lpszText);
				}

				PostQuitMessage(0);
				wasHandled = true;
				result = 1;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}
