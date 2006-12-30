#include <objbase.h>

struct __declspec(uuid("4a791148-19e4-11d3-b86b-00c04f79f802"))
Help : IDispatch
{
    virtual HRESULT __stdcall Contents ( ) = 0;
    virtual HRESULT __stdcall Index ( ) = 0;
    virtual HRESULT __stdcall Search ( ) = 0;
    virtual HRESULT __stdcall IndexResults ( ) = 0;
    virtual HRESULT __stdcall SearchResults ( ) = 0;
    virtual HRESULT __stdcall DisplayTopicFromId (
        BSTR bstrFile,
        unsigned long Id ) = 0;
    virtual HRESULT __stdcall DisplayTopicFromURL (
        BSTR pszURL ) = 0;
    virtual HRESULT __stdcall DisplayTopicFromURLEx (
        BSTR pszURL,
        struct IVsHelpTopicShowEvents * pIVsHelpTopicShowEvents ) = 0;
    virtual HRESULT __stdcall DisplayTopicFromKeyword (
        BSTR pszKeyword ) = 0;
    virtual HRESULT __stdcall DisplayTopicFromF1Keyword (
        BSTR pszKeyword ) = 0;
    virtual HRESULT __stdcall DisplayTopicFrom_OLD_Help (
        BSTR bstrFile,
        unsigned long Id ) = 0;
    virtual HRESULT __stdcall SyncContents (
        BSTR bstrURL ) = 0;
    virtual HRESULT __stdcall CanSyncContents (
        BSTR bstrURL ) = 0;
    virtual HRESULT __stdcall GetNextTopic (
        BSTR bstrURL,
        BSTR * pbstrNext ) = 0;
    virtual HRESULT __stdcall GetPrevTopic (
        BSTR bstrURL,
        BSTR * pbstrPrev ) = 0;
    virtual HRESULT __stdcall FilterUI ( ) = 0;
    virtual HRESULT __stdcall CanShowFilterUI ( ) = 0;
    virtual HRESULT __stdcall Close ( ) = 0;
    virtual HRESULT __stdcall SyncIndex (
        BSTR bstrKeyword,
        long fShow ) = 0;
    virtual HRESULT __stdcall SetCollection (
        BSTR bstrCollection,
        BSTR bstrFilter ) = 0;
    virtual HRESULT __stdcall get_Collection (
        BSTR * pbstrCollection ) = 0;
    virtual HRESULT __stdcall get_Filter (
        BSTR * pbstrFilter ) = 0;
    virtual HRESULT __stdcall put_Filter (
        BSTR pbstrFilter ) = 0;
    virtual HRESULT __stdcall get_FilterQuery (
        BSTR * pbstrFilterQuery ) = 0;
    virtual HRESULT __stdcall get_HelpOwner (
        struct IVsHelpOwner * * ppObj ) = 0;
    virtual HRESULT __stdcall put_HelpOwner (
        struct IVsHelpOwner * ppObj ) = 0;
    virtual HRESULT __stdcall get_HxSession (
        IDispatch * * ppObj ) = 0;
    virtual HRESULT __stdcall get_Help (
        IDispatch * * ppObj ) = 0;
    virtual HRESULT __stdcall GetObject (
        BSTR bstrMoniker,
        BSTR bstrOptions,
        IDispatch * * ppDisp ) = 0;
};

struct __declspec(uuid("4a79114d-19e4-11d3-b86b-00c04f79f802"))
DExploreAppObj;

extern "C" const GUID __declspec(selectany) IID_Help =
    {0x4a791148,0x19e4,0x11d3,{0xb8,0x6b,0x00,0xc0,0x4f,0x79,0xf8,0x02}};
extern "C" const GUID __declspec(selectany) CLSID_DExploreAppObj =
    {0x4a79114d,0x19e4,0x11d3,{0xb8,0x6b,0x00,0xc0,0x4f,0x79,0xf8,0x02}};
