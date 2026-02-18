#include "PdfPropertyHandler.h"

static const PROPERTYKEY g_keys[] = {
    PKEY_Title,
    PKEY_Author,
    PKEY_Subject,
    PKEY_Keywords
};

PdfPropertyHandler::PdfPropertyHandler() {}
PdfPropertyHandler::~PdfPropertyHandler() {}

ULONG PdfPropertyHandler::AddRef() { return InterlockedIncrement(&m_ref); }
ULONG PdfPropertyHandler::Release() {
    ULONG r = InterlockedDecrement(&m_ref);
    if (!r) delete this;
    return r;
}

HRESULT PdfPropertyHandler::QueryInterface(REFIID riid, void **ppv) {
    if (riid == IID_IUnknown || riid == IID_IPropertyStore) {
        *ppv = this;
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

HRESULT PdfPropertyHandler::Load(LPCWSTR path) {
    PdfMetadata meta;
    if (!ExtractPdfMetadata(path, meta))
        return E_FAIL;

    m_title = meta.title;
    m_author = meta.author;
    m_subject = meta.subject;
    m_keywords = meta.keywords;

    return S_OK;
}

HRESULT PdfPropertyHandler::GetCount(DWORD *cProps) {
    *cProps = ARRAYSIZE(g_keys);
    return S_OK;
}

HRESULT PdfPropertyHandler::GetAt(DWORD i, PROPERTYKEY *key) {
    if (i >= ARRAYSIZE(g_keys)) return E_INVALIDARG;
    *key = g_keys[i];
    return S_OK;
}

HRESULT PdfPropertyHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT *pv) {
    PropVariantInit(pv);

    if (key == PKEY_Title) return InitPropVariantFromString(m_title.c_str(), pv);
    if (key == PKEY_Author) return InitPropVariantFromString(m_author.c_str(), pv);
    if (key == PKEY_Subject) return InitPropVariantFromString(m_subject.c_str(), pv);
    if (key == PKEY_Keywords) return InitPropVariantFromString(m_keywords.c_str(), pv);

    return S_FALSE;
}
