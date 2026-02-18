#pragma once
#include <windows.h>
#include <propsys.h>
#include <propvarutil.h>
#include <string>
#include "PdfMetadata.h"

class PdfPropertyHandler : public IPropertyStore {
public:
    PdfPropertyHandler();
    virtual ~PdfPropertyHandler();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IPropertyStore
    IFACEMETHODIMP GetCount(DWORD *cProps);
    IFACEMETHODIMP GetAt(DWORD iProp, PROPERTYKEY *pkey);
    IFACEMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT *pv);
    IFACEMETHODIMP SetValue(REFPROPERTYKEY, REFPROPVARIANT) { return STG_E_ACCESSDENIED; }
    IFACEMETHODIMP Commit() { return S_OK; }

    HRESULT Load(LPCWSTR filePath);

private:
    long m_ref = 1;

    std::wstring m_title;
    std::wstring m_author;
    std::wstring m_subject;
    std::wstring m_keywords;
};
