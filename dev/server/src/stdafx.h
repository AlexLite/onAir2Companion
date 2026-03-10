// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

// SLMessageServer type library from local SDK files.
// This avoids requiring COM registration at compile time.
#import "../softlab/x86/SLMessageQueue2.dll" named_guids raw_interfaces_only\
	rename("GetMessage", "SLGetMessage")\
	rename("SendMessage", "SLSendMessage")
using namespace SLMessageQueue2Lib;
