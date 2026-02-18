#pragma once
#include <windows.h>
#include <unknwn.h>

class PdfClassFactory : public IClassFactory {
public:
    PdfClassFactory();
    ~PdfClassFactory();

    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    IFACEMETHODIMP CreateInstance(IUnknown *outer, REFIID riid, void **ppv);
    IFACEMETHODIMP LockServer(BOOL fLock);

private:
    long m_ref = 1;
};
