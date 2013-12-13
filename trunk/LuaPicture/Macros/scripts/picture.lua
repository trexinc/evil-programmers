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
safe_cdef("PropertyItem",[[
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
safe_cdef("CONSOLE_SCREEN_BUFFER_INFOEX",[[
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
ffi.cdef[[
void* GetDC(void* HWND);
int ReleaseDC(void* HWND,void* HDC);
int32_t GetClientRect(void* HWND,RECT* lpRect);
int32_t GetConsoleScreenBufferInfo(void* hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFO* lpConsoleScreenBufferInfo);
int32_t GetConsoleScreenBufferInfoEx(void* hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFOEX* lpConsoleScreenBufferInfoEx);
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

local function BGR2RGB(color)
  return bit64.bor(bit64.band(bit64.rshift(color,16),0xff),bit64.band(color,0xff00ff00),bit64.band(bit64.lshift(color,16),0xff0000))
end

local function PointInRect(point,rect)
  return point.MousePositionX>=rect.left and point.MousePositionX<=rect.right and point.MousePositionY>=rect.top and point.MousePositionY<=rect.bottom
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
    gdiplus.GdipImageGetFrameCount(image[0],dimensionIDs,frames)
    local delaysize=ffi.new("unsigned int[1]")
    gdiplus.GdipGetPropertyItemSize(image[0],0x5100,delaysize)
    local delay={}
    if delaysize[0]>0 then
      local delayraw=ffi.cast("PropertyItem*",ffi.new("char[?]",delaysize[0]))
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
    return {wnd=wnd,dc=dc,image=image,graphics=graphics,brush=brush,width=width[0],height=height[0],frames=frames[0],delay=delay,memory={image=memimage,graphics=memgraphics}}
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
  local width=params.RangedRect.right-params.RangedRect.left
  local height=params.RangedRect.bottom-params.RangedRect.top
  gdiplus.GdipFillRectangleI(params.image.memory.graphics[0],params.image.brush[0],0,0,params.image.width,params.image.height)
  --GdipDrawImageI fails on some images
  gdiplus.GdipDrawImageRectI(params.image.memory.graphics[0],params.image.image[0],0,0,params.image.width,params.image.height)
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
