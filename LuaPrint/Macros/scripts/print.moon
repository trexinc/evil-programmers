font={name:'Courier New',size:10}
margin={x:10,y:10}
F=far.Flags
ffi=require'ffi'
C=ffi.C
winspool=ffi.load'winspool.drv'
safe_cdef=(def)->pcall ffi.cdef,def
safe_cdef[[
  typedef struct _PRINTER_INFO_1W {
    DWORD Flags;
    LPWSTR pDescription;
    LPWSTR pName;
    LPWSTR pComment;
  } PRINTER_INFO_1W,*PPRINTER_INFO_1W,*LPPRINTER_INFO_1W;
]]
safe_cdef[[
  typedef struct _DOCINFOW {
    int cbSize;
    LPCWSTR lpszDocName;
    LPCWSTR lpszOutput;
    LPCWSTR lpszDatatype;
    DWORD fwType;
  } DOCINFOW,*LPDOCINFOW;
]]
safe_cdef[[
typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT,*PPOINT,*NPPOINT,*LPPOINT;
]]
ffi.cdef[[
WINBOOL EnumPrintersW(DWORD Flags,LPWSTR Name,DWORD Level,LPBYTE pPrinterEnum,DWORD cbBuf,LPDWORD pcbNeeded,LPDWORD pcReturned);
WINBOOL GetDefaultPrinterW(LPWSTR pszBuffer,LPDWORD pcchBuffer);
void* CreateDCW(LPCWSTR pwszDriver,LPCWSTR pwszDevice,LPCWSTR pszPort,void* pdm);
WINBOOL DeleteDC(void* hdc);
int GetDeviceCaps(void* hdc,int index);
int SetMapMode(void* hdc,int iMode);
WINBOOL SetViewportOrgEx(void* hdc,int x,int y,LPPOINT lppt);
int StartDocW(void* hdc,const DOCINFOW *lpdi);
int EndDoc(void* hdc);
int StartPage(void* hdc);
int EndPage(void* hdc);
void* CreateFontW(int cHeight,int cWidth,int cEscapement,int cOrientation,int cWeight,DWORD bItalic,DWORD bUnderline,DWORD bStrikeOut,DWORD iCharSet,DWORD iOutPrecision,DWORD iClipPrecision,DWORD iQuality,DWORD iPitchAndFamily,LPCWSTR pszFaceName);
WINBOOL DeleteObject(void* ho);
void* SelectObject(void* hdc,void* h);
WINBOOL TextOutW(void* hdc,int x,int y,LPCWSTR lpString,int c);
int lstrlenW(wchar_t* lpString);
]]
PRINTER_ENUM_LOCAL=0x00000002
PRINTER_ENUM_CONNECTIONS=0x00000004
MM_LOMETRIC=2
VERTSIZE=6
VERTRES=10
FW_DONTCARE=0
DEFAULT_CHARSET=1
OUT_DEFAULT_PRECIS=0
CLIP_DEFAULT_PRECIS=0
DEFAULT_QUALITY=0
FIXED_PITCH=1
FF_MODERN=48
sizemul=10
font.size=math.floor font.size*25.4*sizemul/72
margin.x*=sizemul
margin.y*=sizemul

ToWChar=(str)->
  str=win.Utf8ToUtf16 str
  result=ffi.new 'wchar_t[?]',#str/2+1
  ffi.copy result,str
  result

ToUtf8=(str)->
  win.Utf16ToUtf8 ffi.string str,2*C.lstrlenW str

DefaultPrinter=->
  size=ffi.new'DWORD[1]'
  winspool.GetDefaultPrinterW ffi.NULL,size
  name=ffi.new 'wchar_t[?]',size[0]
  winspool.GetDefaultPrinterW name,size
  ToUtf8 name

Print=->
  ei=editor.GetInfo!
  if ei
    current,total=1,ei.TotalLines
    selection=editor.GetSelection!
    if selection
      current,total=selection.StartLine,selection.EndLine
    getline=(no)->
      line=editor.GetString F.CURRENT_EDITOR,no,0
      if selection
        line.StringText\sub line.SelStart,line.SelEnd
      else
        line.StringText
    while 0==(getline total)\len! do total-=1
    size=ffi.new'DWORD[1]'
    count=ffi.new'DWORD[1]'
    winspool.EnumPrintersW PRINTER_ENUM_LOCAL+PRINTER_ENUM_CONNECTIONS,ffi.NULL,1,ffi.NULL,0,size,count
    if size[0]>0
      printers_raw=ffi.new 'BYTE[?]',size[0]
      if 0~=winspool.EnumPrintersW PRINTER_ENUM_LOCAL+PRINTER_ENUM_CONNECTIONS,ffi.NULL,1,printers_raw,size[0],size,count
        printers_win=ffi.cast 'PRINTER_INFO_1W*',printers_raw
        printers={}
        defname=DefaultPrinter!
        for ii=0,count[0]-1
          name=ToUtf8 printers_win[ii].pName
          def=name==defname
          table.insert printers,{value:printers_win[ii].pName,text:name,checked:def,selected:def}
        printer=far.Menu{Id:win.Uuid'3a557a89-e862-4e9e-83b2-c01a1bae4a54'},printers
        if printer
          dc=C.CreateDCW ffi.NULL,printer.value,ffi.NULL,ffi.NULL
          C.SetMapMode dc,MM_LOMETRIC
          C.SetViewportOrgEx dc,0,(C.GetDeviceCaps dc,VERTRES),ffi.NULL
          pageheight=sizemul*C.GetDeviceCaps dc,VERTSIZE
          docinfo=ffi.new 'DOCINFOW'
          docinfo.cbSize=ffi.sizeof docinfo
          docinfo.lpszDocName=ToWChar ei.FileName
          C.StartDocW dc,docinfo
          font_win=C.CreateFontW font.size,0,0,0,FW_DONTCARE,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH+FF_MODERN,ToWChar font.name
          linesperpage=math.floor (pageheight-2*margin.y)/font.size
          pages=math.ceil (total-current+1)/linesperpage
          for ii=1,pages
            C.StartPage dc
            oldfont=C.SelectObject dc,font_win
            for jj=1,linesperpage
              line=getline current
              C.TextOutW dc,margin.x,pageheight-margin.y-(jj-1)*font.size,(ToWChar line),line\len!
              current+=1
              break if current>total
            C.SelectObject dc,oldfont
            C.EndPage dc
          C.EndDoc dc
          C.DeleteObject font_win
          C.DeleteDC dc

Macro
  area:'Editor'
  key:'AltF5'
  description:'print text from editor'
  action:Print
