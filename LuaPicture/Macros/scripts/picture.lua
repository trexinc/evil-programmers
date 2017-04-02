-- See http://forum.farmanager.com/viewtopic.php?p=112243#p112243
local function Set(list)
  local set={}
  for _,l in ipairs(list) do set[l]=true end
  return set
end
local Refresh=Set{"CtrlLeft","CtrlRight","CtrlUp","CtrlDown","CtrlClear","CtrlNum4","CtrlNum6","CtrlNum2","CtrlNum8"}
local Mouse1=Set{"MsWheelDown","MsWheelUp"}
local Mouse2=Set{"MsM1Click"}
local Mouse3=Set{"MsLClick"}
local BGColor=function() return 0xff000000 end
local F=far.Flags
local K=far.Colors
local ffi=require("ffi")
local C=ffi.C
local function safe_cdef(def) pcall(ffi.cdef,def) end
safe_cdef([[
typedef enum GpStatus {
  Ok = 0,
  GenericError = 1,
  InvalidParameter = 2,
  OutOfMemory = 3,
  ObjectBusy = 4,
  InsufficientBuffer = 5,
  NotImplemented = 6,
  Win32Error = 7,
  WrongState = 8,
  Aborted = 9,
  FileNotFound = 10,
  ValueOverflow = 11,
  AccessDenied = 12,
  UnknownImageFormat = 13,
  FontFamilyNotFound = 14,
  FontStyleNotFound = 15,
  NotTrueTypeFont = 16,
  UnsupportedGdiplusVersion = 17,
  GdiplusNotInitialized = 18,
  PropertyNotFound = 19,
  PropertyNotSupported = 20,
  ProfileNotFound = 21
} GpStatus;
]])
safe_cdef([[
typedef struct GdiplusStartupInput {
  uint32_t GdiplusVersion;
  void* DebugEventCallback;
  int SuppressBackgroundThread;
  int SuppressExternalCodecs;
} GdiplusStartupInput;
]])
safe_cdef([[
typedef struct GdiplusStartupOutput {
  void* NotificationHook;
  void* NotificationUnhook;
} GdiplusStartupOutput;
]])
safe_cdef([[
typedef struct {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;
]])
safe_cdef([[
typedef struct PropertyItem {
  unsigned long id;
  unsigned long length;
  unsigned short type;
  void* value;
} PropertyItem;
]])
ffi.cdef[[
GpStatus GdiplusStartup(void**,const GdiplusStartupInput*,GdiplusStartupOutput*);
void GdiplusShutdown(void*);
GpStatus GdipLoadImageFromFile(const wchar_t*,void** GpImage);
GpStatus GdipCreateFromHDC(void* HDC,void** GpGraphics);
GpStatus GdipDrawImageRectI(void* GpGraphics,void* GpImage,int,int,int,int);
GpStatus GdipGetImageWidth(void* GpImage,unsigned int*);
GpStatus GdipGetImageHeight(void* GpImage,unsigned int*);
GpStatus GdipDeleteGraphics(void* GpGraphics);
GpStatus GdipDisposeImage(void* GpImage);
GpStatus GdipImageGetFrameDimensionsCount(void* GpImage,unsigned int*);
GpStatus GdipImageGetFrameDimensionsList(void* GpImage,GUID*,unsigned int);
GpStatus GdipImageGetFrameCount(void* GpImage,const GUID*,unsigned int*);
GpStatus GdipImageSelectActiveFrame(void* GpImage,const GUID*,unsigned int);
GpStatus GdipFillRectangleI(void* GpGraphics,void* GpBrush,int,int,int,int);
GpStatus GdipCreateSolidFill(unsigned long,void** GpSolidFill);
GpStatus GdipDeleteBrush(void* GpBrush);
GpStatus GdipCreateBitmapFromScan0(int,int,int,int,unsigned char*,void** GpBitmap);
GpStatus GdipGetImageGraphicsContext(void* GpImage,void** GpGraphics);
GpStatus GdipGetPropertyItemSize(void* GpImage,unsigned long,unsigned int*);
GpStatus GdipGetPropertyItem(void* GpImage,unsigned long,unsigned int,PropertyItem*);
]]
safe_cdef([[
typedef struct _COORD {
  short X;
  short Y;
} COORD;
]])
safe_cdef([[
typedef struct _SMALL_RECT {
  short Left;
  short Top;
  short Right;
  short Bottom;
} SMALL_RECT;
]])
safe_cdef([[
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
  COORD dwSize;
  COORD dwCursorPosition;
  unsigned short wAttributes;
  SMALL_RECT srWindow;
  COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;
]])
safe_cdef([[
typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
  unsigned long cbSize;
  COORD dwSize;
  COORD dwCursorPosition;
  unsigned short wAttributes;
  SMALL_RECT srWindow;
  COORD dwMaximumWindowSize;
  unsigned short wPopupAttributes;
  int bFullscreenSupported;
  unsigned long ColorTable[16];
} CONSOLE_SCREEN_BUFFER_INFOEX;
]])
safe_cdef([[
typedef struct _CONSOLE_FONT_INFO {
  unsigned long nFont;
  COORD dwFontSize;
} CONSOLE_FONT_INFO;
]])
ffi.cdef[[
void* GetDC(void* HWND);
int ReleaseDC(void* HWND,void* HDC);
int32_t GetConsoleScreenBufferInfo(void* hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFO* lpConsoleScreenBufferInfo);
int32_t GetConsoleScreenBufferInfoEx(void* hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFOEX* lpConsoleScreenBufferInfoEx);
int32_t GetCurrentConsoleFont(void* hConsoleOutput,int32_t bMaximumWindow,CONSOLE_FONT_INFO* lpConsoleCurrentFont);
COORD GetConsoleFontSize(void* hConsoleOutput,unsigned long nFont);
void* GetStdHandle(uint32_t nStdHandle);
]]
local gdiplus=ffi.load("gdiplus")
local handle=ffi.new("void*[1]")
local startup=ffi.new("GdiplusStartupInput")
startup.GdiplusVersion=1
gdiplus.GdiplusStartup(handle,startup,ffi.NULL)
local Symbol=0

safe_cdef([[
typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation = 0,
  SystemProcessorInformation = 1,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemProcessInformation = 5,
  SystemProcessorPerformanceInformation = 8,
  SystemHandleInformation = 16,
  SystemPagefileInformation = 18,
  SystemInterruptInformation = 23,
  SystemExceptionInformation = 33,
  SystemRegistryQuotaInformation = 37,
  SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;
]])
safe_cdef([[
typedef enum _THREAD_STATE {
  StateInitialized = 0,
  StateReady, StateRunning, StateStandby, StateTerminated,
  StateWait, StateTransition,
  StateUnknown
} THREAD_STATE;
]])
safe_cdef([[
typedef enum _KWAIT_REASON {
  Executive = 0,
  FreePage, PageIn, PoolAllocation, DelayExecution,
  Suspended, UserRequest, WrExecutive, WrFreePage, WrPageIn,
  WrPoolAllocation, WrDelayExecution, WrSuspended,
  WrUserRequest, WrEventPair, WrQueue, WrLpcReceive,
  WrLpcReply, WrVirtualMemory, WrPageOut, WrRendezvous,
  Spare2, Spare3, Spare4, Spare5, Spare6, WrKernel,
  MaximumWaitReason
} KWAIT_REASON;
]])
safe_cdef([[
typedef struct _LARGE_INTEGER {
  long long QuadPart;
} LARGE_INTEGER;
]])
safe_cdef([[
typedef struct _UNICODE_STRING {
  unsigned short Length;
  unsigned short MaximumLength;
  wchar* Buffer;
} UNICODE_STRING;
]])
safe_cdef([[
typedef struct _VM_COUNTERS {
  size_t PeakVirtualSize;
  size_t VirtualSize;
  unsigned long PageFaultCount;
  size_t PeakWorkingSetSize;
  size_t WorkingSetSize;
  size_t QuotaPeakPagedPoolUsage;
  size_t QuotaPagedPoolUsage;
  size_t QuotaPeakNonPagedPoolUsage;
  size_t QuotaNonPagedPoolUsage;
  size_t PagefileUsage;
  size_t PeakPagefileUsage;
} VM_COUNTERS;
]])
safe_cdef([[
typedef struct _IO_COUNTERS {
  unsigned long long ReadOperationCount;
  unsigned long long WriteOperationCount;
  unsigned long long OtherOperationCount;
  unsigned long long ReadTransferCount;
  unsigned long long WriteTransferCount;
  unsigned long long OtherTransferCount;
} IO_COUNTERS;
]])
safe_cdef([[
typedef struct _SYSTEM_PROCESS_INFORMATION {
  unsigned long NextEntryOffset;
  unsigned long NumberOfThreads;
  LARGE_INTEGER Reserved[3];
  LARGE_INTEGER CreateTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;
  UNICODE_STRING ImageName;
  long BasePriority;
  uintptr_t UniqueProcessId;
  uintptr_t InheritedFromUniqueProcessId;
  unsigned long HandleCount;
  unsigned long SessionId;
  unsigned long PageDirectoryBase;
  VM_COUNTERS VirtualMemoryCounters;
  size_t PrivatePageCount;
  IO_COUNTERS IoCounters;
} SYSTEM_PROCESS_INFORMATION;
]])
safe_cdef([[
typedef struct _CLIENT_ID {
  uintptr_t UniqueProcess;
  uintptr_t UniqueThread;
} CLIENT_ID;
]])
safe_cdef([[
typedef struct _SYSTEM_THREADS
{
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER CreateTime;
  unsigned long WaitTime;
  void* StartAddress;
  CLIENT_ID ClientId;
  long Priority;
  long BasePriority;
  unsigned long ContextSwitchCount;
  THREAD_STATE State;
  KWAIT_REASON WaitReason;
} SYSTEM_THREADS, *PSYSTEM_THREADS;
]])
ffi.cdef[[
unsigned long ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS,void*,unsigned long,unsigned long*);
unsigned long GetCurrentProcessId(void);
]]
local ntdll=ffi.load("ntdll")
local pid=C.GetCurrentProcessId()

local function GetProcessesAndThreads()
  local size=512*1024
  while true do
    local buffer=ffi.new("char[?]",size)
    local status=ntdll.ZwQuerySystemInformation(C.SystemProcessInformation,buffer,size,ffi.NULL)
    if 0xC0000004==status then
      size=size*2
    elseif 0==status then
      return buffer
    else
      break
    end
  end
end

local function IsWorking()
  local ptr=GetProcessesAndThreads()
  if ptr then
    local ptr1=ptr
    while true do
      local process=ffi.cast("SYSTEM_PROCESS_INFORMATION*",ptr1)
      if pid==process.InheritedFromUniqueProcessId and 'conhost.exe'==win.Utf16ToUtf8(ffi.string(process.ImageName.Buffer,process.ImageName.Length)):lower() then
        local thread=ffi.cast("SYSTEM_THREADS*",ptr1+ffi.sizeof"SYSTEM_PROCESS_INFORMATION")
        for ii=1,process.NumberOfThreads do
          if thread[ii-1].State~=C.StateWait or thread[ii-1].WaitReason==C.DelayExecution then return true end
        end
        break
      end
      if process.NextEntryOffset==0 then break end
      ptr1=ptr1+process.NextEntryOffset
    end
  end
end

local function ToWChar(str)
  str=win.Utf8ToUtf16(str)
  local result=ffi.new("wchar_t[?]",#str/2+1)
  ffi.copy(result,str)
  return result
end

local function LongPath(path)
  local type=path:match([[^\\(.?.?)]])
  return type and (([[?\]]==type or [[.\]]==type) and path or [[\\?\UNC]]..path:sub(2)) or [[\\?\]]..path
end

local function BGR2RGB(color)
  return bit64.bor(bit64.band(bit64.rshift(color,16),0xff),bit64.band(color,0xff00ff00),bit64.band(bit64.lshift(color,16),0xff0000))
end

local function PointInRect(point,rect)
  return point.MousePositionX>=rect.left and point.MousePositionX<=rect.right and point.MousePositionY>=rect.top and point.MousePositionY<=rect.bottom
end

local function getDNG(filename)
  local function intBE(b)
    local result=0
    for _,v in ipairs(b) do
      if not v then break end
      result=result*256+v
    end
    return result
  end
  local result,thumbs,int,fix=false,{},function(b) return (b[1] or 0)+(b[2] or 0)*256+(b[3] or 0)*65536+(b[4] or 0)*16777216 end,function(a) return a end
  local function get(file,bytes) return int({string.byte(file:read(bytes),1,bytes)}) end
  local function get2(file) return get(file,2) end
  local function get4(file) return get(file,4) end
  local function parse_ifd(file,base,process)
    local entries,index=get2(file),#thumbs+1
    thumbs[index]={size=0,compression=6}
    for _=1,entries do
      local d={f=file,process=process,value=thumbs[index],base=base,tag=get2(file),type=get2(file),count=get4(file),offset=get4(file)}
      process(d)
    end
  end
  local f=io.open(filename,"rb")
  if f then
    local tag=f:read(2)
    if tag=="II" or tag=="MM" then
      if tag=="MM" then
        int=intBE
        fix=function(a) return a/65536 end
      end
      get2(f)
      while true do
        local offset=get4(f)
        if offset==0 then break end
        f:seek('set',offset)
        parse_ifd(f,0,function(d)
          if d.tag==46 then d.value.data,d.value.size=(d.base+d.offset),d.count
          elseif d.tag==259 then d.value.compression=fix(d.offset)
          elseif (d.tag==273 or d.tag==513) and d.offset~=0xffffffff then d.value.data=d.base+d.offset
          elseif d.tag==277 then d.value.samples=fix(d.offset)
          elseif (d.tag==279 or d.tag==514) and d.offset>0 then d.value.size=d.offset
          elseif d.tag==330 then
            local size,pos=4*d.count,d.f:seek()
            for ii=1,d.count do
              d.f:seek('set',d.base+d.offset+(ii-1)*4)
              if size>4 then d.f:seek('set',get4(d.f)) end
              parse_ifd(d.f,d.base,d.process)
            end
            d.f:seek('set',pos)
          elseif d.tag==34665 then
            local pos=d.f:seek()
            d.f:seek('set',d.base+d.offset)
            parse_ifd(d.f,d.base,function(d)
              if d.tag==37500 then
                local pos=d.f:seek()
                d.f:seek('set',d.base+d.offset)
                local new_base=d.f:seek()
                local maker=d.f:read(10)
                if maker=="OLYMPUS\0II" then
                  get2(d.f)
                  parse_ifd(d.f,new_base,function(d)
                    if d.tag==8224 then
                      local pos=d.f:seek()
                      d.f:seek('set',d.base+d.offset)
                      parse_ifd(d.f,d.base,function(d) if d.tag==257 then d.value.data=d.base+d.offset elseif d.tag==258 then d.value.size=d.offset end end)
                      d.f:seek('set',pos)
                    end
                  end)
                end
                d.f:seek('set',pos)
              end
            end)
            d.f:seek('set',pos)
          elseif d.tag==50752 then d.value.raw=true end
        end)
      end
    end
    table.sort(thumbs,function(a,b) return a.size>b.size end)
    for _,v in ipairs(thumbs) do
      if v.data and not v.raw and (v.compression==6 or (v.samples==3 and v.compression==7)) then
        result=far.MkTemp()
        local w=io.open(result,"wb")
        if w then
          f:seek('set',v.data)
          w:write(f:read(v.size))
          w:close()
        end
        break
      end
    end
    f:close()
  end
  return result
end

local function InitBGColor()
  local color=far.AdvControl(F.ACTL_GETCOLOR,K.COL_PANELTEXT)
  local bgcolor
  if bit64.band(color.Flags,F.FCF_BG_4BIT)~=0 then
    local info=ffi.new("CONSOLE_SCREEN_BUFFER_INFOEX")
    info.cbSize=ffi.sizeof(info)
    C.GetConsoleScreenBufferInfoEx(C.GetStdHandle(-11),info)
    bgcolor=bit64.bor(BGR2RGB(info.ColorTable[bit64.band(color.BackgroundColor,0xf)]),0xff000000)
  else
    bgcolor=BGR2RGB(color.BackgroundColor)
  end
  BGColor=function() return bgcolor end
end

local function InitImage(filename)
  local delete=far.ProcessName(F.PN_CMPNAMELIST,"*.dng,*.pef,*.nef,*.cr2,*.sr2,*.arw,*.orf,*.rw2,*.srw",filename,F.PN_SKIPPATH) and getDNG(filename)
  local wnd=far.AdvControl(F.ACTL_GETFARHWND)
  local dc=C.GetDC(wnd)
  local image=ffi.new("void*[1]")
  local status=gdiplus.GdipLoadImageFromFile(ToWChar(LongPath(delete or filename)),image)
  if status==0 then
    local graphics=ffi.new("void*[1]")
    gdiplus.GdipCreateFromHDC(dc,graphics)
    local width=ffi.new("unsigned int[1]")
    gdiplus.GdipGetImageWidth(image[0],width)
    local height=ffi.new("unsigned int[1]")
    gdiplus.GdipGetImageHeight(image[0],height)
    local count=ffi.new("unsigned int[1]")
    gdiplus.GdipImageGetFrameDimensionsCount(image[0],count)
    local dimensionIDs=ffi.new("GUID[?]",count[0])
    gdiplus.GdipImageGetFrameDimensionsList(image[0],dimensionIDs,count[0])
    local frames=ffi.new("unsigned int[1]")
    gdiplus.GdipImageGetFrameCount(image[0],dimensionIDs,frames)
    local delaysize=ffi.new("unsigned int[1]")
    gdiplus.GdipGetPropertyItemSize(image[0],0x5100,delaysize)
    local delay={}
    if delaysize[0]>0 then
      local delayraw_buffer=ffi.new("char[?]",delaysize[0])
      local delayraw=ffi.cast("PropertyItem*",delayraw_buffer)
      gdiplus.GdipGetPropertyItem(image[0],0x5100,delaysize[0],delayraw)
      for ii=1,frames[0] do
        local value=ffi.cast("long*",delayraw.value)[ii-1]
        table.insert(delay,value>0 and value or 1)
      end
    end
    local brush=ffi.new("void*[1]")
    gdiplus.GdipCreateSolidFill(BGColor(),brush)
    local memimage=ffi.new("void*[1]")
    gdiplus.GdipCreateBitmapFromScan0(width[0],height[0],0,0x26200a,ffi.NULL,memimage)
    local memgraphics=ffi.new("void*[1]")
    gdiplus.GdipGetImageGraphicsContext(memimage[0],memgraphics)
    return {wnd=wnd,dc=dc,image=image,graphics=graphics,brush=brush,width=width[0],height=height[0],frames=frames[0],delay=delay,delete=delete,memory={image=memimage,graphics=memgraphics}}
  end
  return false
end

local function DeleteImage(params)
  gdiplus.GdipDisposeImage(params.image.image[0])
  gdiplus.GdipDeleteGraphics(params.image.graphics[0])
  gdiplus.GdipDeleteBrush(params.image.brush[0])
  gdiplus.GdipDeleteGraphics(params.image.memory.graphics[0])
  gdiplus.GdipDisposeImage(params.image.memory.image[0])
  C.ReleaseDC(params.image.wnd,params.image.dc)
  if params.image.delete then win.DeleteFile(params.image.delete) end
end

local function InitArea(params)
  local info=ffi.new("CONSOLE_SCREEN_BUFFER_INFO")
  local font=ffi.new("CONSOLE_FONT_INFO")
  local handle=C.GetStdHandle(-11)
  C.GetConsoleScreenBufferInfo(handle,info)
  if C.GetCurrentConsoleFont(handle,false,font) then font.dwFontSize=C.GetConsoleFontSize(handle,font.nFont) end --xp workaround

  local dx=font.dwFontSize.X
  local dy=font.dwFontSize.Y
  local DCRect={}
  DCRect.left=math.floor(dx*(params.DrawRect.left-info.srWindow.Left))
  DCRect.right=math.floor(dx*(params.DrawRect.right+1-info.srWindow.Left))
  DCRect.top=math.floor(dy*(params.DrawRect.top))
  DCRect.bottom=math.floor(dy*(params.DrawRect.bottom+1))
  return DCRect
end

local function RangingPic(params)
  local asp_dst=(params.DCRect.right-params.DCRect.left)/(params.DCRect.bottom-params.DCRect.top)
  local asp_src=params.image.width/params.image.height

  local dst_w,dst_h

  if asp_dst<asp_src then
    dst_w=math.min(params.DCRect.right-params.DCRect.left,params.image.width)
    dst_h=math.floor(dst_w/asp_src)
  else
    dst_h=math.min(params.DCRect.bottom-params.DCRect.top,params.image.height)
    dst_w=math.floor(asp_src*dst_h)
  end

  local left=params.DCRect.left+math.floor((params.DCRect.right-params.DCRect.left-dst_w)/2)
  local top=params.DCRect.top+math.floor((params.DCRect.bottom-params.DCRect.top-dst_h)/2)

  return {left=left,top=top,right=dst_w,bottom=dst_h}
end

local function UpdateImage(params)
  local width=params.RangedRect.right-params.RangedRect.left
  local height=params.RangedRect.bottom-params.RangedRect.top
  gdiplus.GdipFillRectangleI(params.image.memory.graphics[0],params.image.brush[0],0,0,params.image.width,params.image.height)
  --GdipDrawImageI fails on some images
  --FIXME: какая-то ересь. на некоторых картинках первый вызов GdipDrawImageRectI завершается с ошибкой.
  for _=0,1 do
    if C.Win32Error~=gdiplus.GdipDrawImageRectI(params.image.memory.graphics[0],params.image.image[0],0,0,params.image.width,params.image.height) then break end
  end
  while IsWorking() do end
  gdiplus.GdipDrawImageRectI(params.image.graphics[0],params.image.memory.image[0],params.RangedRect.left,params.RangedRect.top,params.RangedRect.right,params.RangedRect.bottom)
  if params.timer and not params.timer.Closed then
    if params.image.delay then
      params.timer.Interval=params.image.delay[params.image.frame+1]*10
      if not params.timer.Enabled then params.timer.Enabled=true end
    end
    params.image.frame=params.image.frame+1
    if params.image.frame==params.image.frames then
      params.image.frame=0
    end
    gdiplus.GdipImageSelectActiveFrame(params.image.image[0],ffi.cast("GUID*",params.image.guid),params.image.frame);
  end
end

local function ShowImage(xpanel)
  local vinfo,pinfo=viewer.GetInfo(),panel.GetPanelInfo(nil,xpanel)
  if pinfo and vinfo and vinfo.WindowSizeX==(pinfo.PanelRect.right-pinfo.PanelRect.left-1) and pinfo.PanelType==F.PTYPE_QVIEWPANEL then
    local params={CurPanel=bit64.band(pinfo.Flags,F.PFLAGS_FOCUS)~=0,Redraw=false,Key=false,Exit=false}
    params.image=InitImage(viewer.GetFileName())
    if params.image then
      local width,height=pinfo.PanelRect.right-pinfo.PanelRect.left-1,pinfo.PanelRect.bottom-pinfo.PanelRect.top-1
      local buffer=far.CreateUserControl(width,height)
      local function FillBuffer()
        local color=far.AdvControl(F.ACTL_GETCOLOR,K.COL_PANELTEXT)
        local textel={Char=bit64.bor(bit64.band(Symbol,0xf),0x30),Attributes={Flags=(bit64.band(color.Flags,F.FCF_BG_4BIT)==0) and 0 or bit64.bor(F.FCF_FG_4BIT,F.FCF_BG_4BIT),ForegroundColor=color.BackgroundColor,BackgroundColor=color.BackgroundColor}}
        Symbol=Symbol+1
        for ii=1,#buffer do
          buffer[ii]=textel
        end
      end
      FillBuffer()
      local items={
        {"DI_DOUBLEBOX",0,0,width+1,height+1,0,0,0,0,params.image.width.." x "..params.image.height.." * "..params.image.frames},
        {"DI_USERCONTROL",1,1,width,height,buffer,0,0,0,""}
      }
      local function DlgProc(dlg,msg,param1,param2)
        local function CloseTimer()
          if params.timer then params.timer:Close() end
        end
        local function CloseDialog(check,key)
          if not params.Exit then
            key=key and key or far.InputRecordToName(param2)
            if not check or check(key) then
              params.Exit=true
              CloseTimer()
              if Refresh[key] then key=key.." CtrlR" end
              far.MacroPost([[Keys("Esc ]]..key..[[")]])
              return true
            end
          end
        end
        if msg==F.DN_INITDIALOG then
          far.SendDlgMessage (dlg,F.DM_SETMOUSEEVENTNOTIFY,1)
          if params.image.frames>1 then
            local function ShowAnimation()
              if params.timer and not params.timer.Closed then params.timer.Enabled=false end
              UpdateImage(params)
            end
            params.timer=far.Timer(1000000,ShowAnimation)
            params.timer.Enabled=false
            params.image.frame=0
            params.image.guid=win.Uuid("6aedbd6d-3fb5-418a-83a6-7f45229dc872")
          end
        elseif msg==F.DN_CTLCOLORDLGITEM then
          if param1==1 then
            local colors=
            {
              far.AdvControl(F.ACTL_GETCOLOR,params.CurPanel and K.COL_PANELSELECTEDTITLE or K.COL_PANELTITLE),
              far.AdvControl(F.ACTL_GETCOLOR,K.COL_PANELTITLE),
              far.AdvControl(F.ACTL_GETCOLOR,K.COL_PANELTEXT)
            }
            return colors
          end
        elseif msg==F.DN_DRAWDLGITEM then
          params.Redraw=true
        elseif msg==F.DN_DRAWDIALOGDONE then
          if params.Redraw then
            params.Redraw=false
            far.Text(0,0,0,nil)
            UpdateImage(params)
          end
        elseif msg==F.DN_INPUT then
          if param2.EventType==F.FOCUS_EVENT and param2.SetFocus then
            UpdateImage(params)
          elseif param2.EventType==F.MOUSE_EVENT then
            return not CloseDialog(function(key) return Mouse2[key] or Mouse3[key] and not PointInRect(param2,params.PanelRect) end)
          end
        elseif msg==F.DN_CONTROLINPUT then
          if param1==-1 then return true end
          return CloseDialog(function(key) return ((param2.EventType==F.KEY_EVENT or param2.EventType==F.FARMACRO_KEY_EVENT) and param2.KeyDown or param2.EventType==F.MOUSE_EVENT and Mouse1[key]) end)
        elseif msg==F.DN_RESIZECONSOLE then
          CloseDialog(nil,"CtrlR")
        elseif msg==F.DN_DRAGGED then
          return false
        elseif msg==F.DN_CLOSE then
          CloseTimer()
        end
      end
      params.DrawRect={}
      params.DrawRect.left=pinfo.PanelRect.left+1
      params.DrawRect.top=pinfo.PanelRect.top+1
      params.DrawRect.right=pinfo.PanelRect.right-1
      params.DrawRect.bottom=pinfo.PanelRect.bottom-1
      params.DCRect=InitArea(params)
      params.RangedRect=RangingPic(params)
      params.PanelRect=pinfo.PanelRect
      local dialog=far.DialogInit(win.Uuid("BFC62A3A-1ED5-4590-AF86-A582F6237E6F"),pinfo.PanelRect.left,pinfo.PanelRect.top,pinfo.PanelRect.right,pinfo.PanelRect.bottom,nil,items,F.FDLG_NODRAWSHADOW,DlgProc)
      far.DialogRun(dialog)
      far.DialogFree(dialog)
      DeleteImage(params)
    end
  end
end

Event
{
  group="ViewerEvent";
  action=function(id,event,param)
    if event==F.VE_READ then
      local xpanel=0
      if far.AdvControl(F.ACTL_GETWINDOWTYPE).Type==F.WTYPE_PANELS then
        if panel.GetPanelInfo(nil,0).PanelType==F.PTYPE_INFOPANEL then return end
        local type=panel.GetPanelInfo(nil,1).PanelType
        if type==F.PTYPE_INFOPANEL then return end
        if type==F.PTYPE_QVIEWPANEL then xpanel=1 end
        for ii=0,1 do panel.RedrawPanel(nil,ii) end
      end
      pcall(InitBGColor)
      ShowImage(xpanel)
    end
  end
}

Event
{
  group="ExitFAR";
  action=function()
    gdiplus.GdiplusShutdown(handle[0])
  end
}
