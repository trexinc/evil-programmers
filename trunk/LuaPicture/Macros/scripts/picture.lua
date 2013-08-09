local Refresh={"CtrlLeft","CtrlRight","CtrlUp","CtrlDown","CtrlClear","CtrlNum4","CtrlNum6","CtrlNum2","CtrlNum8"}
local Delay=100
local F=far.Flags
local K=far.Colors
local ffi=require("ffi")
local C=ffi.C
local function safe_cdef(name,def)
if not pcall(ffi.sizeof,name) then ffi.cdef(def) end
end
safe_cdef("GpStatus",[[
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
safe_cdef("GdiplusStartupInput",[[
typedef struct GdiplusStartupInput {
  uint32_t GdiplusVersion;
  void* DebugEventCallback;
  int SuppressBackgroundThread;
  int SuppressExternalCodecs;
} GdiplusStartupInput;
]])
safe_cdef("GdiplusStartupOutput",[[
typedef struct GdiplusStartupOutput {
  void* NotificationHook;
  void* NotificationUnhook;
} GdiplusStartupOutput;
]])
safe_cdef("GUID",[[
typedef struct {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;
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
]]
safe_cdef("RECT",[[
typedef struct tagRECT {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
} RECT;
]])
safe_cdef("COORD",[[
typedef struct _COORD {
  short X;
  short Y;
} COORD;
]])
safe_cdef("SMALL_RECT",[[
typedef struct _SMALL_RECT {
  short Left;
  short Top;
  short Right;
  short Bottom;
} SMALL_RECT;
]])
safe_cdef("CONSOLE_SCREEN_BUFFER_INFO",[[
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
  COORD dwSize;
  COORD dwCursorPosition;
  unsigned short wAttributes;
  SMALL_RECT srWindow;
  COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;
]])
ffi.cdef[[
void* GetDC(void* HWND);
int32_t GetClientRect(void* HWND,RECT* lpRect);
int32_t GetConsoleScreenBufferInfo(void* hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFO* lpConsoleScreenBufferInfo);
void* GetStdHandle(uint32_t nStdHandle);
]]
local gdiplus=ffi.load("gdiplus")
local handle=ffi.new("void*[1]")
local startup=ffi.new("GdiplusStartupInput")
startup.GdiplusVersion=1
gdiplus.GdiplusStartup(handle,startup,ffi.NULL)
local Symbol=0

local function ToWChar(str)
  str=win.Utf8ToUtf16(str)
  local result=ffi.new("wchar_t[?]",#str/2+1)
  ffi.copy(result,str)
  return result
end

local function InitImage(filename)
  local wnd=far.AdvControl(F.ACTL_GETFARHWND)
  local dc=C.GetDC(wnd)
  local image=ffi.new("void*[1]")
  local status=gdiplus.GdipLoadImageFromFile(ToWChar(filename),image)
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
    gdiplus.GdipImageGetFrameCount(image[0],dimensionIDs,frames);
    return {wnd=wnd,dc=dc,image=image,graphics=graphics,width=width[0],height=height[0],frames=frames[0]}
  end
  return false
end

local function DeleteImage(params)
  gdiplus.GdipDisposeImage(params.image.image[0])
  gdiplus.GdipDeleteGraphics(params.image.graphics[0])
end

local function InitArea(params)
  local rect=ffi.new("RECT")
  local info=ffi.new("CONSOLE_SCREEN_BUFFER_INFO")
  C.GetClientRect(params.image.wnd,rect)
  C.GetConsoleScreenBufferInfo(C.GetStdHandle(-11),info)

  local dx=math.floor(rect.right/(info.srWindow.Right-info.srWindow.Left))
  local dy=math.floor(rect.bottom/(info.srWindow.Bottom-info.srWindow.Top))

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
  gdiplus.GdipDrawImageRectI(params.image.graphics[0],params.image.image[0],params.RangedRect.left,params.RangedRect.top,params.RangedRect.right,params.RangedRect.bottom)
  if params.timer then
    params.image.frame=params.image.frame+1
    if params.image.frame==params.image.frames then
      params.image.frame=0
    end
    gdiplus.GdipImageSelectActiveFrame(params.image.image[0],ffi.cast("GUID*",params.image.guid),params.image.frame);
  end
end

local function ShowImage(xpanel)
  local vinfo,pinfo=viewer.GetInfo(),panel.GetPanelInfo(nil,xpanel)
  if vinfo.WindowSizeX==(pinfo.PanelRect.right-pinfo.PanelRect.left-1) and pinfo.PanelType==F.PTYPE_QVIEWPANEL then
    local params={CurPanel=bit64.band(pinfo.Flags,F.PFLAGS_FOCUS)~=0,Redraw=false,Key=false,Exit=false}
    params.image=InitImage(viewer.GetFileName())
    if params.image then
      local dialog
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
        {"DI_DOUBLEBOX",0,0,width+1,height+1,0,0,0,0,params.image.width.." x "..params.image.height},
        {"DI_USERCONTROL",1,1,width,height,buffer,0,0,0,""}
      }
      local function DlgProc(dlg,msg,param1,param2)
        if msg==F.DN_INITDIALOG then
          if params.image.frames>1 then
            local function ShowAnimation()
              FillBuffer()
              far.SendDlgMessage(dlg,F.DM_REDRAW)
            end
            params.timer=far.Timer(Delay,ShowAnimation)
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
          end
        elseif msg==F.DN_CONTROLINPUT or msg==F.DN_RESIZECONSOLE then
          if not params.Exit then
            params.Exit=true
            local key=msg==F.DN_RESIZECONSOLE and "CtrlR" or far.InputRecordToName(param2)
            if params.timer then params.timer:Close() end
            for _,v in ipairs(Refresh) do
              if v==key then
                key=key.." CtrlR"
                break
              end
            end
            far.MacroPost([[Keys("Esc ]]..key..[[")]])
          end
        end
      end
      params.DrawRect={}
      params.DrawRect.left=pinfo.PanelRect.left+1
      params.DrawRect.top=pinfo.PanelRect.top+1
      params.DrawRect.right=pinfo.PanelRect.right-1
      params.DrawRect.bottom=pinfo.PanelRect.bottom-1
      params.DCRect=InitArea(params)
      params.RangedRect=RangingPic(params)
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
      ShowImage(xpanel)
    end
  end
}

Event
{
  group="ExitFAR";
  action=function()
  end
}
