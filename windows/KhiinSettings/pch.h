#pragma once

#include <SDKDDKVer.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <Unknwn.h>

#include <Shlobj_core.h>
#include <commctrl.h>
#include <prsht.h>
#include <windows.h>
#include <windowsx.h>
//#include <WinUser.h>
#include <winrt/base.h>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251 6385 26495 26812)
#endif

#include "proto/config.pb.h"
#include "proto/messages.pb.h"

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "KhiinPJH/log.h"
#include "KhiinPJH/Utils.h"


#define ID_APPLY_NOW 0x3021
#define PCSB_INITIALIZED 1
#define PCSB_PRECREATE 2
#define PSCB_BUTTONPRESSED 3
