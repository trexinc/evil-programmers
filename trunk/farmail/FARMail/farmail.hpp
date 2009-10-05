/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __FARMAIL_HPP__
#define __FARMAIL_HPP__

#include "plugin.hpp"
#include <winsock.h>
#include "memory.hpp"
#include "language.hpp"
#include "fmp_internal.hpp"
#include "crt.hpp"
#ifdef FARMAIL_SSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#endif

#ifdef FARMAIL_SSL
#define CON_NORMAL 0
#define CON_SSL 1
#endif

#define BUFFER_SIZE 512
#define SD_SEND 0x01

#define PROGRESS_LEN 30

#define uchar unsigned char

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

#define NUM_OF_CUSTOM_COLS 10

#define DllExport
#define UNKNOWN_CHARSET_TABLE -2

enum {
  MULTI_START = 1,
  MULTI_SENDFILE,
  MULTI_SENDTEXT,
  MULTI_END
};

typedef struct CHARSET_TABLE {

  char charset[20];
  char table[128];
  char DecodeTable[256];
  char EncodeTable[256];
  int  num;

} CHARSET_TABLE;

enum {
 TYPE_POP3  = 0,
 TYPE_IMAP4 = 1,
};

enum {
 SAVE_OPEN,
 SAVE_STR,
 SAVE_CLOSE,
 SAVE_KILL };

struct HEADERSTRUCT
{
  char **list;
  size_t count;
};

struct HEADERPTRSTRUCT
{
  struct HEADERSTRUCT *list;
  size_t count;
};

class HeaderList
{
  private:
    struct HEADERSTRUCT name,attach;
    struct HEADERPTRSTRUCT value;
    bool related;

    size_t Count(struct HEADERSTRUCT *ptr);
    size_t CountSub(struct HEADERPTRSTRUCT *ptr);
    const char *GetStr(struct HEADERSTRUCT *ptr, size_t index);
    const char *GetSubStr(struct HEADERPTRSTRUCT *ptr, size_t index, size_t sub_index);
    BOOL SetStr(struct HEADERSTRUCT *ptr, size_t index, const char *str);
    BOOL SetSubStr(struct HEADERPTRSTRUCT *ptr, size_t index, size_t sub_index, const char *str);
    BOOL Insert(struct HEADERSTRUCT *ptr, size_t index, const char *str);
    BOOL InsertSub(struct HEADERPTRSTRUCT *ptr, size_t index, const char *str);
    BOOL Append(struct HEADERSTRUCT *ptr, const char *str);
    BOOL AppendSub(struct HEADERPTRSTRUCT *ptr, const char *str);
    void Delete(struct HEADERSTRUCT *ptr, size_t index);
    void DeleteSub(struct HEADERPTRSTRUCT *ptr, size_t index);
    void Swap(struct HEADERSTRUCT *ptr, size_t index1, size_t index2);
    void SwapSub(struct HEADERPTRSTRUCT *ptr, size_t index1, size_t index2);
    void Clear(struct HEADERSTRUCT *ptr);
    void ClearSub(struct HEADERPTRSTRUCT *ptr);

  public:
    HeaderList();
    const char *GetName(size_t index);
    const char *GetValue(size_t index, size_t sub_index);
    const char *GetAttach(size_t index);
    size_t CountHeader(void);
    size_t CountValues(size_t index);
    size_t CountAttach(void);
    void DecodeName(class FARMail *obj, char *charset, size_t index);
    void DecodeValues(class FARMail *obj, char *charset, size_t index);
    void DecodeAttach(class FARMail *obj, char *charset, size_t index);
    void EncodeName(class FARMail *obj, char *charset, size_t index);
    void EncodeValues(class FARMail *obj, char *charset, size_t index);
    void EncodeAttach(class FARMail *obj, char *charset, size_t index);
    BOOL InsertHeader(size_t index, const char *NewName, const char *NewValue);
    BOOL InsertHeader(size_t index, const char *NewName);
    BOOL InsertValue(size_t index, size_t sub_index, const char* NewValue);
    BOOL InsertAttach(size_t index, const char *NewAttach);
    BOOL AppendHeader(const char *NewName, const char *NewValue);
    BOOL AppendHeader(const char *NewName);
    BOOL AppendValue(size_t index, const char *NewValue);
    BOOL AppendAttach(const char *NewAttach);
    BOOL SetValue(size_t index, size_t sub_index, const char *NewValue);
    BOOL SetValue(const char *Name, const char *NewValue);
    void DeleteHeader(size_t index);
    void DeleteValue(size_t index, size_t sub_index);
    void DeleteAttach(size_t index);
    void ClearAll(void);
    bool GetRelated(void);
    void SetRelated(bool Value=true);
    ~HeaderList();
};

struct MAILSEND
{
  char Sender[512];
  char ReplyTo[512];
  char Recipient[512];
  char Subject[512];
  char CC[512];
  char BCC[512];
  char Mailer[512];
  char Organization[512];
  int  nbit;
  int  bitcontrol;
  int  how;
  int  all;
  int  analize_all;

  char charset[20];
  char encode[20];

  int  multipart;
  char boundary[50];
  long infopos;
  bool infofound;

  class HeaderList Header;

  bool UseCC;
  bool UserSender;
};

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

struct PanelModeOptions {
  char *lpColumnTypes;
  char *lpColumnWidths;
  char *lpStatusColumnTypes;
  char *lpStatusColumnWidths;

  BOOL bFullScreen;
//  DWORD dwFlags;
};

#define PLUGIN_PANEL_COUNT 4

#define PLUGIN_PANEL_MAILBOXES      0
#define PLUGIN_PANEL_IMAP4_FOLDERS  1
#define PLUGIN_PANEL_POP3           2
#define PLUGIN_PANEL_IMAP4          3

typedef struct Options
{
  int AddToDisksMenu;
  int DisksMenuDigit;
  int DebugSession;
  char LOGFILE[MAX_PATH];
  char LOGFILE2[MAX_PATH];
  char LOGFILE3[MAX_PATH];
  char EXT[20];

  int  DisplayZeroSizeMess;

  int  LeadingZeros;

  int LastViewMode[PLUGIN_PANEL_COUNT];
  PanelModeOptions Modes[PLUGIN_PANEL_COUNT][10];

  char DefCharset[20];
  char DefOutCharset[20];

  int  Timeout;
  int  QuickLines;
  int  QuickBytes;
  int  UseCC;

  char PathToOutbox[MAX_PATH];
  int  UseOutbox;
  char PathToInbox[MAX_PATH];
  int  UseInbox;
  char SaveDir[MAX_PATH];
  int  SaveMessages;
  int  UseHeader;
  int  Resume;
  int  FileDate;
  int  DisableTOP;

  char Format[100];

  int  UseNameF;
  char IMAP_Inbox[512];

  int Unique;
  int AddToPluginMenu;
  int ConfirmBinary;

  int SaveMessageID;
  char MessageIDTemplate[512];

  int UseAttrHighlighting;

  int NewAttributes;
  int ReadAttributes;
  int MarkedAttributes;
  int DeletedAttributes;

} Options;



typedef struct POPSERVER {

   char Name[80];
   char UserName[80];
   char EMail[80];
   char Organization[80];

   char SMTPUrl[80];
   int  SMTPPort;
#ifdef FARMAIL_SSL
   int  UseSMTPS;
#endif

   char Url[80];
   int  Port;
#ifdef FARMAIL_SSL
   int  UsePOP3S;
#endif
   char User[80];
   char Pass[80];
   int  timeout;
   int  Default;
   int  uidl;

   char _User[80];
   char _Pass[80];

   int  Type;

   char SMTP_User[80];
   char SMTP_Pass[80];
   int  UseAuth;

   int TopValue;

   char MailboxPath[200];
} POPSERVER;

class MessageCache
{
 private:

    int _UseCache;
    PluginPanelItem *pPanelItem;
    int pItemsNumber;
    char MailboxPath[200];
    int InternalClearCachedData(void);
 public:
    MessageCache();
    ~MessageCache();
    int UseCache(void) { return _UseCache; }
    int LoadCachedData( PluginPanelItem *pPanelItem, int pItemsNumber, const char *MailboxPath );
    int UseCachedData( PluginPanelItem **pPanelItem, int *pItemsNumber );
    int ClearCachedData(void);
    bool MarkMessage(const char *uidl, DWORD state);
    bool MarkMessage(int i, DWORD state);
    bool ClearState(DWORD state);
};


class Bar
{
 private:
         long Total;
         int Len;
         HANDLE hs;
         const char *Mes[4];
         char bar[200];
         char head[200];
         const char *Head;
 public:
         Bar( long total , const char * s , int len );
        ~Bar();
         int UseBar( long current );

};



class ShortMessage
{
 private:
         HANDLE hs;
         const char *mes[2];
 public:
         ShortMessage( int num );
        ~ShortMessage();

};


class FMSocket
{
  public:
    FMSocket();
    ~FMSocket();
    int Receive(char *buf, int size ,long timeout);
    int Send(const char *buf, int size , long timeout);
    bool ShutdownConnection();
    u_long LookupAddress(char* pcHost);
#ifdef FARMAIL_SSL
    int Connect(char *host, int port ,long timeout, int type);
#else
    int Connect(char *host, int port, long timeout);
#endif
    int StopSocket();
    int RecreateSocket();
  private:
    SOCKET s;
#ifdef FARMAIL_SSL
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *sbio;
#endif
    int errstate;
    int _StopSocket;
};




class MailClient
{
 public:
   MailClient(BOOL _log, char *file, int interval);
   ~MailClient();
#ifdef FARMAIL_SSL
   BOOL Connect(char *Host, char *User, char *Password, int port, int type);
#else
   BOOL Connect(char *Host, char *User, char *Password, int port);
#endif
   BOOL Delete(int MsgNumber);
   BOOL Disconnect();
   BOOL Noop();
   BOOL Reset();
   BOOL Retrieve(int MsgNumber,int OpMode);
   BOOL Top(int MsgNumber, int);
   BOOL Statistics();
   char *GetErrorMessage();
   BOOL List();
   BOOL CorrectList(void);
   BOOL Uidl(void);

   int NumberMail;
   int TotalSize;
   int DownloadedSize;
   const char *GetMsg();
   int    connected;
   int  *MessageLens;
   int  *MessageNums;
   char **MessageUidls;
   int    ResponseBufferLen;
   BOOL   FastDownload;
   BOOL   FastDelete;
   char ServerName[80];
   FMSocket PopServer;

   // Polling thread parameters
   HANDLE bkThread;
   int    EndThread;
   int    Interval;

 private:
   BOOL log;
   char *ResponseBuffer;
   char  ErrMessage[BUFFER_SIZE];
   HANDLE fplog;
   char _Name[100];
   long _Size;

   BOOL GetResponseBuffer( char *InitBuf, int initsize , const char * endstr , char *pname , long tsize, long isize=0 );
   BOOL CheckResponse(int ResponseType);
   BOOL AddMessage( int n, int num, int len );
   int AddLog( const char *s );
   BOOL IsError( char *buf );

   HANDLE   hTransferSemaphore;
   int GetMsgSize( int num );
};

class SMTP
{
 public:
   SMTP( BOOL _log , char *file );
   ~SMTP();

#ifdef FARMAIL_SSL
   BOOL Connect(char *Host, int port, int type);
   BOOL Connect(char *Host, int port, char *user, char *pwd, int type);
#else
   BOOL Connect(char *Host, int port);
   BOOL Connect(char *Host, int port, char *user, char *pwd);
#endif
   BOOL Disconnect();
   BOOL Mail(char *from);
   BOOL Receipt(char *rcp);
   BOOL Data();
   BOOL DataLine(const char *line, int check);
   char *GetErrorMessage();
   int    connected;
   FMSocket wsocket;
   BOOL AuthLogin(char *user, char *pwd);
   BOOL AuthPlain(char *user, char *pwd);
   char message_id[1000];

 private:
   BOOL log;
   char   ErrMessage[BUFFER_SIZE];
   HANDLE fplog;

   const char *GetSMTPError(int code);
   BOOL CheckResponse(int ResponseType);
   int AddLog(const char *s);
   BOOL IsError(char *buf, int response);
   int SetAuthAlgorithms( char *buf);
   char Auth[1001];
};


class IMAP
{
 public:
   IMAP( BOOL _log , char *file , int interval );
   ~IMAP();
   void IncreaseTag( void );
   int SendCommand( const char *str );
   int ReceiveResponse( int , long _size, long _startsize, const char *_name );
   int ReceiveResponse2( int );
   int ExecCommand( const char * str , long _size, long _startsize, const char *_name );
   int ExecCommand2( const char * str );

#ifdef FARMAIL_SSL
   int Connect(char *Host, int port, int type);
#else
   int Connect(char *Host, int port);
#endif
   int Disconnect();
   int Noop();
   int Noop2();
   int Capability();
   int Login( char *user , char *pass );
   int List( const char*, const char*);
   int Select( const char *mailbox );
   int Close( void );
   int Fetch( int num, const char *type , long _size, long _startsize, const char *_name );
   int Status( const char*, const char *type );
   int Delete( int num );
   int DeleteUID( char * num );
   int Expunge( void ) ;
   int Create(char *dir);
   int DeleteBox( char *dir );
   int Rename( char *olddir , char *newdir );
   int Copy( int num , char *dir );

   char  *GetRespString( int num );
   char  *GetRespToken( int num );
   void   UnquotString( void );
   const char *GetMsg();
   char  *GetMsgText( int num , int _size, const char * _name );
   char  *GetMsgTop( int num , int top , const char *_name );
   char  *GetMsgTail( int num , long startpos , long endpos , const char *_name );
   void   FreeMsgText( void );

   char  *GetErrorMessage();
   int    connected;
   int MessageNumber;
   int Partial;

   char     Tag[20];
   char     Field[1024];
   int      Version;
   FMSocket Socket;

   char RespString[BUFFER_SIZE];
   char RespString2[BUFFER_SIZE];
   char *RespStringPtr;
   int  RespStringLen;
   int  ReceivedMessageLen;

   // Polling thread parameters
   HANDLE bkThread;
   int    EndThread;
   int    Interval;

 private:

   int  CheckTag( char *buffer, int len /*, char *tagpos*/ );

   long TagCounter;
   BOOL log;

   char * ResponseBuffer;
   int    ResponseBufferLen;

   char * ResponseBuffer2;
   int    ResponseBufferLen2;
   int    RealBufferLen2;
   int    RealBufferLen;

   char * MessageBuffer;
   long   MessageLen;

   char   ErrMessage[BUFFER_SIZE];
   HANDLE fplog;
   int AddLog( const char *s , int );

   HANDLE   hTransferSemaphore;

};




#define CONNECTION_CHECK        0
#define USER_CHECK              1
#define PASSWORD_CHECK          2
#define QUIT_CHECK              3
#define DELETE_CHECK            4
#define RSET_CHECK              5
#define STAT_CHECK              6
#define NOOP_CHECK              7
#define LIST_CHECK              8
#define RETR_CHECK              9
#define TOP_CHECK              10
#define UIDL_CHECK             11

#define MESSAGE_STATE_NEW       1
#define MESSAGE_STATE_READ      2
#define MESSAGE_STATE_MARKED    4
#define MESSAGE_STATE_DELETED   8

class FARMail
{
  private:

 //   int         m_nPanel; //current panel (0-3)
    char      **m_pColumnTitles;
    POPSERVER  *server;
    int         ServerCount;
    int         Level;
    char        SelectedServer[80];
    char        SelectedMailbox[80];
    MailClient *clnt;
    IMAP       *imap;
    SMTP       *FMsmtp;
    POPSERVER  *current;
    int         MailSendDialog(MAILSEND *parm,char *name,bool multi,bool all,bool analize_all);
    int         AnalizeFile( char *name , MAILSEND *parm );
    int         SaveToFile(HANDLE fp,const char *msg,int UseInbox=FALSE);
    void        SendHeaderLine(const char *headername, const char *content, const char *encode);
    void        SendRecipientsHeader(const char *emptr, const char *headername, const char *encode);
    int         SendRecipients(char *emptr);
    void        SendBody(HANDLE fp,MAILSEND *parm,SMTP *smtp,const char *filename,long tsize);
    int         SelectMailbox( void );
    int         ClearDefault( void );

    void        ReadReset( long );
    int         ReadChar( void );
    int         ReadBlock( char *ptr, int size );

    int         rd_curline;
    int         rd_curpos;
    int         rd_eof;
    struct      EditorGetString rd_egs;
    char        IMAP_Mailbox[512];
    HANDLE      savefp;
    HANDLE      tempfp;
    char        tempfp_name[MAX_PATH];
    char        savefp_name[MAX_PATH];
    MessageCache Cache;
    int         MakeDescription( char *buf, char *format, char *from, char *subj, char *date, int buflen );
    int         CopyMoveIMAP( int move );
    int         CopyMoveMsg( int move , struct PluginPanelItem *item , char *dir );
    void        DecodeAttachList(MAILSEND *parm);
    int         ProcessHeaderDirectives(MAILSEND *parm);

    //uidls
    DWORD       GetUidlState(const char *uidl);
  public:
    FARMail();
    ~FARMail();
    int About();
    int Init();
    int Close();
    int EditMailbox( POPSERVER *server , int isnew );
    int InsertMailbox( POPSERVER *server );
    int UpdateMailbox( POPSERVER *server );
    int DeleteMailbox( char * name );
    int UpdateAndRenameMailbox( POPSERVER *server , char * oldname );
    int FillMailbox( POPSERVER *server, char * name );
    int ReadServers( void );
    int SendFromEditor( void );
    long GetEditorFileSize( void );
    char * GetEditorFileName( void );

    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(const char *Dir,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    int FastStatus(struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
    int FastDownload(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, const char *DestPath, int OpMode);
    int FastExpunge(struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
    int GetFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode);
    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber, int Move,int OpMode);

    int ProcessCommandLine(const char *Cmd);

    int GetUser( char *User, char *Pass );
    int GetDialog( int, const char *, char* , int*);
    int GetDialogEx( int, const char *, char* , int*, int*);
    int ProcessEvent( int Event, void *Param );
    int SendFile( MAILSEND * parm, const char *FileName , long tsize );
    int SendFileWA( MAILSEND * parm, const char *FileName , long tsize );
    int DoAllInOne( void );

    void InitMAILSEND(MAILSEND *parm);
    int AnalizeFileMulti( MAILSEND *parm );
    int BeginSendFileMulti( MAILSEND * parm );
    int SendFileMulti( MAILSEND * parm, const char *FileName , long tsize );
    int EndSendFileMulti( MAILSEND * parm );
    int GetHeaderInfo(HANDLE fp,MAILSEND *parm);
    int ProcessMacro( char *buf, MAILSEND *parm );
    void FinalMacroProcessing(MAILSEND *parm);
    int SaveOutgoingMessage( int how, const char *str );

    char TranscodeChar8( char ch, const char * charset, const char* encode );
    int TranscodeStr8Ext( char *str, int size, const char * charset, const char * encode );
    int EncodeStr8( char * str , const char *encode );
    int DecodeStr8( char * str , const char *charset );
    int DecodeChar8( char *c , const char *charset );
    void DecodeSubj( char *str , char *charset_h );
    int InvWin( const char *name, const char *prompt, const char *def, char *res );

    CHARSET_TABLE *CharsetTable;
};


extern char PluginRootKey[80];
extern char PluginMailBoxKey[100];
extern char PluginCommonKey[100];
extern char PluginHotkeyKey[100];
extern char PluginUidlKey[100];

extern const char FASTDOWNLOADCOPY_PREFIX[];
extern const char FASTDOWNLOADMOVE_PREFIX[];
extern const char FASTEXPUNGE_PREFIX[];

extern const char ADDTODISKSMENU    [];
extern const char DISKSMENUDIGIT    [];
extern const char ADDTOPLUGINMENU   [];
extern const char EXTENSION         [];
extern const char DEBUGSESSION      [];
extern const char NULLSTR           [];
extern const char DISPLAYZEROSIZEMESS[];
extern const char FIXEDZEROS        [];
extern const char COLUMNS           [];
extern const char WIDTHS            [];
extern const char STATUSCOLUMNS     [];
extern const char STATUSWIDTHS      [];
extern const char FULLSIZE          [];
extern const char DEFAULTCHARSET    [];
extern const char DEFAULTOUTCHARSET [];
extern const char MAINCHARSET1      [];
extern const char MAINCHARSET2      [];
extern const char MAINTABLE1        [];
extern const char MAINTABLE2        [];
extern const char CHARSET1          [];
extern const char CHARSET2          [];
extern const char CHARSET3          [];
extern const char TABLE1            [];
extern const char TABLE2            [];
extern const char TABLE3            [];
extern const char USASCII           [];
extern const char TIMEOUT           [];
extern const char QV1               [];
extern const char QV2               [];
extern const char USECC             [];
extern const char CONFIRMBINARY     [];
extern const char INBOX             [];
extern const char SAVEMESSAGES  [];
extern const char SAVEFOLDER    [];
extern const char USEINBOX      [];
extern const char PATHTOINBOX   [];
extern const char USEOUTBOX     [];
extern const char PATHTOOUTBOX  [];
extern const char SAVEMESSAGEID [];
extern const char MESSAGEIDTEMPLATE[];
extern const char USEHEADER     [];
extern const char RESUME        [];
extern const char FILEDATE      [];
extern const char DISABLETOP    [];
extern const char NAMEFORMAT    [];
extern const char USENAMEF      [];
extern const char IMAP_INBOX    [];
extern const char USEATTRHIGHLIGHTING[];
extern const char NEWATTRIBUTES     [];
extern const char READATTRIBUTES    [];
extern const char MARKEDATTRIBUTES  [];
extern const char DELETEDATTRIBUTES [];

extern const char ASTERISK      [];
extern const char D1            [];
extern const char S3            [];
extern const char BACKSLASH     [];
extern const char CRLF          [];
extern const char QUESTIONMARK  [];

extern const char FETCH            [];
extern const char UID              [];
extern const char MESSAGES         [];
extern const char BRACED_MESSAGES  [];
extern const char STATUS           [];
extern const char LIST             [];
extern const char RECENT           [];
extern const char UNSEEN           [];
extern const char RFC822HEADER     [];
extern const char RFC822SIZE       [];
extern const char XRESUMEDATA      [];

extern const char SUBJECT          [];
extern const char FROM             [];
extern const char _DATE            [];
extern const char CONTENTTYPE      [];

extern const char DEFSTARTBLOCK [];
extern const char DEFENDBLOCK   [];
extern const char STARTBLOCK    [];
extern const char ENDBLOCK      [];

extern const char UidlKey[];

extern struct PluginStartupInfo _Info;
extern FARSTANDARDFUNCTIONS FSF;
extern Options Opt;
extern PluginManager *pm;

extern HANDLE UidlMutex;

extern const char *Mon[];
extern const char *Day[];

extern CHARSET_TABLE ** GetCharsetTable( void );

int Random(int x);

HKEY CreateRegKey2(HKEY hRoot,const char *root, const char *Key);
HKEY OpenRegKey2(HKEY hRoot, const char*root, const char *Key);
void SetRegKey2(HKEY hRoot, const char *root, const char *Key,const char *ValueName,const char *ValueData,DWORD type=REG_SZ);
void SetRegKey2(HKEY hRoot,const char *root ,const char *Key,const char *ValueName,DWORD ValueData);
void DeleteRegKey2(HKEY hRoot,const char *root, const char *Key);
int GetRegKey2(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
int GetRegKey2(HKEY hRoot,const char *root, const char *Key,const char *ValueName,int *ValueData,DWORD Default);

char *RegQueryStringValueEx (
    HKEY hKey,
    const char *lpValueName,
    char *lpCurrentValue /* = NULL */
    );

const char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);
int FARMailConfig( void );
void EncodeBase64 ( char * dest, char *source , int num );
void EncodeString( uchar *str );
void DecodeString( uchar *str );

void strcatchr( char *str, char s );
//int EncodeQuotedPrintable( char *source, int lensrc, char *dest );
int DecodeQuotedPrintable( char *source, int lensrc, char *dest );
char * SplitHeaderLine( char *line, char *charset, char *encoding, char *text );
int GetGeaderField(const char *header,char *field,const char *type,int len);
void DecodeBase64 ( char * dest, char *source , int num );
//void DecodeUUE ( char * dest, char *source , int num );
int Confirm( int title );
void FreeMailSend( MAILSEND *parm );
int GetFreeNumber( char *dir );
int SayError( const char *s );

const char *get_token( const char *str, int n , int imap );

void ReadRegistryInterface(void);
void ReadRegistryFiles(void);
void ReadRegistryConnection(void);
void ReadRegistryEncodings(void);
void ReadRegistryUidl(void);
void ReadRegistryOther(void);

void ReadCharsetTable(CHARSET_TABLE *CharsetTable);
int ConstructCharset( CHARSET_TABLE **CharsetTable );
int InitCharset( CHARSET_TABLE **CharsetTable );
int DestructCharset( CHARSET_TABLE **CharsetTable );
int FindCharset( const char *charset , CHARSET_TABLE **CharsetTable );
int ComparePattern( char *str, char *mask );

char *GenerateName(int i, char *str);
char * GetShortFileName( const char *fname );
void InitBoundary( char *bound );

extern int DecodeField(char *str,char *charset_h,CHARSET_TABLE **CharsetTable);
extern int ExtDecodeStr8(char *str,const char *charset,CHARSET_TABLE **CharsetTable);
extern int ExtDecodeChar8(char *c,const char *charset,CHARSET_TABLE **CharsetTable);

extern DWORD MapStateToAttribute(DWORD state);

char *GetDateInSMTP(void);

enum {
 SMTP_CONNECTION_CHECK = 0,
 SMTP_HELLO_CHECK,
 SMTP_MAIL_CHECK,
 SMTP_RCPT_CHECK,
 SMTP_DATA_CHECK,
 SMTP_DATA2_CHECK,
 SMTP_QUIT_CHECK,
 SMTP_EHLO_CHECK,
 SMTP_AUTH_CHECK,
 SMTP_LOGIN_CHECK1,
 SMTP_LOGIN_CHECK2
};

#define Dialog_Move(Index,NewX,NewY) {SMALL_RECT pos; int width; _Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,Index,(long)&pos); width=pos.Right-pos.Left; pos.Left=NewX; pos.Top=NewY; pos.Right=pos.Left+width; _Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,Index,(long)&pos);}
#define Dialog_MoveV(Index,NewY) {SMALL_RECT pos; _Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,Index,(long)&pos); pos.Top=NewY; _Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,Index,(long)&pos);}
#define Dialog_Show(Index) _Info.SendDlgMessage(hDlg,DM_SHOWITEM,Index,1)
#define Dialog_Hide(Index) _Info.SendDlgMessage(hDlg,DM_SHOWITEM,Index,0)
#define Dialog_Enable(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,TRUE)
#define Dialog_Disable(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,FALSE)
#define Dialog_IsEnabled(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,-1)
#define Dialog_Focus(Index) _Info.SendDlgMessage(hDlg,DM_SETFOCUS,Index,0)
#define Dialog_SetText(Index,Text) _Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,Index,(long)Text)
#define Dialog_GetText(Index,Text) _Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,Index,(long)Text)
#define Dialog_Focus(Index) _Info.SendDlgMessage(hDlg,DM_SETFOCUS,Index,0)
#define Dialog_Click(Index) _Info.SendDlgMessage(hDlg,DN_BTNCLICK,Index,0)
#define Dialog_SetState(Index,State) _Info.SendDlgMessage(hDlg,DM_SETCHECK,Index,State?BSTATE_CHECKED:BSTATE_UNCHECKED)
#define Dialog_GetState(Index) _Info.SendDlgMessage(hDlg,DM_GETCHECK,Index,0)
#define Dialog_GetUnchanged(Index) _Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,Index,-1)
#define Dialog_SetUnchanged(Index,State) _Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,Index,State?1:0)
#define Dialog_ShowDialog(Show) _Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,Show,0)
#define Dialog_SetFlag(Index,Flag) {struct FarDialogItem Item; _Info.SendDlgMessage(hDlg,DM_GETDLGITEM,Index,(long)&Item); if (!(Item.Flags&Flag)) {Item.Flags|=Flag; _Info.SendDlgMessage(hDlg,DM_SETDLGITEM,Index,(long)&Item);}}
#define Dialog_UnsetFlag(Index,Flag) {struct FarDialogItem Item; _Info.SendDlgMessage(hDlg,DM_GETDLGITEM,Index,(long)&Item); if (Item.Flags&Flag) {Item.Flags-=Flag; _Info.SendDlgMessage(hDlg,DM_SETDLGITEM,Index,(long)&Item);}}

#endif
