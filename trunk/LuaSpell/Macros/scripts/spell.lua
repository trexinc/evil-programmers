local F=far.Flags
local config=
{
  --ru
  {
    dictionary=[[%FARPROFILE%\Dictionaries\ru_RU_yo.dic]],
    affix=[[%FARPROFILE%\Dictionaries\ru_RU_yo.aff]],
    regexstr=[[/[а-яёА-ЯЁ]+/]],
    color={Flags=bit64.bor(F.FCF_FG_4BIT,F.FCF_BG_4BIT),ForegroundColor=0xf,BackgroundColor=0x4},
    active=true
  },
  --en
  {
    dictionary=[[%FARPROFILE%\Dictionaries\en_US.dic]],
    affix=[[%FARPROFILE%\Dictionaries\en_US.aff]],
    regexstr=[[/[a-zA-Z]+/]],
    color={Flags=bit64.bor(F.FCF_FG_4BIT,F.FCF_BG_4BIT),ForegroundColor=0xf,BackgroundColor=0x2},
    active=true
  }
}

local editors={}
local colorguid=win.Uuid("46CCE102-965A-4e2d-8263-B59F268C74C8")
local active=false
local ffi=require("ffi")
ffi.cdef[[
void* HunspellInit(void* affixBuffer,size_t affixBufferSize,void* dictionaryBuffer,size_t dictionaryBufferSize,wchar_t* key);
void HunspellFree(void* handle);
bool HunspellSpell(void* handle,const wchar_t* word);
wchar_t** HunspellSuggest(void* handle,const wchar_t* word);
int lstrlenW(wchar_t* lpString);
]]
local hunspell=ffi.load("hunspell"..win.GetEnv("PROCESSOR_ARCHITECTURE"))
local kernel32=ffi.load("kernel32")

local function LoadFile(filename)
  local file=io.open(filename,"rb")
  local data
  if file then
    data=file:read("*a")
    file:close()
    local result=ffi.new("uint8_t[?]",#data)
    ffi.copy(result,data,#data)
    return result,#data
  end
end

local function ExpandEnv(str) return (str:gsub("%%(.-)%%",win.GetEnv)) end

for _,v in ipairs(config) do
  v.dict_data,v.dict_len=LoadFile(ExpandEnv(v.dictionary))
  v.affix_data,v.affix_len=LoadFile(ExpandEnv(v.affix))
  if v.dict_data and v.affix_data then
    v.handle=hunspell.HunspellInit(v.affix_data,v.affix_len,v.dict_data,v.dict_len,nil)
    v.regex=regex.new(v.regexstr)
  else
    v.active=false
  end
end

function GetData(id)
  local data=editors[id]
  if not data then
    editors[id]={start=0,finish=0}
    data=editors[id]
  end
  return data
end

local function ToWChar(str)
  str=win.Utf8ToUtf16(str)
  local result=ffi.new("wchar_t[?]",#str/2+1)
  ffi.copy(result,str)
  return result
end

local function ShowMenu(strings,wordLen)
  local info=editor.GetInfo()
  local menuOverheadWidth=6 --6 => 2 рамка, 2 тень, 2 место для чекмарка
  local menuOverheadHeight=3 --3 => 2 рамка, 1 тень
  local menuWidth=0
  local listItems={}
  for _,v in ipairs(strings) do
    menuWidth=math.max(menuWidth,v:len())
    table.insert(listItems,{Flags=0,Text=v})
  end
  local menuHeight=1
  local coorX=info.CurPos-info.LeftPos
  local coorY=info.CurLine-info.TopScreenLine
  local menuX=math.max(0,coorX+1-wordLen-menuWidth-menuOverheadWidth)
  menuX=(info.WindowSizeX-coorX)>(coorX+2-wordLen) and (coorX+1) or menuX --меню справа или слева от слова?
  local menuY=0
  if (info.WindowSizeY-coorY-1)>coorY+1 then --меню сверху или снизу?
    --снизу
    menuY=coorY+2
    menuHeight=info.WindowSizeY-menuY+1-menuOverheadHeight
    menuHeight=math.min(menuHeight,#strings)
  else
    --сверху
    menuY=coorY-#strings-1
    if menuY<1 then menuY=1 end
    menuHeight=coorY-menuY-1
  end

  --fix menu width
  if (menuX+menuWidth+menuOverheadWidth)>info.WindowSizeX then
    menuWidth=info.WindowSizeX-menuX-menuOverheadWidth
  end

  local items={
    {"DI_LISTBOX",0,0,menuWidth+3,menuHeight+1,listItems,0,0,0,""}
  }
  local function DlgProc(dlg,msg,param1,param2)
  end
  dialog=far.DialogInit(win.Uuid("ECD10910-8CC6-4685-AA8D-7D7413DD7D06"),menuX,menuY,menuX+menuWidth+3,menuY+menuHeight+1,nil,items,0,DlgProc)
  local result=far.DialogRun(dialog)>0 and far.SendDlgMessage(dialog,F.DM_LISTGETCURPOS,1).SelectPos or nil
  far.DialogFree(dialog)
  return result
end

local function CheckSpell()
  local pos,pos2=editor.GetInfo(-1).CurPos,0
  local row=editor.GetInfo(-1).CurLine
  local line=editor.GetString(-1,-1)
  local linestr=line.StringText
  local word=""
  if pos<=linestr:len()+1 then
    local slab=pos>1 and linestr:sub(1,pos-1):match('[%w_]+$') or ""
    local tail=linestr:sub(pos):match('^[%w_]+') or ""
    pos2=pos-slab:len()
    word=slab..tail
  end
  if word:len() then
    word_data=ToWChar(word)
    for _,v in ipairs(config) do
      if v.active and v.regex:match(word) and not hunspell.HunspellSpell(v.handle,word_data) then
        suggestions=hunspell.HunspellSuggest(v.handle,word_data)
        local ii,items=0,{}
        while suggestions[ii]~=ffi.NULL do
          table.insert(items,win.Utf16ToUtf8(ffi.string(suggestions[ii],2*kernel32.lstrlenW(suggestions[ii]))))
          ii=ii+1
        end
        if #items>0 then
          local sel=ShowMenu(items,word:len())
          if sel then
            linestr=line.StringText:sub(1,pos2-1)..items[sel]..line.StringText:sub(pos2+word:len())
            editor.SetString(-1,0,linestr,line.StringEol)
          end
        end
        break
      end
    end
  end
end

local function RemoveColors(id)
  local data=GetData(id)
  for ii=data.start,data.finish-1 do
    editor.DelColor(id,ii,0,colorguid)
  end
  data.start=0
  data.finish=0
  return data
end

local function RemoveColorsAll()
  local wincount=far.AdvControl(F.ACTL_GETWINDOWCOUNT,0,0)
  for ii=1,wincount do
    local info=far.AdvControl(F.ACTL_GETWINDOWINFO,ii,0)
    if info and F.WTYPE_EDITOR==info.Type then
      RemoveColors(info.Id)
    end
  end
end

local function CheckSpellAll(ei)
  local data=RemoveColors(ei.EditorID)
  data.start=ei.TopScreenLine
  data.finish=math.min(ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines)
  local regex=regex.new([[/\b\i+\b/]])
  for ii=data.start,data.finish-1 do
    local line=editor.GetString(-1,ii).StringText
    local pos=1
    while true do
      local sbegin,send=regex:find(line,pos)
      if not sbegin then break end
      pos=send+1
      local word=line:sub(sbegin,send)
      for _,v in ipairs(config) do
        if v.active and v.regex:match(word) and not hunspell.HunspellSpell(v.handle,ToWChar(word)) then
          editor.AddColor(ei.EditorID,ii,sbegin,send,F.ECF_TABMARKCURRENT,v.color,199,colorguid)
          break
        end
      end
    end
  end
end

Event
{
  group="EditorEvent";
  action=function(id,event,param)
    if event==F.EE_READ then
      editors[id]={start=0,finish=0}
    end
    if event==F.EE_CLOSE then
      editors[id]=nil
    end
    if event==F.EE_REDRAW then
      if active then
        CheckSpellAll(editor.GetInfo())
      end
    end
  end
}

Event
{
  group="ExitFAR";
  action=function()
    RemoveColorsAll()
    for _,v in ipairs(config) do
      if v.active then
        hunspell.HunspellFree(v.handle)
      end
    end
  end
}

Macro
{
  area="Editor";key="F3";description="check spell";
  action=CheckSpell
}

Macro
{
  area="Editor";key="ShiftF3";description="highlight spell errors";
  action=function()
    active=not active
    if not active then
      RemoveColorsAll()
    end
    editor.Redraw(-1)
  end
}
