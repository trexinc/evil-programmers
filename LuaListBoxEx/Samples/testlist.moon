F=far.Flags
ListBox=require"listboxex"
listbox=ListBox 63,16

items={
  {"DI_DOUBLEBOX",   3, 1,67,18,0,0,0,0,                    "Very Cool Dialog"},
  {"DI_USERCONTROL", 4, 2,66,17,listbox.Buffer,0,0,0,       ""},
  {"DI_BUTTON",      3,19, 0, 0,0,0,0,{DIF_DEFAULTBUTTON:1},"Ok"},
  {"DI_BUTTON",     12,19, 0, 0,0,0,0,0,                    "Cancel"}
}

DlgProc=(dlg,msg,param1,param2)->
  if listbox\DlgProc dlg,msg,param1,param2
    true
  else
    with listbox
      switch msg
        when F.DN_INITDIALOG
          \Begin!
          \Add "a│aa"
          \Add "b│bb"
          \Add "c│cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          \Add "1│ bbb"
          \Add "2│ bbb"
          \Add "3│ bbb"
          color=(start,finish,fg,bg=0x7,type="Item")->
            for ii=start,finish do \SetColor 3,type,ii,{Flags:(bit64.bor F.FCF_FG_4BIT,F.FCF_BG_4BIT),ForegroundColor:fg,BackgroundColor:bg}
          color(3,5,0xc)
          color(8,10,0xa)
          color(13,15,0x9)
          color(13,15,0x9,0,"Selected")
          \SetHotkey 3,"a"
          .Frozen=2
          \End!
        when F.DN_CONTROLINPUT
          key=far.InputRecordToName param2
          switch key
            when "Ins"
              \Toggle listbox.Pos
            when "Del"
              \Delete listbox.Pos
            when "F5"
              \Begin!
              start=far.FarClock!
              for _=1,200
                \Add"123"
              far.Show far.FarClock!-start
              \End!
            when "F8"
              \Disable listbox.Pos
            when "CtrlUp"
              \MoveUp listbox.Pos
            when "CtrlDown"
              \MoveDown listbox.Pos
    nil

dialog=far.DialogInit (win.Uuid"df8aaab5-f781-4488-be63-c817364769a3"),-1,-1,71,21,nil,items,0,DlgProc
far.DialogRun dialog
far.DialogFree dialog
