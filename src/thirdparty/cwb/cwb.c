/**
 * CWB API (Plain C WebBrowser Control mini API) - Implementation file
 *
 * Version 1.0, Jan 2022 by Eric M.
 *
 * CWB (C WebBrowser) API is a minimalist implementation of a WebBrowser control
 * adapted to C WinAPI environment. The API allow to embed a WebBrowser
 * control instance (Internet Explorer/Trident) within a common WinAPI window.
 *
 * This mini API is widely inspired from the Codeguru's CWebPage.c example (see
 * link bellow) with help of the René Nyffenegger's C++ implementation (see link
 * bellow) for frame resize handling and navigation interception.
 *
 * Codeguru's Plain C Web Page tutorial :
 *  https://www.codeguru.com/network/display-a-web-page-in-a-plain-c-win32-application/
 *
 * René Nyffenegger's MSHTML Web Browser demonstration :
 *  https://renenyffenegger.ch/notes/Windows/development/MSHTML/index
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software in
 *     a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not
 *     be misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 */
#include <windows.h>
#include <exdisp.h>     // defines interfaces for IWebBrowser2, IWebBrowserEvent2
#include <exdispid.h>   // defines EXDIS DISPID_ values
#include <mshtml.h>     // defines interfaces for IHTMLDocument2.
#include <mshtmdid.h>   // defines MSHTML DISPID_ values

#define VERSION 1.0

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MINGW32__
/* Missing GUID definition for MinGW */
const GUID DECLSPEC_SELECTANY DIID_HTMLDocumentEvents2 = { 0x30510737, 0x98b5, 0x11cf, {0xbb,0x82, 0x00,0xaa,0x00,0xbd,0xce,0x0b}};
#endif // __MINGW32__

/* WindowProc callback function declaration */
LRESULT CALLBACK __CwbWndProc(HWND, UINT, WPARAM, LPARAM);

/* BeforeNavigate callback function declaration */
typedef void (*CwbBeforeNav)(LPCWSTR lpUrl, SHORT* pCancel);

/* HTML View custom container early declaration */
typedef struct _CwbContainer CwbContainer;

/** \brief Extended IStorage interface
 *
 * Custom structure that extend the IStorage interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbStorage
{
  /* IOleClientSite interface */
	IStorage            st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbStorage;


/** \brief Extended IOleClientSite interface
 *
 * Custom structure that extend the IOleClientSite interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbOleClientSite
{
  /* IOleClientSite interface */
	IOleClientSite      st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbOleClientSite;


/** \brief Extended IOleInPlaceSite interface
 *
 * Custom structure that extend the IOleInPlaceSite interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbOleInPlaceSite
{
  /* IOleInPlaceSite interface */
	IOleInPlaceSite     st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbOleInPlaceSite;


/** \brief Extended IOleInPlaceFrame interface
 *
 * Custom structure that extend the IOleInPlaceFrame interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbOleInPlaceFrame
{
  /* IOleInPlaceFrame interface */
	IOleInPlaceFrame    st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbOleInPlaceFrame;


/** \brief Extended DWebBrowserEvents2 interface
 *
 * Custom structure that extend the DWebBrowserEvents2 interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbWebBrowserEvents2
{
  /* DWebBrowserEvents2 interface */
	DWebBrowserEvents2  st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbWebBrowserEvents2;


/** \brief Extended IOleClientSite interface
 *
 * Custom structure that extend the IOleClientSite interface
 * to hold a pointer to the custom container structure.
 */
typedef struct _CwbHTMLDocumentEvents2
{
  /* IOleClientSite interface */
	HTMLDocumentEvents2 st;
	/* Pointer to HTML View custom container */
	CwbContainer*       pContainer;

} CwbHTMLDocumentEvents2;


/** \brief Custom container structure definition
 *
 * Custom structure to hold all needed interfaces pointers and
 * references related to the Web Browser object.
 *
 */
typedef struct _CwbContainer
{
  /* Extended IStorage interface structure */
  CwbStorage              Storage;
  /* Extended IOleClientSite interface structure */
  CwbOleClientSite        OleClientSite;
  /* Extended IOleInPlaceSite interface structure */
  CwbOleInPlaceSite	      OleInPlaceSite;
  /* Extended IOleInPlaceFrame interface structure */
  CwbOleInPlaceFrame      OleInPlaceFrame;
  /* Extended DWebBrowserEvents2 interface structure */
  CwbWebBrowserEvents2    WebBrowserEvents2;
  /* Extended DWebBrowserEvents2 interface structure */
  CwbHTMLDocumentEvents2  HTMLDocumentEvents2;

  /* Web Browser IOleObject interface pointer */
  IOleObject*             pOleObject;
  /* Web Browser IWebBrowser2 interface pointer */
  IWebBrowser2*           pWebBrowser2;
  /* Web Browser IHTMLDocument2 interface pointer */
  IHTMLDocument2*         pHTMLDocument2;

  /* BeforeNavigate callback function pointer */
  CwbBeforeNav            BeforeNav;

  /* Owner window handle */
  HWND                    hWnd;

} CwbContainer;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Custom IStorage interface implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static HRESULT STDMETHODCALLTYPE __CwbStorage_QueryInterface(IStorage* This, REFIID riid, LPVOID* ppvObj) {return E_NOTIMPL;}
static ULONG STDMETHODCALLTYPE __CwbStorage_AddRef(IStorage* This) {return 1;}
static ULONG STDMETHODCALLTYPE __CwbStorage_Release(IStorage* This) {return 1;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_CreateStream(IStorage* This, const WCHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_OpenStream(IStorage* This, const WCHAR * pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_CreateStorage(IStorage* This, const WCHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage **ppstg) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_OpenStorage(IStorage* This, const WCHAR * pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_CopyTo(IStorage* This, DWORD ciidExclude, IID const *rgiidExclude, SNB snbExclude,IStorage *pstgDest) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_MoveElementTo(IStorage* This, const OLECHAR *pwcsName,IStorage * pstgDest, const OLECHAR *pwcsNewName, DWORD grfFlags) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_Commit(IStorage* This, DWORD grfCommitFlags) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_Revert(IStorage* This) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_EnumElements(IStorage* This, DWORD reserved1, void * reserved2, DWORD reserved3, IEnumSTATSTG ** ppenum) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_DestroyElement(IStorage* This, const OLECHAR *pwcsName) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_RenameElement(IStorage* This, const WCHAR *pwcsOldName, const WCHAR *pwcsNewName) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_SetElementTimes(IStorage* This, const WCHAR *pwcsName, FILETIME const *pctime, FILETIME const *patime, FILETIME const *pmtime) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_SetClass(IStorage* This, REFCLSID clsid) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_SetStateBits(IStorage* This, DWORD grfStateBits, DWORD grfMask) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbStorage_Stat(IStorage* This, STATSTG * pstatstg, DWORD grfStatFlag) {return E_NOTIMPL;}
/* Custom vtable for IStorage interface */
static IStorageVtbl __CwbStorageVtbl = {
  __CwbStorage_QueryInterface,
  __CwbStorage_AddRef,
  __CwbStorage_Release,
  __CwbStorage_CreateStream,
  __CwbStorage_OpenStream,
  __CwbStorage_CreateStorage,
  __CwbStorage_OpenStorage,
  __CwbStorage_CopyTo,
  __CwbStorage_MoveElementTo,
  __CwbStorage_Commit,
  __CwbStorage_Revert,
  __CwbStorage_EnumElements,
  __CwbStorage_DestroyElement,
  __CwbStorage_RenameElement,
  __CwbStorage_SetElementTimes,
  __CwbStorage_SetClass,
  __CwbStorage_SetStateBits,
  __CwbStorage_Stat
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * HTMLDocumentEvents2 interface custom implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
HRESULT STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_QueryInterface(HTMLDocumentEvents2* This, REFIID riid, void** ppvObject)
{
  CwbContainer* pContainer = ((CwbOleClientSite*)This)->pContainer;

	if( !memcmp(riid, &IID_IUnknown, sizeof(GUID)) ||
      !memcmp(riid, &IID_IDispatch, sizeof(GUID)) ||
      !memcmp(riid, &DIID_HTMLDocumentEvents2, sizeof(GUID))) {

		(*ppvObject) = (void**)&pContainer->HTMLDocumentEvents2;

		return S_OK;
	}

  (*ppvObject) = 0;

  return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_AddRef(HTMLDocumentEvents2* This) {return 1;}
ULONG STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_Release(HTMLDocumentEvents2* This) {return 1;}
HRESULT STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_GetTypeInfoCount(HTMLDocumentEvents2* This,UINT *pctinfo) {return E_NOTIMPL;}
HRESULT STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_GetTypeInfo(HTMLDocumentEvents2* This, UINT iTInfo, LCID lcid,ITypeInfo **ppTInfo) {return E_NOTIMPL;}
HRESULT STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_GetIDsOfNames(HTMLDocumentEvents2* This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) {return E_NOTIMPL;}
HRESULT STDMETHODCALLTYPE __CwbHTMLDocumentEvents2_Invoke(HTMLDocumentEvents2* This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr)
{
  CwbContainer* pContainer = ((CwbOleClientSite*)This)->pContainer;

  IHTMLEventObj* pIHTMLEventObj;

  if(pDispParams->rgvarg[0].vt == VT_DISPATCH) {
    IDispatch* pDispatch = pDispParams->rgvarg[0].pdispVal;
    if(S_OK != pDispatch->lpVtbl->QueryInterface(pDispatch, &IID_IHTMLEventObj, (void**)&pIHTMLEventObj)) {
      return E_UNEXPECTED;
    }
    pDispatch->lpVtbl->Release(pDispatch);
  }

  switch(dispIdMember)
  {
  case DISPID_EVMETH_ONMOUSEMOVE:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONMOUSEDOWN:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONMOUSEUP:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONCLICK:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONKEYDOWN:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONKEYUP:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_EVMETH_ONKEYPRESS:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_ONMOUSEWHEEL:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_ONFOCUSIN:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_ONFOCUSOUT:
    // Do something here with IHTMLEventObj
    return S_OK;

  case DISPID_ONCONTEXTMENU:
    // Do something here with IHTMLEventObj
    return S_OK;
  }

  return DISP_E_MEMBERNOTFOUND;
}

/* Custom vtable for HTMLDocumentEvents2 interface */
static HTMLDocumentEvents2Vtbl __CwbHTMLDocumentEvents2Vtbl = {
  __CwbHTMLDocumentEvents2_QueryInterface,
  __CwbHTMLDocumentEvents2_AddRef,
  __CwbHTMLDocumentEvents2_Release,
  __CwbHTMLDocumentEvents2_GetTypeInfoCount,
  __CwbHTMLDocumentEvents2_GetTypeInfo,
  __CwbHTMLDocumentEvents2_GetIDsOfNames,
  __CwbHTMLDocumentEvents2_Invoke
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * DWebBrowserEvents2 interface custom implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static HRESULT STDMETHODCALLTYPE __CwbWebBrowserEvent2_QueryInterface(DWebBrowserEvents2* This, REFIID riid, void **ppvObject)
{
  CwbContainer* pContainer = ((CwbOleClientSite*)This)->pContainer;

	if( !memcmp(riid, &IID_IUnknown, sizeof(GUID)) ||
      !memcmp(riid, &IID_IDispatch, sizeof(GUID)) ||
      !memcmp(riid, &DIID_DWebBrowserEvents2, sizeof(GUID))) {

		(*ppvObject) = (void**)&pContainer->WebBrowserEvents2;

		return S_OK;
	}

  (*ppvObject) = 0;

  return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE __CwbWebBrowserEvent2_AddRef(DWebBrowserEvents2* This) {return 1;}
static ULONG STDMETHODCALLTYPE __CwbWebBrowserEvent2_Release(DWebBrowserEvents2 * This) {return 1;}
static HRESULT STDMETHODCALLTYPE __CwbWebBrowserEvent2_GetTypeInfoCount(DWebBrowserEvents2* This, UINT *pctinfo) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbWebBrowserEvent2_GetTypeInfo(DWebBrowserEvents2* This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbWebBrowserEvent2_GetIDsOfNames(DWebBrowserEvents2* This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbWebBrowserEvent2_Invoke(DWebBrowserEvents2* This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  CwbContainer* pContainer = ((CwbOleClientSite*)This)->pContainer;

  if(dispIdMember == DISPID_BEFORENAVIGATE2) {

    VARIANT_BOOL* pCancel = pDispParams->rgvarg[0].pboolVal;
    //VARIANT* pvHeaders = pDispParams->rgvarg[1].pvarVal;
    //VARIANT* pvPostData = pDispParams->rgvarg[2].pvarVal;
    //VARIANT* pvTarget = pDispParams->rgvarg[3].pvarVal;
    //VARIANT* pvFlags = pDispParams->rgvarg[4].pvarVal;
    VARIANT* pvUrl = pDispParams->rgvarg[5].pvarVal;

    // prevent navigation to 'about:blank' to be catch or canceled by callback
    if(wcscmp(pvUrl->bstrVal, L"about:blank")) {

      // pass URL and Cancel pointer to callback if exists
      if(pContainer->BeforeNav) {
        pContainer->BeforeNav(pvUrl->bstrVal, pCancel);
      }
    }

    return S_OK;
  }

  return DISP_E_MEMBERNOTFOUND;
}

/* Custom vtable for DWebBrowserEvents2 interface */
static DWebBrowserEvents2Vtbl __CwbWebBrowserEvents2Vtbl = {
  __CwbWebBrowserEvent2_QueryInterface,
  __CwbWebBrowserEvent2_AddRef,
  __CwbWebBrowserEvent2_Release,
  __CwbWebBrowserEvent2_GetTypeInfoCount,
  __CwbWebBrowserEvent2_GetTypeInfo,
  __CwbWebBrowserEvent2_GetIDsOfNames,
  __CwbWebBrowserEvent2_Invoke
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * IOleInPlaceFrame interface custom implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_QueryInterface(IOleInPlaceFrame* This, REFIID riid, LPVOID* ppvObj) {return E_NOTIMPL;}
static ULONG STDMETHODCALLTYPE __CwbOleInPlaceFrame_AddRef(IOleInPlaceFrame* This) {return 1;}
static ULONG STDMETHODCALLTYPE __CwbOleInPlaceFrame_Release(IOleInPlaceFrame* This) {return 1;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_GetWindow(IOleInPlaceFrame* This, HWND* lphWnd)
{
  CwbContainer* pContainer = ((CwbOleInPlaceFrame*)This)->pContainer;

  (*lphWnd) = pContainer->hWnd;

  return S_OK;
}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_ContextSensitiveHelp(IOleInPlaceFrame* This, BOOL fEnterMode) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_GetBorder(IOleInPlaceFrame* This, LPRECT lprectBorder) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_RequestBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_SetBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_SetActiveObject(IOleInPlaceFrame* This, IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_InsertMenus(IOleInPlaceFrame* This, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_SetMenu(IOleInPlaceFrame* This, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_RemoveMenus(IOleInPlaceFrame* This, HMENU hmenuShared) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_SetStatusText(IOleInPlaceFrame* This, LPCOLESTR pszStatusText) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_EnableModeless(IOleInPlaceFrame* This, BOOL fEnable) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceFrame_TranslateAccelerator(IOleInPlaceFrame* This, LPMSG lpmsg, WORD wID) {return S_OK;}

/* Custom vtable for IOleInPlaceFrame interface */
static IOleInPlaceFrameVtbl __CwbOleInPlaceFrameVtbl = {
  __CwbOleInPlaceFrame_QueryInterface,
  __CwbOleInPlaceFrame_AddRef,
  __CwbOleInPlaceFrame_Release,
  __CwbOleInPlaceFrame_GetWindow,
  __CwbOleInPlaceFrame_ContextSensitiveHelp,
  __CwbOleInPlaceFrame_GetBorder,
  __CwbOleInPlaceFrame_RequestBorderSpace,
  __CwbOleInPlaceFrame_SetBorderSpace,
  __CwbOleInPlaceFrame_SetActiveObject,
  __CwbOleInPlaceFrame_InsertMenus,
  __CwbOleInPlaceFrame_SetMenu,
  __CwbOleInPlaceFrame_RemoveMenus,
  __CwbOleInPlaceFrame_SetStatusText,
  __CwbOleInPlaceFrame_EnableModeless,
  __CwbOleInPlaceFrame_TranslateAccelerator
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * IOleClientSite interface custom implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_QueryInterface(IOleClientSite* This, REFIID riid, void ** ppvObject)
{
  CwbContainer* pContainer = ((CwbOleClientSite*)This)->pContainer;

	if(!memcmp(riid, &IID_IUnknown, sizeof(GUID)) || !memcmp(riid, &IID_IOleClientSite, sizeof(GUID))) {

		(*ppvObject) = (void**)&pContainer->OleClientSite;

		return S_OK;

	} else if (!memcmp(riid, &IID_IOleInPlaceSite, sizeof(GUID))) {

		(*ppvObject) = (void**)&pContainer->OleInPlaceSite;

		return S_OK;

	} else if (!memcmp(riid, &DIID_DWebBrowserEvents2, sizeof(GUID))) {

		(*ppvObject) = (void**)&pContainer->WebBrowserEvents2;

		return S_OK;
	}

  (*ppvObject) = 0;

  return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE __CwbOleClientSite_AddRef(IOleClientSite* This) {return 1;}
static ULONG STDMETHODCALLTYPE __CwbOleClientSite_Release(IOleClientSite* This) {return 1;}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_SaveObject(IOleClientSite* This) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_GetMoniker(IOleClientSite* This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_GetContainer(IOleClientSite* This, LPOLECONTAINER* ppContainer)
{
	*ppContainer = 0;

	return E_NOINTERFACE;
}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_ShowObject(IOleClientSite* This) {return NOERROR;}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_OnShowWindow(IOleClientSite* This, BOOL fShow) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleClientSite_RequestNewObjectLayout(IOleClientSite* This) {return E_NOTIMPL;}

/* Custom vtable for IOleClientSite interface */
static IOleClientSiteVtbl __CwbOleClientSiteVtbl = {
  __CwbOleClientSite_QueryInterface,
  __CwbOleClientSite_AddRef,
  __CwbOleClientSite_Release,
  __CwbOleClientSite_SaveObject,
  __CwbOleClientSite_GetMoniker,
  __CwbOleClientSite_GetContainer,
  __CwbOleClientSite_ShowObject,
  __CwbOleClientSite_OnShowWindow,
  __CwbOleClientSite_RequestNewObjectLayout
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * IOleInPlaceSite interface custom implementation
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_QueryInterface(IOleInPlaceSite* This, REFIID riid, void ** ppvObject)
{
  CwbContainer* pContainer = ((CwbOleInPlaceSite*)This)->pContainer;

	if(!memcmp(riid, &IID_IOleInPlaceSite, sizeof(GUID))) {

		(*ppvObject) = &pContainer->OleInPlaceSite;

		return S_OK;
	}

  (*ppvObject) = 0;

  return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE __CwbOleInPlaceSite_AddRef(IOleInPlaceSite* This) {return 1;}
static ULONG STDMETHODCALLTYPE __CwbOleInPlaceSite_Release(IOleInPlaceSite* This) {return 1;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_GetWindow(IOleInPlaceSite* This, HWND* lphwnd)
{
  CwbContainer* pContainer = ((CwbOleInPlaceSite*)This)->pContainer;

	*lphwnd = pContainer->hWnd;

	return S_OK;
}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_ContextSensitiveHelp(IOleInPlaceSite* This, BOOL fEnterMode) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_CanInPlaceActivate(IOleInPlaceSite* This) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_OnInPlaceActivate(IOleInPlaceSite* This) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_OnUIActivate(IOleInPlaceSite* This) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_GetWindowContext(IOleInPlaceSite* This, LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
  CwbContainer* pContainer = ((CwbOleInPlaceSite*)This)->pContainer;

	(*lplpFrame) = (LPOLEINPLACEFRAME)&pContainer->OleInPlaceFrame;

	(*lplpDoc) = 0;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = pContainer->hWnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	GetClientRect(lpFrameInfo->hwndFrame, lprcPosRect);
	GetClientRect(lpFrameInfo->hwndFrame, lprcClipRect);

	return S_OK;
}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_Scroll(IOleInPlaceSite* This, SIZE scrollExtent) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_OnUIDeactivate(IOleInPlaceSite* This, BOOL fUndoable) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_OnInPlaceDeactivate(IOleInPlaceSite* This) {return S_OK;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_DiscardUndoState(IOleInPlaceSite* This) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_DeactivateAndUndo(IOleInPlaceSite* This) {return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE __CwbOleInPlaceSite_OnPosRectChange(IOleInPlaceSite* This, LPCRECT lprcPosRect)
{
  CwbContainer* pContainer = ((CwbOleInPlaceSite*)This)->pContainer;

  IOleObject* pOleObject = pContainer->pOleObject;

	IOleInPlaceObject* pOleInPlaceObject = NULL;
	pOleObject->lpVtbl->QueryInterface(pOleObject, &IID_IOleInPlaceObject, (void**)&pOleInPlaceObject);
	if(!pOleInPlaceObject) return E_UNEXPECTED;

  pOleInPlaceObject->lpVtbl->SetObjectRects(pOleInPlaceObject, lprcPosRect, lprcPosRect);

	return S_OK;
}

/* Custom vtable for IOleInPlaceSite interface */
static IOleInPlaceSiteVtbl __CwbOleInPlaceSiteVtbl = {
  __CwbOleInPlaceSite_QueryInterface,
  __CwbOleInPlaceSite_AddRef,
  __CwbOleInPlaceSite_Release,
  __CwbOleInPlaceSite_GetWindow,
  __CwbOleInPlaceSite_ContextSensitiveHelp,
  __CwbOleInPlaceSite_CanInPlaceActivate,
  __CwbOleInPlaceSite_OnInPlaceActivate,
  __CwbOleInPlaceSite_OnUIActivate,
  __CwbOleInPlaceSite_GetWindowContext,
  __CwbOleInPlaceSite_Scroll,
  __CwbOleInPlaceSite_OnUIDeactivate,
  __CwbOleInPlaceSite_OnInPlaceDeactivate,
  __CwbOleInPlaceSite_DiscardUndoState,
  __CwbOleInPlaceSite_DeactivateAndUndo,
  __CwbOleInPlaceSite_OnPosRectChange
};



/** \brief Write to Web Browser document (DOM)
 *
 * Write the given HTML content into Web Browser HTML document (DOM).
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 * \param[in]  lpHtml  HTML content to write.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserDocumentWrite(HWND hWnd, LPCWSTR lpHtml)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return -1;

  // get pointer to IHTMLDocument2 interface
  IHTMLDocument2* pHTMLDocument2 = pContainer->pHTMLDocument2;

  // allocate new SAFEARRAY to be passed to write method
  SAFEARRAY* pSafeArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);
  if(!pSafeArray) return -2;

  // get access to VARIANT array pointer
  VARIANT* vHtml;
  SafeArrayAccessData(pSafeArray, (void**)&vHtml);

  // add HTML data to the first (and alone) VARIANT
  vHtml[0].vt = VT_BSTR; // VARIANT type
  vHtml[0].bstrVal = SysAllocString(lpHtml); // allocate

  // write data to HTML document (DOM)
  pHTMLDocument2->lpVtbl->write(pHTMLDocument2, pSafeArray);

  // freed the previously allocated data
  SafeArrayDestroy(pSafeArray);

  return 0;
}


/** \brief Close Web Browser document (DOM)
 *
 * Closes (reset) the Web Browser HTML document (DOM).
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserDocumentClose(HWND hWnd)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return -1;

  // get pointer to IHTMLDocument2 interface
  IHTMLDocument2* pHTMLDocument2 = pContainer->pHTMLDocument2;

  // close the document to clear it
  pHTMLDocument2->lpVtbl->close(pHTMLDocument2);

  return 0;
}


/** \brief Open location
 *
 * Orders the specified instance to open the location
 * at given URL.
 *
 * \param[in]  hWnd     Handle to the window with Web Browser instance.
 * \param[in]  lpUrl    URL to open.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserNavigate(HWND hWnd, LPCWSTR lpUrl)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return -1;

  // get pointer to IWebBrowser2 interface
  IWebBrowser2* pWebBrowser2 = pContainer->pWebBrowser2;

  // create VARIANT object to pass as URL
  VARIANT vUrl;
  vUrl.vt = VT_BSTR; //< VARIANT type
  vUrl.bstrVal = SysAllocString(lpUrl); //< allocate BSTR copy of string

  VARIANT vFlags;
  vFlags.vt = VT_I4; //< VARIANT type
  vFlags.lVal = navNoWriteToCache|navEnforceRestricted;

  pWebBrowser2->lpVtbl->Navigate2(pWebBrowser2, &vUrl, &vFlags, NULL, NULL, NULL);

  // freed the previously allocated data
  VariantClear(&vUrl);

  /* alternative code to open URL
  // get pointer to IHTMLWindow2 interface
  IHTMLWindow2* pHTMLWindow2 = pContainer->pHTMLWindow2;

  BSTR bUrl = SysAllocString(lpUrl);

  //IHTMLWindow2* pNewWindow2	= NULL;
  //pHTMLWindow2->lpVtbl->open(pHTMLWindow2, bUrl, L"_self", NULL, VARIANT_TRUE, &pNewWindow2);
  pHTMLWindow2->lpVtbl->navigate(pHTMLWindow2, bUrl);

  SysFreeString(bUrl);
  */

  return 0;
}


/** \brief Resize Web Browser frame
 *
 * Resizes the Web Browser frame.
 *
 * \param[in]  hWnd     Handle to the window with Web Browser instance.
 * \param[in]  w        The new width of the frame, in pixels.
 * \param[in]  h        The new height of the frame, in pixels.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserResize(HWND hWnd, int w, int h)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return -1;

  // get pointer to IWebBrowser2 interface
  IWebBrowser2* pWebBrowser2 = pContainer->pWebBrowser2;
  if(!pWebBrowser2) return -1;

  // adjust Web Browser to owner client area
  pWebBrowser2->lpVtbl->put_Width(pWebBrowser2, w);
  pWebBrowser2->lpVtbl->put_Height(pWebBrowser2, h);

  return 0;
}


/** \brief Set Before Navigation callback
 *
 * Defines the Before Navigation callback function the specified instance
 *
 * \param[in]  hWnd         Handle to the window with Web Browser instance.
 * \param[in]  pBeforeNav   Pointer to Before Navigate callback function.
 *
 * \return The previously assigned function pointer.
 */
CwbBeforeNav WebBrowserSetBeforeNav(HWND hWnd, CwbBeforeNav pBeforeNav)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return NULL;

  // store old pointer
  CwbBeforeNav pOldBeforeNav = pContainer->BeforeNav;

  // set callback function
  pContainer->BeforeNav = pBeforeNav;

  return pOldBeforeNav;
}


/** \brief Release Web Browser instance
 *
 * Releases a previously attached Web Browser instance from a window.
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserRelease(HWND hWnd)
{
  // get pointer to custom container structure
  CwbContainer* pContainer = (CwbContainer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if(!pContainer) return -1;

  // release IWebBrowser2 interface if exists
  if(pContainer->pWebBrowser2)
    pContainer->pWebBrowser2->lpVtbl->Release(pContainer->pWebBrowser2);

  // release IHTMLDocument2 interface if exists
  if(pContainer->pHTMLDocument2)
    pContainer->pHTMLDocument2->lpVtbl->Release(pContainer->pHTMLDocument2);

  // close and release WebBrowser OleObject if exists
  if(pContainer->pOleObject) {
    pContainer->pOleObject->lpVtbl->Close(pContainer->pOleObject, OLECLOSE_NOSAVE);
    pContainer->pOleObject->lpVtbl->Release(pContainer->pOleObject);
  }

  // free the custom container structure allocated memory
  GlobalFree((HGLOBAL)pContainer);

  // invalidate pointer to prevent double-free
  SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);

  return 0;
}


/** \brief Attach Web Browser to window
 *
 * Attaches a new Web Browser instance to an existing window.
 *
 * \param[in]  hWnd         Window handle to attach Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserAttach(HWND hWnd)
{
  // allocate new custom container structure
  CwbContainer* pContainer = (CwbContainer*)GlobalAlloc(GMEM_FIXED, sizeof(CwbContainer));
  if(!pContainer) return -2;

  // set window user data pointer to custom container structure
  SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pContainer);

  // initialize custom container structure members and values
  pContainer->Storage.st.lpVtbl = &__CwbStorageVtbl;
  pContainer->Storage.pContainer = pContainer;
  pContainer->OleClientSite.st.lpVtbl = &__CwbOleClientSiteVtbl;
  pContainer->OleClientSite.pContainer = pContainer;
  pContainer->OleInPlaceSite.st.lpVtbl = &__CwbOleInPlaceSiteVtbl;
  pContainer->OleInPlaceSite.pContainer = pContainer;
  pContainer->OleInPlaceFrame.st.lpVtbl = &__CwbOleInPlaceFrameVtbl;
  pContainer->OleInPlaceFrame.pContainer = pContainer;
  pContainer->WebBrowserEvents2.st.lpVtbl = &__CwbWebBrowserEvents2Vtbl;
  pContainer->WebBrowserEvents2.pContainer = pContainer;
  pContainer->HTMLDocumentEvents2.st.lpVtbl = &__CwbHTMLDocumentEvents2Vtbl;
  pContainer->HTMLDocumentEvents2.pContainer = pContainer;
  pContainer->pOleObject = NULL;
  pContainer->pWebBrowser2 = NULL;
  pContainer->pHTMLDocument2 = NULL;
  pContainer->BeforeNav = NULL;
  pContainer->hWnd = hWnd;

  // get local variables to some interfaces
  IStorage* pStorage = (IStorage*)&pContainer->Storage;

  // create new WebBrowser OleObject
  IOleClientSite* pOleClientSite = (IOleClientSite*)&pContainer->OleClientSite;
  IOleObject* pOleObject = NULL;
	if(S_OK != OleCreate(&CLSID_WebBrowser, &IID_IOleObject, OLERENDER_DRAW, NULL, pOleClientSite, pStorage, (void**)&pOleObject)) {
    WebBrowserRelease(hWnd); return -3;
	}
	pContainer->pOleObject = pOleObject; //< store pointer into custom container struct

  // Order the OleObject (which embed IWebBrowser2) to show up itself
  RECT rect;
  GetClientRect(hWnd, &rect); // use entire client area of parent window
  if(S_OK != pOleObject->lpVtbl->DoVerb(pOleObject, OLEIVERB_SHOW, NULL, pOleClientSite, 0, hWnd, &rect)) {
    WebBrowserRelease(hWnd); return -3;
  }

  // set host window title as hostname
  wchar_t title[64];
  GetWindowTextW(hWnd, title, 64);
  pOleObject->lpVtbl->SetHostNames(pOleObject, title, 0);

  // this seem useless...
  if(OleSetContainedObject((IUnknown*)pOleObject, TRUE)) {
    WebBrowserRelease(hWnd);
    return -3;
  }

  // Retrieve the IWebBrowser2 object embedded in OleObject
  IWebBrowser2* pWebBrowser2 = NULL;
  if(S_OK != pOleObject->lpVtbl->QueryInterface(pOleObject, &IID_IWebBrowser2, (void**)&pWebBrowser2)) {
    WebBrowserRelease(hWnd); return -4;
  }
  pContainer->pWebBrowser2 = pWebBrowser2; //< store pointer into custom container struct


  // opens the 'about:blank' page to force Web Browser to create a "safe" HTML document (DOM)
  if(S_OK != WebBrowserNavigate(hWnd, L"about:blank")) {
    WebBrowserRelease(hWnd); return -5;
  }

  // get the HTML document (DOM) from Web Browser
  IDispatch *pDispatch = NULL;
  if(S_OK != pWebBrowser2->lpVtbl->get_Document(pWebBrowser2, &pDispatch)) {
    WebBrowserRelease(hWnd); return -6;
  }

  Sleep(0);

  IHTMLDocument2* pHTMLDocument2 = NULL;
  pDispatch->lpVtbl->QueryInterface(pDispatch, &IID_IHTMLDocument2, (void**)&pHTMLDocument2);
  pDispatch->lpVtbl->Release(pDispatch); //< we do not need IDispatch anymore
  if(!pHTMLDocument2) {
    WebBrowserRelease(hWnd); return -7;
  }
  pContainer->pHTMLDocument2 = pHTMLDocument2; //< store pointer into custom container struct

  // Add "Sink" connections to forward Web Browsers events to implemented callbacks
  IConnectionPointContainer* pConnectionPointContainer;
  IConnectionPoint* pConnectionPoint;
  DWORD dwCookie;

  // get OleObject connection container object to find connection point to DWebBrowserEvent2
  pConnectionPointContainer = NULL;
  if(S_OK != pOleObject->lpVtbl->QueryInterface(pOleObject, &IID_IConnectionPointContainer, (void**)&pConnectionPointContainer)) {
    WebBrowserRelease(hWnd); return -8;
  }
  // get connection point to DWebBrowserEvent2
  pConnectionPoint = NULL;
  if(S_OK != pConnectionPointContainer->lpVtbl->FindConnectionPoint(pConnectionPointContainer, &DIID_DWebBrowserEvents2, &pConnectionPoint)) {
    WebBrowserRelease(hWnd); return -8;
  }
  // set connection (Sink) from Web Browser OleObject and DWebBrowserEvents2 interface
  DWebBrowserEvents2* pWebBrowserEvents2 = (DWebBrowserEvents2*)&pContainer->WebBrowserEvents2;
  if(S_OK != pConnectionPoint->lpVtbl->Advise(pConnectionPoint, (IUnknown*)pWebBrowserEvents2, &dwCookie)) {
    WebBrowserRelease(hWnd); return -8;
  }

  /* The following code work but not used yet

  // get HTMLDocument2 connection container object to find connection point to HTMLDocumentEvents2
  pConnectionPointContainer = NULL;
  if(S_OK != pHTMLDocument2->lpVtbl->QueryInterface(pHTMLDocument2, &IID_IConnectionPointContainer, (void**)&pConnectionPointContainer)) {
    WebBrowserRelease(hWnd); return -8;
  }
  // get connection point to HTMLDocumentEvents2
  pConnectionPoint = NULL;
  if(S_OK != pConnectionPointContainer->lpVtbl->FindConnectionPoint(pConnectionPointContainer, &DIID_HTMLDocumentEvents2, &pConnectionPoint)) {
    WebBrowserRelease(hWnd); return -8;
  }
  // set connection (Sink) from HTMLDocument2 and HTMLDocumentEvents2 interface
  HTMLDocumentEvents2* pHTMLDocumentEvents2 = (HTMLDocumentEvents2*)&pContainer->HTMLDocumentEvents2;
  if(S_OK != pConnectionPoint->lpVtbl->Advise(pConnectionPoint, (IUnknown*)pHTMLDocumentEvents2, &dwCookie)) {
    WebBrowserRelease(hWnd); return -8;
  }
  */

  return 0;
}


/*
 * Window class name for internally created and manager windows.
 */
static const char __CwbClsName[] = "CWBAPI_WINDOW";


/** \brief Create Web Browser window
 *
 * Creates a new Web Browser window or child control.
 *
 * This function first create a new window with the given parameters then
 * attach a new Web Browser instance to it. The created window uses use its own
 * Window Procedure to manage its Web Browser instance.
 *
 * This function is the best way to embed a Web Browser as window child
 * control by specifying a parent window handle and WS_CHILD style.
 *
 * \param[in]  dwExStyle    The extended window style of the window being created.
 * \param[in]  dwStyle      The style of the window being created.
 * \param[in]  hWndParent   A handle to the parent or owner window of the window being created.
 * \param[in]  hMenu        A handle to a menu, or specifies a child-window identifier, depending on the window style.
 * \param[in]  hInstance    A handle to a menu, or specifies a child-window identifier, depending on the window style.
 *
 * \return Handle to created window with Web Browser instance or NULL if error.
 */
HWND WebBrowserCreate(DWORD dwExStyle, DWORD dwStyle, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance)
{
  // register new class for window
  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.hInstance = 0;
  wc.lpfnWndProc = __CwbWndProc;
  wc.lpszClassName = &__CwbClsName[0];
  RegisterClassEx(&wc);

  // create the new window
  HWND hWnd = CreateWindowEx(dwExStyle, &__CwbClsName[0], NULL, dwStyle,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hWndParent, hMenu, hInstance, NULL);

  // attach the Web Browser object to this window
  if(hWnd) {
    if(S_OK != WebBrowserAttach(hWnd)) {
      DestroyWindow(hWnd);
      return NULL;
    }
    // automatically show child window
    if(dwStyle & WS_CHILD)
      ShowWindow(hWnd, SW_NORMAL);
  }

  return hWnd;
}


/*
 * Window Proc function for internally created and managed windows.
 */
LRESULT CALLBACK __CwbWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_SIZE:
    WebBrowserResize(hWnd, LOWORD(lParam), HIWORD(lParam));
    return 1;

  case WM_DESTROY:
    WebBrowserRelease(hWnd);
    return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#ifdef __cplusplus
}
#endif
