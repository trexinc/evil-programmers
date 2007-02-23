#ifndef ___FarDlgHlp_H___
#define ___FarDlgHlp_H___

#if _MSC_VER >= 1000
#pragma once
#endif

// Far console window loss focus
#define DM_DEACTIVATE    DM_USER-1

struct _FARDIALOGHELPER
{
private:
	_FARDIALOGHELPER() {}
	~_FARDIALOGHELPER() {}

public:
	long sendMessage( int msg, int param1, long param2 );
	
	long defDlgProc( int msg, int param1, long param2 );
	
	long close( int id ) 
	{
		return sendMessage( DM_CLOSE, id, 0 );
	}
	
	void enable( int id, bool enable = true )
	{
		sendMessage( DM_ENABLE, id, enable ? TRUE : FALSE );
	}
	
	void disable( int id, bool disable = false )
	{
		enable( id, disable );
	}
	
	bool isEnabled( int id )
	{
		return sendMessage( DM_ENABLE, id, -1 ) != FALSE;
	}
	
	bool isDisabled( int id )
	{
		return !isEnabled( id );
	}
	
	long enableRedraw()
	{
		return sendMessage( DM_ENABLEREDRAW, TRUE, 0 );
	}
	
	long disableRedraw()
	{
		return sendMessage( DM_ENABLEREDRAW, FALSE, 0 );
	}
	
	long getRedrawState()
	{
		return sendMessage( DM_ENABLEREDRAW, FALSE, -1 );
	}
	
	long getData()
	{
		return sendMessage( DM_GETDLGDATA, 0, 0 );
	}
	
	bool getItem( int id, FarDialogItem * item )
	{
		return sendMessage( DM_GETDLGITEM, id, (long)item ) != FALSE;
	}
	
	bool getRect( SMALL_RECT * r )
	{
		return sendMessage( DM_GETDLGRECT, 0, (long)r ) != FALSE;
	}
	
	long getText( int id, FarDialogItemData * data )
	{
		return sendMessage( DM_GETTEXT, id, (long)data );
	}
	
	long getTextLength( int id )
	{
		return sendMessage( DM_GETTEXTLENGTH, id, 0 );
	}
	
	void postKey( LPDWORD keys, int count )
	{
		sendMessage( DM_KEY, count, (long)keys );
	}
	
	COORD move( COORD c, bool absolute )
	{
		long result = sendMessage( DM_MOVEDIALOG, absolute ? TRUE : FALSE, (long)&c );
		return *(PCOORD)&result;
	}
	
	long setData( long data )
	{
		return sendMessage( DM_SETDLGDATA, 0, data );
	}
	
	bool setItem( int id, FarDialogItem * item )
	{
		return sendMessage( DM_SETDLGITEM, id, (long)item ) != FALSE;
	}
	
	bool setFocus( int id )
	{
		return sendMessage( DM_SETFOCUS, id, 0 ) != FALSE;
	}
	
	void redraw()
	{
		sendMessage( DM_REDRAW, 0, 0 );
	}
	
	long setText( int id, FarDialogItemData * data )
	{
		return sendMessage( DM_SETTEXT, id, (long)data );
	}
	
	long setMaxTextLength( int id, long length )
	{
		return sendMessage( DM_SETMAXTEXTLENGTH, id, length );
	}
	
	long setTextLength( int id, int length )
	{
		return setMaxTextLength( id, length );
	}
	
	void show( bool show = true )
	{
		sendMessage( DM_SHOWDIALOG, show ? TRUE : FALSE, 0 );
	}
	
	void hide( bool hide )
	{
		show( hide );
	}
	
	int getFocus()
	{
		return sendMessage( DM_GETFOCUS, 0, 0 );
	}

/*
  DM_GETCURSORPOS,
  DM_SETCURSORPOS,
  DM_GETTEXTPTR,
  DM_SETTEXTPTR,
  DM_SHOWITEM,
  DM_ADDHISTORY,

  DM_GETCHECK,
  DM_SETCHECK,
  DM_SET3STATE,

  DM_LISTSORT,
  DM_LISTGETITEM,
  DM_LISTGETCURPOS,
  DM_LISTSETCURPOS,
  DM_LISTDELETE,
  DM_LISTADD,
  DM_LISTADDSTR,
  DM_LISTUPDATE,
  DM_LISTINSERT,
  DM_LISTFINDSTRING,
  DM_LISTINFO,
  DM_LISTGETDATA,
  DM_LISTSETDATA,
  DM_LISTSETTITLES,
  DM_LISTGETTITLES,

  DM_RESIZEDIALOG,
  DM_SETITEMPOSITION,

  DM_GETDROPDOWNOPENED,
  DM_SETDROPDOWNOPENED,

  DM_SETHISTORY,

  DM_GETITEMPOSITION,
  DM_SETMOUSEEVENTNOTIFY,
*/
	bool getEditUnchanged( int id )
	{
		return sendMessage( DM_EDITUNCHANGEDFLAG, id, -1 ) == 1;
	}
	
	void setEditUnchanged( int id, bool unchanged )
	{
		sendMessage( DM_EDITUNCHANGEDFLAG, id, unchanged ? 1 : 0 );
	}
	
	void * getItemData( int id )
	{
		return (void*)sendMessage( DM_GETITEMDATA, id, 0 );
	}
	
	long setItemData( int id, long data )
	{
		return sendMessage( DM_SETITEMDATA, id, data );
	}
	
	long setItemData( int id, void * data )
	{
		return setItemData( id, (long)data );
	}
	/*  
  DM_LISTSET,
  DM_LISTSETMOUSEREACTION,

  DM_GETCURSORSIZE,
  DM_SETCURSORSIZE,

  DM_LISTGETDATASIZE,
*/
};

#endif //!defined(___FarDlgHlp_H___)
