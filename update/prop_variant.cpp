#include "headers.hpp"
#pragma hdrstop

#include "prop_variant.hpp"

prop_variant::prop_variant(const PROPVARIANT& Value)
{
	vt = VT_EMPTY;
	InternalCopy(&Value);
}

prop_variant::prop_variant(const prop_variant& Value)
{
	vt = VT_EMPTY;
	InternalCopy(&Value);
}

prop_variant::prop_variant(BSTR Value)
{
	vt = VT_EMPTY;
	*this = Value;
}

prop_variant::prop_variant(LPCOLESTR Value)
{
	vt = VT_EMPTY;
	*this = Value;
}

prop_variant& prop_variant::operator=(const prop_variant& Value)
{
	InternalCopy(&Value);
	return *this;
}
prop_variant& prop_variant::operator=(const PROPVARIANT& Value)
{
	InternalCopy(&Value);
	return *this;
}

prop_variant& prop_variant::operator=(BSTR Value)
{
	*this = static_cast<LPCOLESTR>(Value);
	return *this;
}

prop_variant& prop_variant::operator=(LPCOLESTR Value)
{
	InternalClear();
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(Value);
	if (!bstrVal && Value)
	{
		vt = VT_ERROR;
		scode = E_OUTOFMEMORY;
	}
	return *this;
}


prop_variant& prop_variant::operator=(bool Value)
{
	ChangeType(VT_BOOL);
	boolVal = Value? VARIANT_TRUE : VARIANT_FALSE;
	return *this;
}

prop_variant& prop_variant::operator=(uint32_t Value)
{
	ChangeType(VT_UI4);
	ulVal = Value;
	return *this;
}

prop_variant& prop_variant::operator=(uint64_t Value)
{
	ChangeType(VT_UI8);
	uhVal.QuadPart = Value;
	return *this;
}

prop_variant& prop_variant::operator=(const FILETIME& Value)
{
	ChangeType(VT_FILETIME);
	filetime = Value;
	return *this;
}

prop_variant& prop_variant::operator=(int32_t Value)
{
	ChangeType(VT_I4);
	lVal = Value;
	return *this;
}

prop_variant& prop_variant::operator=(unsigned char Value)
{
	ChangeType(VT_UI1);
	bVal = Value;
	return *this;
}

prop_variant& prop_variant::operator=(uint16_t Value)
{
	ChangeType(VT_I2);
	iVal = Value;
	return *this;
}

static HRESULT MyPropVariantClear(PROPVARIANT* Variant)
{
	switch (Variant->vt)
	{
	case VT_UI1:
	case VT_I1:
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
	case VT_FILETIME:
	case VT_UI8:
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		Variant->vt = VT_EMPTY;
		return S_OK;

	default:
		return ::VariantClear(reinterpret_cast<VARIANTARG*>(Variant));
	}
}

HRESULT prop_variant::Clear()
{
	return MyPropVariantClear(this);
}

HRESULT prop_variant::Copy(const PROPVARIANT* Value)
{
	::VariantClear(reinterpret_cast<tagVARIANT*>(this));
	switch (Value->vt)
	{
	case VT_UI1:
	case VT_I1:
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
	case VT_FILETIME:
	case VT_UI8:
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		memmove(static_cast<PROPVARIANT*>(this), Value, sizeof(PROPVARIANT));
		return S_OK;

	default:
		return ::VariantCopy(reinterpret_cast<tagVARIANT *>(this), (tagVARIANT *)Value);
	}
}

HRESULT prop_variant::Attach(PROPVARIANT* Src)
{
	const auto Hr = Clear();
	if (FAILED(Hr))
		return Hr;
	memcpy(this, Src, sizeof(PROPVARIANT));
	Src->vt = VT_EMPTY;
	return S_OK;
}

HRESULT prop_variant::Detach(PROPVARIANT* Dest)
{
	const auto Hr = MyPropVariantClear(Dest);
	if (FAILED(Hr))
		return Hr;
	memcpy(Dest, this, sizeof(PROPVARIANT));
	vt = VT_EMPTY;
	return S_OK;
}

HRESULT prop_variant::InternalClear()
{
	const auto Hr = Clear();
	if (FAILED(Hr))
	{
		vt = VT_ERROR;
		scode = Hr;
	}
	return Hr;
}

void prop_variant::InternalCopy(const PROPVARIANT* Value)
{
	const auto Hr = Copy(Value);
	if (FAILED(Hr))
	{
		vt = VT_ERROR;
		scode = Hr;
	}
}
