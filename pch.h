#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0400
#define _WIN32_WINNT 0x0501
#define _RICHEDIT_VER 0x0100
#define NOMINMAX

#include <winsock2.h>

#include <cstdlib>
#include <tchar.h>
#include <malloc.h>
#include <memory.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlframe.h>
#include <atlgdi.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlcrack.h>
#include <atlmisc.h>