#include "CalculatorCore.h"
#include <sstream>
#include <string>

enum{ TK_EOF, TK_CONST, TK_PLUS, TK_MINUS, TK_MULT, TK_DIV, TK_POW, TK_LPAREN, TK_RPAREN };

LPTSTR g_pbuf;
RR g_value;
int g_back;
int g_error;

int get_token()
{
	if (g_back >= 0){ int ret = g_back; g_back = -1; return ret; }
	while (isspace(*g_pbuf))g_pbuf++;
	switch (*g_pbuf)
	{
	case TEXT('\0'):return TK_EOF;
	case TEXT('+'):g_pbuf++; return TK_PLUS;
	case TEXT('-'):g_pbuf++; return TK_MINUS;
	case TEXT('*'):g_pbuf++; return TK_MULT;
	case TEXT('/'):g_pbuf++; return TK_DIV;
	case TEXT('^'):g_pbuf++; return TK_POW;
	case TEXT('('):g_pbuf++; return TK_LPAREN;
	case TEXT(')'):g_pbuf++; return TK_RPAREN;
	default:
		{
			const int nSize = WideCharToMultiByte(CP_ACP, 0, g_pbuf, -1, NULL, 0, NULL, NULL);
			LPSTR lpStr = (LPSTR)GlobalAlloc(GMEM_FIXED, nSize);
			WideCharToMultiByte(CP_ACP, 0, g_pbuf, -1, lpStr, nSize, NULL, NULL);
			g_value = to_RR(lpStr);
			g_pbuf += strcspn(lpStr, "+-*/()^");
			GlobalFree(lpStr);
		}
		return TK_CONST;
	}
}

//
// 要素
//
RR prim()
{
	RR val;
	const int token = get_token();
	switch (token)
	{
	case TK_CONST:return g_value;
	case TK_MINUS:return -prim();
	case TK_LPAREN:val = expr(); get_token(); return val;
	default:return to_RR(0);
	}
}

//
// 因子
//
RR factor()
{
	RR val = prim();          // 最初の基本要素
	for (;;)
	{
		const int token = get_token();
		switch (token)
		{
		case TK_POW:val = power(val, to_long(prim())); break;
		default:g_back = token; return val;
		}
	}
}

//
// 乗除
//
RR term()
{
	RR val1 = factor(), val2;
	g_back = get_token();
	for (;;)
	{
		const int token = get_token();
		switch (token)
		{
		case TK_MULT:
			val1 *= factor();
			break;
		case TK_DIV:
			val2 = factor();
			g_back = get_token();
			if (val2 != 0)
			{
				val1 /= val2;
			}
			else
			{
				val1 = 1;
				g_error = -1;//0で除算
			}
			break;
		default:
			g_back = token;
			return val1;
		}
	}
}

//
// 加減
//
RR expr()
{
	RR val = term();
	for (;;)
	{
		const int token = get_token();
		switch (token)
		{
		case TK_PLUS:val += term(); break;
		case TK_MINUS:val -= term(); break;
		default:g_back = token; return val;
		}
	}
}

//
// 文字列の長さ（10進数）
//
long RR_len(const RR& n)
{
	long i;
	RR num;
	num = abs(n);
	for (i = 0; num != 0; i++)
	{
		num /= 10;
	}
	return (n>0) ? i : i + 1;
}

//
// 余分な文字を取り除く
//
std::string trim(const std::string& string, const char* trimCharacterList = "0./")
{
	std::string result;

	// 左側からトリムする文字以外が見つかる位置を検索します。
	std::string::size_type left = string.find_first_not_of(trimCharacterList);

	if (left != std::string::npos)
	{
		// 左側からトリムする文字以外が見つかった場合は、同じように右側からも検索します。
		std::string::size_type right = string.find_last_not_of(trimCharacterList);

		// 戻り値を決定します。ここでは右側から検索しても、トリムする文字以外が必ず存在するので判定不要です。
		result = string.substr(left, right - left + 1);
	}

	return result;
}

//
// 文字列へ変換
//
LPTSTR RRtoString(const RR& r)
{
	if (IsZero(r))
	{
		LPTSTR p = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR)* 2);
		p[0] = TEXT('0');
		p[1] = 0;
		return p;
	}
	else
	{
		r.SetOutputPrecision(1000);
		std::ostringstream stream;
		stream << r;
		std::string result = stream.str();
		trim(result);
		const int nSize = (int)result.size();
		LPSTR lpszText = (LPSTR)GlobalAlloc(0, nSize + 1);
		lstrcpyA(lpszText, result.c_str());

		const DWORD len = MultiByteToWideChar(CP_ACP, 0, lpszText, -1, 0, 0);
		LPWSTR pwsz = (LPWSTR)GlobalAlloc(0, len*sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, lpszText, -1, pwsz, len);

		GlobalFree(lpszText);

		return pwsz;
	}
}

//
// リセット
//
void Reset(LPTSTR *lpszText)
{
	*lpszText = (LPTSTR)GlobalReAlloc(*lpszText, sizeof(TCHAR), GMEM_MOVEABLE);
	*lpszText[0] = 0;
}

//
// 文字列を追加
//
void AddText(LPTSTR *lpszText, LPTSTR lpszAddText)
{
	const DWORD dwSize = (DWORD)GlobalSize(*lpszText);
	*lpszText = (LPTSTR)GlobalReAlloc(*lpszText, dwSize + sizeof(TCHAR)*lstrlen(lpszAddText), GMEM_MOVEABLE);
	lstrcat(*lpszText, lpszAddText);
}

//
// 計算時の初期化
//
void InitValue(LPTSTR lpszText)
{
	g_pbuf = lpszText;
	g_back = -1;
	g_error = 0;
}

BOOL IsError()
{
	return (g_error != 0);
}