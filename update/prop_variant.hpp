#pragma once

#include "7z.headers/Common/MyWindows.h"

class prop_variant: public tagPROPVARIANT
{
public:
	prop_variant()
	{
		vt = VT_EMPTY;
	}

	~prop_variant()
	{
		Clear();
	}

	prop_variant(const PROPVARIANT& Value);
	prop_variant(const prop_variant& Value);
	prop_variant(BSTR Value);
	prop_variant(LPCOLESTR Value);

	prop_variant(bool Value)
	{
		vt = VT_BOOL; boolVal = Value? VARIANT_TRUE : VARIANT_FALSE;
	};

	prop_variant(uint32_t value)
	{
		vt = VT_UI4; ulVal = value;
	}

	prop_variant(uint64_t Value)
	{
		vt = VT_UI8; uhVal.QuadPart = Value;
	}

	prop_variant(const FILETIME& Value)
	{
		vt = VT_FILETIME; filetime = Value;
	}

	prop_variant(int32_t Value)
	{
		vt = VT_I4; lVal = Value;
	}

	prop_variant(unsigned char Value)
	{
		vt = VT_UI1; bVal = Value;
	}

	prop_variant(uint16_t Value)
	{
		vt = VT_I2; iVal = Value;
	}

	prop_variant& operator=(const prop_variant& Value);
	prop_variant& operator=(const PROPVARIANT& Value);
	prop_variant& operator=(BSTR Value);
	prop_variant& operator=(LPCOLESTR Value);
	prop_variant& operator=(bool Value);
	prop_variant& operator=(uint32_t Value);
	prop_variant& operator=(uint64_t Value);
	prop_variant& operator=(const FILETIME& Value);
	prop_variant& operator=(int32_t Value);
	prop_variant& operator=(unsigned char Value);
	prop_variant& operator=(uint16_t Value);

private:
	void ChangeType(VARTYPE Type)
	{
		if (vt != Type)
		{
			InternalClear();
			vt = Type;
		}
	}

	HRESULT Clear();
	HRESULT Copy(const PROPVARIANT* Value);
	HRESULT Attach(PROPVARIANT* Src);
	HRESULT Detach(PROPVARIANT* Dest);

	HRESULT InternalClear();
	void InternalCopy(const PROPVARIANT* Value);
};

