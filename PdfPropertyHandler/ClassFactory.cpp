#include "ClassFactory.h"
#include "PdfPropertyHandler.h"

PdfClassFactory::PdfClassFactory() {}
PdfClassFactory::~PdfClassFactory() {}

ULONG PdfClassFactory::AddRef() {
    return InterlockedIncrement(&m_ref);
}

ULONG PdfClassFactory::Release() {
    ULONG res = InterlockedDecrement(&m_ref);
    if (!res) delete this;
    return res;
}

HRESULT PdfClassFactory::QueryInterface(REFIID riid, void **ppv) {
    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory *>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

HRESULT PdfClassFactory::CreateInstance(IUnknown *outer, REFIID riid, void **ppv) {
    if (outer) return CLASS_E_NOAGGREGATION;

    PdfPropertyHandler *handler = new PdfPropertyHandler();
    return handler->QueryInterface(riid, ppv);
}

HRESULT PdfClassFactory::LockServer(BOOL) { return S_OK; }
