#pragma once

#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib,"x64\\Debug\\WinNTL-11_4_4")
#else
#pragma comment(lib,"x64\\Release\\WinNTL-11_4_4")
#endif
#elif _WIN32
#ifdef _DEBUG
#pragma comment(lib,"Debug\\WinNTL-11_4_4")
#else
#pragma comment(lib,"Release\\WinNTL-11_4_4")
#endif
#endif


#define NTL_NO_MIN_MAX
#include<NTL/RR.h>
#include<Windows.h>

using namespace NTL;

NTL::RR expr();
int get_token();
NTL::RR prim();
NTL::RR factor();
NTL::RR term();
NTL::RR expr();
long RR_len(const NTL::RR& r);
LPTSTR RRtoString(const RR& r);
void Reset(LPTSTR *lpszText);
void AddText(LPTSTR *lpszText, LPTSTR lpszAddText);
void InitValue(LPTSTR lpszText);
BOOL IsError();