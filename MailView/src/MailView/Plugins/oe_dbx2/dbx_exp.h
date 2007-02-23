#ifndef __DBX_EXP_API_H__
#define __DBX_EXP_API_H__
//dbx_exp.h

/*
	OE5 DBX file scanner API (DBXOUT)
	(c)1999 andrey_no
	andrey_no@mail.primorye.ru
*/

/*
for more information refer to readme.txt
or
send mail andrey_no@mail.primorye.ru	(RU means RUSSIAN)
*/

/*
DO_NOT_EDIT_THIS_FILE!!!!!
*/


#ifndef __NO_DBXOUT_API__
#define __DBXOUT_API__
#endif

#ifdef __DBXOUT_API__
#define DBXOUT_API __declspec(dllexport)
#else
#define DBXOUT_API __declspec(dllimport)
#endif //__DBXOUT_API__

#define HDBX	HANDLE

typedef struct
{
	DWORD dwSizeLocked;
	BYTE* pbData;
}DBX_LOCK_DATA,*PDBX_LOCK_DATA;

typedef DBX_LOCK_DATA DBX_HEADER_DATA;//coming soon...

extern "C"
{
//informations
DWORD DBXOUT_API DBXGetID(DBX_HEADER_DATA* pData);
DWORD DBXOUT_API DBXGetParentFolderID(DBX_HEADER_DATA* pData);//!!!!For FolderDatabase only
PFILETIME DBXOUT_API DBXGetTimeRecieved(DBX_HEADER_DATA* pData);
PFILETIME DBXOUT_API DBXGetTimeSend(DBX_HEADER_DATA* pData);
PFILETIME DBXOUT_API DBXGetTimeAccessed(DBX_HEADER_DATA* pData);
LPCTSTR	  DBXOUT_API DBXGetFolderDisplayName(DBX_HEADER_DATA* pData);//!!!!For FolderDatabase only
LPCTSTR	  DBXOUT_API DBXGetFolderFileName(DBX_HEADER_DATA* pData);//!!!!For FolderDatabase only
//Still it is possible to write a heap of functions 
//for deriving those or other datas from DBX_HEADER_DATA.
//But I think that it will be easier to publish a structure of DBX header...

//up-level functions
PDBX_LOCK_DATA DBXOUT_API DBXLockMsgHeaderByPos(HDBX hDBX,DWORD dwMsgPos);
PDBX_LOCK_DATA DBXOUT_API DBXLockMsgHeaderByID(HDBX hDBX,DWORD dwMsgID);
//Always call DBXUnlock after using PDBX_LOCK_DATA returnned by DBXLockXXX
VOID DBXOUT_API DBXUnlock(PDBX_LOCK_DATA pDLD);
//memory model
PVOID	DBXOUT_API	DBXAlloc(DWORD dwSize);//
VOID	DBXOUT_API	DBXFree(void* pPtr);//
//
HDBX	DBXOUT_API	DBXOpen(LPCTSTR pszFullPathName);//
BOOL	DBXOUT_API	DBXIsMessageDatabase(HDBX hDBX);//new
BOOL	DBXOUT_API	DBXIsFolderDatabase(HDBX hDBX);//new
//YOU must close DBX
DWORD	DBXOUT_API	DBXClose(HDBX hDBX);//!!!!
//can be used with OE_MessageList
DWORD	DBXOUT_API	DBXGetFolderID(HDBX hDBX);//
//
DWORD	DBXOUT_API	DBXGetFolderName(HDBX hDBX,LPTSTR pszBuffer,DWORD* pcbBuffer);//
DWORD	DBXOUT_API	DBXGetMsgCount(HDBX hDBX);//
DWORD	DBXOUT_API	DBXGetMsgWatchedCount(HDBX hDBX);//
//position from 0 to (MsgCount-1) returned DBXGetMsgCount
DWORD	DBXOUT_API	DBXGetMsgHeaderByPos(HDBX hDBX,DWORD dwMsgPos,LPTSTR pszBuffer,DWORD* pcbBuffer);
DWORD	DBXOUT_API	DBXGetMsgByPos(HDBX hDBX,DWORD dwMsgPos,LPTSTR pszBuffer,DWORD* pcbBuffer);
//ID is 24-bits value assigned by OE, typical from 0x000002 to 0x0FFFFFF
DWORD	DBXOUT_API	DBXGetMsgHeaderByID(HDBX hDBX,DWORD dwID,LPTSTR pszBuffer,DWORD* pcbBuffer);
DWORD	DBXOUT_API	DBXGetMsgByID(HDBX hDBX,DWORD dwID,LPTSTR pszBuffer,DWORD* pcbBuffer);
//conversion ID <> Position
DWORD	DBXOUT_API	DBXGetMsgID(HDBX hDBX,DWORD dwMsgPos);//
DWORD	DBXOUT_API	DBXGetMsgPos(HDBX hDBX,DWORD dwMsgID);//
//next functions save msg in current directory with it's own names
DWORD	DBXOUT_API	DBXSaveMsgByID(HDBX hDBX,DWORD dwMsgID);
DWORD	DBXOUT_API	DBXSaveMsgByPos(HDBX hDBX,DWORD dwMsgPos);
//next functions use pszFileName as directory path if no extention added
//etc. C:\\winnt		- msg will be saved in directory 'C:\\winnt' with it's own name
//	   C:\\winnt.txt	- msg will be saved in directory 'C:\\' with name 'winnt.txt'
DWORD	DBXOUT_API	DBXSaveAsMsgByID(HDBX hDBX,DWORD dwMsgID,LPCTSTR pszFileName);
DWORD	DBXOUT_API	DBXSaveAsMsgByPos(HDBX hDBX,DWORD dwMsgPos,LPCTSTR pszFileName);
//end specific

};


//error codes returned
#define DBXERROR		0xFFFFFFFF	//common error code

#endif //__DBX_EXP_API_H__