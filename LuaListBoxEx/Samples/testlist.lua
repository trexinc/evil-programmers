local F=far.Flags
local ListBox=require"listboxex"
local listbox=ListBox(63,16)

local items={
  {"DI_DOUBLEBOX",   3, 1,67,18,0,0,0,0,                    "Very Cool Dialog"},
  {"DI_USERCONTROL", 4, 2,66,17,listbox.Buffer,0,0,0,       ""},
  {"DI_BUTTON",      3,19, 0, 0,0,0,0,{DIF_DEFAULTBUTTON=1},"Ok"},
  {"DI_BUTTON",     12,19, 0, 0,0,0,0,0,                    "Cancel"}
}

local DlgProc=function(dlg,msg,param1,param2)
  if listbox:DlgProc(dlg,msg,param1,param2) then
    return true
  else
    if F.DN_INITDIALOG==msg then
      listbox:Begin()
      listbox:Add "a│aa"
      listbox:Add "b│bb"
      listbox:Add "c│cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
      listbox:Add "1│ bbb"
      listbox:Add "2│ bbb"
      listbox:Add "3│ bbb"
      local color=function(start,finish,fg,bg,type)
        bg=bg or 0x7
        type=type or "Item"
        for ii=start,finish do listbox:SetColor(3,type,ii,{Flags=bit64.bor(F.FCF_FG_4BIT,F.FCF_BG_4BIT),ForegroundColor=fg,BackgroundColor=bg}) end
      end
      color(3,5,0xc)
      color(8,10,0xa)
      color(13,15,0x9)
      color(13,15,0x9,0,"Selected")
      listbox:SetHotkey(3,"a")
      listbox.Frozen=2
      listbox:End()
    elseif F.DN_CONTROLINPUT==msg then
      local key=far.InputRecordToName(param2)
      if "Ins"==key then
        listbox:Toggle(listbox.Pos)
      elseif "Del"==key then
        listbox:Delete(listbox.Pos)
      elseif "F8"==key then
        listbox:Disable(listbox.Pos)
      elseif "CtrlUp"==key then
        listbox:MoveUp(listbox.Pos)
      elseif "CtrlDown"==key then
        listbox:MoveDown(listbox.Pos)
      end
    end
  end
end

local dialog=far.DialogInit(win.Uuid"b1bf2eac-70c2-4099-9a1f-571c0ffe2c92",-1,-1,71,21,nil,items,0,DlgProc)
far.DialogRun(dialog)
far.DialogFree(dialog)
