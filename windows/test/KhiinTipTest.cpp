#include "pch.h"

#include "gtest/gtest.h"

#include <functional>

#include "KhiinClassFactory.h"
#include "Profile.h"
#include "Guids.h"

namespace khiin::win32::tip {
using namespace winrt;
TEST(KhiinTipTest, Activate) {
    check_hresult(CoInitialize(nullptr));
    HMODULE hmod = LoadLibraryEx(L"KhiinPJH.dll", NULL, 0);
    if (hmod) {
        typedef HRESULT (*getcls)(REFCLSID, REFIID, LPVOID);

        auto threadmgr = com_ptr<ITfThreadMgr>();
        auto clientid = com_ptr<ITfClientId>();
        auto docmgr = com_ptr<ITfDocumentMgr>();

        TfClientId tid = TF_CLIENTID_NULL;
        check_hresult(
            CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, threadmgr.put_void()));
        check_hresult(threadmgr->QueryInterface(IID_ITfClientId, clientid.put_void()));
        check_hresult(clientid->GetClientId(guids::kTextService, &tid));
        check_hresult(threadmgr->Activate(&tid));
        check_hresult(threadmgr->CreateDocumentMgr(docmgr.put()));
        HWND dummyHWND = ::CreateWindowA("STATIC", "dummy", WS_VISIBLE, 0, 0, 100, 100, NULL, NULL, NULL, NULL);
        threadmgr->AssociateFocus(dummyHWND, docmgr.get(), nullptr);

        FARPROC fptr = GetProcAddress(hmod, "DllGetClassObject");
        auto cf = static_cast<KhiinClassFactory *>(malloc(sizeof(KhiinClassFactory)));
        ((getcls)fptr)(__uuidof(KhiinClassFactory), IID_IClassFactory, &cf);
        auto tip = static_cast<ITfTextInputProcessorEx *>(malloc(sizeof(ITfTextInputProcessorEx)));
        check_hresult(cf->CreateInstance(NULL, IID_ITfTextInputProcessorEx, (void **)&tip));
        check_hresult(tip->Activate(threadmgr.get(), tid));
    }
    EXPECT_TRUE(true);
}

} // namespace khiin::win32::tip
