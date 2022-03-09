#pragma once

#include <SDKDDKVer.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <Unknwn.h>

#include <Shlobj_core.h>
#include <commctrl.h>
#include <msctf.h>
#include <prsht.h>
#include <windows.h>
#include <windowsx.h>
#include <winrt/base.h>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")

#define ID_APPLY_NOW 0x3021
#define PCSB_INITIALIZED 1
#define PCSB_PRECREATE 2
#define PSCB_BUTTONPRESSED 3

#include "tip/log.h"
