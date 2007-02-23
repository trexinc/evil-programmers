#include "FARPlus.h"
#include "FarDlgHlp.h"

long _FARDIALOGHELPER::sendMessage( int msg, int param1, long param2 )
{
	return Far::m_Info.SendDlgMessage( this, msg, param1, param2 );
}

long _FARDIALOGHELPER::defDlgProc( int msg, int param1, long param2 )
{
	return Far::m_Info.DefDlgProc( this, msg, param1, param2 );
}
