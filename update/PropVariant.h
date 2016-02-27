// Windows/PropVariant.h

#include "7z.headers\Common\Types.h"
#include "7z.headers\Common\MyWindows.h"

#pragma once

class CPropVariant : public tagPROPVARIANT
{
public:
  CPropVariant() { vt = VT_EMPTY; }
  ~CPropVariant() { Clear(); }
  CPropVariant(const PROPVARIANT& varSrc);
  CPropVariant(const CPropVariant& varSrc);
  CPropVariant(BSTR bstrSrc);
  CPropVariant(LPCOLESTR lpszSrc);
  CPropVariant(bool bSrc) { vt = VT_BOOL; boolVal = (bSrc ? VARIANT_TRUE : VARIANT_FALSE); };
  CPropVariant(unsigned __int32 value) {  vt = VT_UI4; ulVal = value; }
  CPropVariant(unsigned __int64 value) {  vt = VT_UI8; uhVal = *(ULARGE_INTEGER*)&value; }
  CPropVariant(const FILETIME &value) {  vt = VT_FILETIME; filetime = value; }
  CPropVariant(__int32 value) { vt = VT_I4; lVal = value; }
  CPropVariant(unsigned char value) { vt = VT_UI1; bVal = value; }
  CPropVariant(unsigned short value) { vt = VT_I2; iVal = value; }
  // CPropVariant(LONG value, VARTYPE vtSrc = VT_I4) { vt = vtSrc; lVal = value; }

  CPropVariant& operator=(const CPropVariant& varSrc);
  CPropVariant& operator=(const PROPVARIANT& varSrc);
  CPropVariant& operator=(BSTR bstrSrc);
  CPropVariant& operator=(LPCOLESTR lpszSrc);
  CPropVariant& operator=(bool bSrc);
  CPropVariant& operator=(unsigned __int32 value);
  CPropVariant& operator=(unsigned __int64 value);
  CPropVariant& operator=(const FILETIME &value);

  CPropVariant& operator=(__int32 value);
  CPropVariant& operator=(unsigned char value);
  CPropVariant& operator=(unsigned short value);
  // CPropVariant& operator=(LONG  value);

  HRESULT Clear();
  HRESULT Copy(const PROPVARIANT* pSrc);
  HRESULT Attach(PROPVARIANT* pSrc);
  HRESULT Detach(PROPVARIANT* pDest);

  HRESULT InternalClear();
  void InternalCopy(const PROPVARIANT* pSrc);
};

