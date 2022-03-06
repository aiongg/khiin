#include "pch.h"

#include "gtest/gtest.h"

#include <functional>

#include "KhiinClassFactory.h"
#include "Profile.h"

namespace khiin::win32 {
using namespace winrt;
TEST(KhiinPJHTest, Loads) {
    check_hresult(CoInitialize(nullptr));
    HMODULE hmod = LoadLibraryEx(L"KhiinPJH.dll", NULL, 0);
    if (hmod) {
        typedef HRESULT (*getcls)(REFCLSID, REFIID, LPVOID);
        ITfThreadMgr *pThreadMgr;
        check_hresult(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void **)&pThreadMgr));
        ITfClientId *pClientId;
        check_hresult(pThreadMgr->QueryInterface<ITfClientId>(&pClientId));
        TfClientId client_id;
        check_hresult(pClientId->GetClientId(Profile::textServiceGuid, &client_id));
        ITfThreadMgrEx *pThreadMgrEx;
        check_hresult(pThreadMgr->QueryInterface<ITfThreadMgrEx>(&pThreadMgrEx));
        pThreadMgrEx->ActivateEx(&client_id, TF_TMAE_NOACTIVATETIP | TF_TMAE_COMLESS);
        FARPROC fptr = GetProcAddress(hmod, "DllGetClassObject");
        auto cf = static_cast<KhiinClassFactory *>(malloc(sizeof(KhiinClassFactory)));
        ((getcls)fptr)(__uuidof(KhiinClassFactory), IID_IClassFactory, &cf);
        auto tip = static_cast<ITfTextInputProcessorEx *>(malloc(sizeof(ITfTextInputProcessorEx)));
        check_hresult(cf->CreateInstance(NULL, IID_ITfTextInputProcessorEx, (void **)&tip));
        check_hresult(tip->ActivateEx(pThreadMgrEx, client_id, TF_TMAE_NOACTIVATETIP | TF_TMAE_COMLESS));
    }
    EXPECT_TRUE(true);
}

} // namespace khiin::win32
