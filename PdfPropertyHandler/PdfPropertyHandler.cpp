
#include "PdfPropertyHandler.h"
#include "ClassFactory.h"
#include <new>
#include <strsafe.h>

extern long g_cDllRef;
extern "C" IMAGE_DOS_HEADER __ImageBase; // for module path

const PROPERTYKEY PdfPropertyHandler::s_props[4] = {
    PKEY_Title,
    PKEY_Author,
    PKEY_Subject,
    PKEY_Keywords
};

PdfPropertyHandler::PdfPropertyHandler() : m_ref(1) {}
PdfPropertyHandler::~PdfPropertyHandler() {}

// IUnknown
HRESULT PdfPropertyHandler::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_IPropertyStore) {
        *ppv = static_cast<IPropertyStore*>(this);
    } else if (riid == IID_IInitializeWithStream) {
        *ppv = static_cast<IInitializeWithStream*>(this);
    } else {
        *ppv = nullptr; return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG PdfPropertyHandler::AddRef() { return InterlockedIncrement(&m_ref); }
ULONG PdfPropertyHandler::Release() {
    ULONG c = InterlockedDecrement(&m_ref);
    if (!c) delete this; return c;
}

// IPropertyStore
HRESULT PdfPropertyHandler::GetCount(DWORD* cProps)
{ if (!cProps) return E_POINTER; *cProps = ARRAYSIZE(s_props); return S_OK; }

HRESULT PdfPropertyHandler::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{ if (!pkey) return E_POINTER; if (iProp >= ARRAYSIZE(s_props)) return E_INVALIDARG; *pkey = s_props[iProp]; return S_OK; }

HRESULT PdfPropertyHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if (!pv) return E_POINTER; PropVariantInit(pv);
    if (key == PKEY_Title) return InitPropVariantFromString(m_meta.title.c_str(), pv);
    if (key == PKEY_Author) return InitPropVariantFromString(m_meta.author.c_str(), pv);
    if (key == PKEY_Subject) return InitPropVariantFromString(m_meta.subject.c_str(), pv);
    if (key == PKEY_Keywords) return InitPropVariantFromString(m_meta.keywords.c_str(), pv);
    return S_FALSE;
}

// IInitializeWithStream
HRESULT PdfPropertyHandler::Initialize(IStream* pstream, DWORD /*grfMode*/)
{
    if (!pstream) return E_INVALIDARG;
    if (m_loaded) return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    m_loaded = ExtractPdfMetadataFromStream(pstream, m_meta);
    return m_loaded ? S_OK : E_FAIL;
}

//=== COM server exports ===

static HRESULT RegisterServerInternal(BOOL doRegister)
{
    const CLSID clsid = GetHandlerClsid();
    LPOLESTR clsidStr = nullptr;
    StringFromCLSID(clsid, &clsidStr);

    HKEY hKey = nullptr; LONG r;
    wchar_t modulePath[MAX_PATH]; GetModuleFileNameW((HMODULE)&__ImageBase, modulePath, MAX_PATH);

    // HKCR\CLSID\{CLSID}
    wchar_t clsidKey[256]; StringCchPrintfW(clsidKey, 256, L"CLSID\%s", clsidStr);

    if (doRegister)
    {
        // create CLSID key
        r = RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidKey, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (r != ERROR_SUCCESS) goto done;
        const wchar_t* friendly = L"PDF Property Handler";
        RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)friendly, (DWORD)((wcslen(friendly)+1)*sizeof(wchar_t)));
        RegCloseKey(hKey); hKey = nullptr;

        // InprocServer32
        wchar_t ips32[300]; StringCchPrintfW(ips32, 300, L"%s\InprocServer32", clsidKey);
        r = RegCreateKeyExW(HKEY_CLASSES_ROOT, ips32, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (r != ERROR_SUCCESS) goto done;
        RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)modulePath, (DWORD)((wcslen(modulePath)+1)*sizeof(wchar_t)));
        const wchar_t* tm = L"Both";
        RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (const BYTE*)tm, (DWORD)((wcslen(tm)+1)*sizeof(wchar_t)));
        RegCloseKey(hKey); hKey = nullptr;

        // HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.pdf
        r = RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.pdf", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (r == ERROR_SUCCESS) {
            RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)clsidStr, (DWORD)((wcslen(clsidStr)+1)*sizeof(wchar_t)));
            RegCloseKey(hKey); hKey = nullptr;
        }

        // HKCR \.pdf\shellex\PropertyHandler
        r = RegCreateKeyExW(HKEY_CLASSES_ROOT, L".pdf\shellex\PropertyHandler", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (r == ERROR_SUCCESS) {
            RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)clsidStr, (DWORD)((wcslen(clsidStr)+1)*sizeof(wchar_t)));
            RegCloseKey(hKey); hKey = nullptr;
        }

        // (Optional) Approved list
        r = RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (r == ERROR_SUCCESS) {
            const wchar_t* desc = L"PDF Property Handler";
            RegSetValueExW(hKey, clsidStr, 0, REG_SZ, (const BYTE*)desc, (DWORD)((wcslen(desc)+1)*sizeof(wchar_t)));
            RegCloseKey(hKey); hKey = nullptr;
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
    else
    {
        // delete Approved value
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            RegDeleteValueW(hKey, clsidStr);
            RegCloseKey(hKey); hKey = nullptr;
        }
        // delete .pdf shellex PropertyHandler
        RegDeleteTreeW(HKEY_CLASSES_ROOT, L".pdf\shellex\PropertyHandler");
        // delete PropertyHandlers mapping
        RegDeleteTreeW(HKEY_LOCAL_MACHINE, L"SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.pdf");
        // delete CLSID keys
        RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidKey);

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }

    done:
    if (clsidStr) CoTaskMemFree(clsidStr);
    if (hKey) RegCloseKey(hKey);
    return (doRegister ? S_OK : S_OK);
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (IsEqualCLSID(rclsid, GetHandlerClsid())) {
        PdfClassFactory* factory = new (std::nothrow) PdfClassFactory();
        if (!factory) return E_OUTOFMEMORY;
        HRESULT hr = factory->QueryInterface(riid, ppv);
        factory->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

extern "C" HRESULT __stdcall DllCanUnloadNow(void)
{
    return (g_cDllRef == 0) ? S_OK : S_FALSE;
}

extern "C" HRESULT __stdcall DllRegisterServer(void)
{
    return RegisterServerInternal(TRUE);
}

extern "C" HRESULT __stdcall DllUnregisterServer(void)
{
    return RegisterServerInternal(FALSE);
}
