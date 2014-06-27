#pragma once

#ifdef _DEBUG
#pragma comment(lib,"WinNTL-6_0_0_DEBUG")
#else
#pragma comment(lib,"WinNTL-6_0_0")
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