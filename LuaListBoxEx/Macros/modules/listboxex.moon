F=far.Flags
K=far.Colors

UpperKey=(key)->(far.LIsAlpha key) and (far.LUpperBuf key) or key

GetNextItem=(current,delta,direction)=>
  while current<=#@Items and current>0
    if not @Items[current].Hidden
      delta-=1
      break if delta<0
    current+=direction and 1 or -1
  current=0 if current<1 or current>#@Items
  current
GetNearestItem=(current,delta,direction)=>
  current=GetNextItem @,current,delta,direction
  if current<1 and #@Items>0
    current=direction and #@Items or 1
    current=GetNextItem @,current,1,not direction if @Items[current].Hidden
  current
NormalizePos=(direction)=>
  cnt=#@Items
  if cnt>0
    @Pos=cnt if @Pos>cnt
    @Pos=1 if @Pos<1
    if @Items[@Pos].Hidden or @Items[@Pos].Disabled
      new_index=0
      for dd in *{direction,not direction}
        iter=->
          ii,total=@Pos,@Wrap and @Pos or (dd and #@Items+1 or 0)
          ->
            ii+=dd and 1 or -1
            if @Wrap
              ii=1 if ii>cnt
              ii=cnt if ii<1
            ii~=total and ii or nil
        for ii in iter!
          if not (@Items[ii].Hidden or @Items[ii].Disabled)
            new_index=ii
            break
        break if new_index>0
      @Pos=new_index if new_index>0
  else @Pos=0
NormalizeTop==>
  cnt=#@Items
  if cnt>0
    @Top=1 if @Top<1
    @Top=cnt if @Top>cnt
    @Top=@Pos if @Top>@Pos
    top_pos=@Pos
    top_pos=GetNearestItem @,top_pos,@Height-1,false
    top_pos=@Pos if top_pos>@Pos
    @Top=GetNearestItem @,@Top,0,true
    @Top=top_pos if @Top<top_pos or @Top>@Pos
  else
    @Top=0
Init=(dlg)=>
  @_Dlg=dlg
  far.SendDlgMessage @Dlg!,F.DM_SETMOUSEEVENTNOTIFY,1
  @Colors.Background=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTBOX
  @Colors.Item=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTTEXT
  @Colors.Hotkey=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTHIGHLIGHT
  @Colors.Selected=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTSELECTEDTEXT
  @Colors.SelectedHotkey=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTSELECTEDHIGHLIGHT
  @Colors.Disabled=far.AdvControl F.ACTL_GETCOLOR,K.COL_DIALOGLISTDISABLED
  index,data=1,{}
  while far.GetDlgItem @Dlg!,index,data
    if @Buffer.rawhandle==data[6]
      @Index=index
      break
    index+=1
Draw==>
  current=@Top
  for ii=1,@Height
    kk,char,attr=-1,{},{}
    for jj=1,@Width
      if current<1
        char[jj]=" "
        attr[jj]=@Colors.Background
      else
        if kk>0 and kk<=@Items[current].Item\len!
          color=current==@Pos and "Selected" or (@Items[current].Disabled and "Disabled" or "Item")
          char[jj]=@Items[current].Item\sub kk,kk
          attr[jj]=@Items[current].Attribute[color][kk] or @Colors[color]
        else
          char[jj]=" "
          if @Items[current].Checked and jj==1
            char[jj]="√"
            char[jj]=@Items[current].CheckMark if @Items[current].CheckMark
          if current==@Pos
            attr[jj]=@Colors.Selected
          elseif @Items[current].Disabled
            attr[jj]=@Colors.Disabled
          else
            attr[jj]=@Colors.Item
      kk+=@Col-1 if kk==@Frozen
      kk+=1
    --
    for jj=1,@Width
      @Buffer[(ii-1)*@Width+jj]={Char:char[jj],Attributes:attr[jj]}
    current=GetNextItem @,current,1,true
ControlInput=(rec)=>
  if rec.EventType==F.MOUSE_EVENT
    if 0 ~= band rec.ButtonState,F.FROM_LEFT_1ST_BUTTON_PRESSED
      far.SendDlgMessage @Dlg!,F.DM_CLOSE,-1
      return true
  elseif rec.EventType==F.KEY_EVENT
    key=far.InputRecordToName rec
    redraw,direction,delta=true,true,0
    switch key
      when "Left"
        @Col-=1 if @HScroll and @Col>1
      when "Right"
        @Col+=1 if @HScroll
      when "Up"
        if @Wrap and @Pos==1 then @Pos=#@Items
        else delta=1
        @Pos=GetNearestItem @,@Pos,delta,false
        direction=false
      when "Down"
        if @Wrap and @Pos==#@Items then @Pos=1
        else delta=1
        @Pos=GetNearestItem @,@Pos,delta,true
      when "Home"
        @Pos=1
        @Pos=GetNearestItem @,@Pos,delta,true
      when "End"
        @Pos=#@Items
        @Pos=GetNearestItem @,@Pos,delta,false
      when "PgUp"
        delta=@Height
        @Pos=GetNearestItem @,@Pos,delta,false
        direction=false
      when "PgDn"
        delta=@Height
        @Pos=GetNearestItem @,@Pos,delta,true
      when "Esc"
        redraw=false
      else
        redraw=false
        first=UpperKey rec.UnicodeChar
        second=first
        if "\0"~=first
          second=UpperKey far.XLat first if far.LIsAlpha first
          check=(item)->(item.Hotkey==first or item.Hotkey==second) and not (item.Disabled or item.Hidden)
          for ii=1,#@Items
            if check @Items[ii]
              @Pos=ii
              redraw=true
              break
    if redraw
      NormalizePos @,direction
      NormalizeTop @
      far.SendDlgMessage @Dlg!,F.DM_REDRAW
      return true
  nil
Input=(rec)=>
  if @Index>0 and rec.EventType==F.MOUSE_EVENT and (band rec.EventFlags,F.MOUSE_MOVED)~=0
    rect=far.SendDlgMessage @Dlg!,F.DM_GETDLGRECT
    if rect
      x,y=rec.MousePositionX-rect.Left,rec.MousePositionY-rect.Top
      item=far.SendDlgMessage @Dlg!,F.DM_GETITEMPOSITION,@Index
      if item
        if x>=item.Left and x<=item.Right and y>=item.Top and y<=item.Bottom
          new=@Top+y-item.Top
          if new<=#@Items
            old=@Pos
            @Pos=new
            NormalizePos @,old>=new
            NormalizeTop @
            far.SendDlgMessage @Dlg!,F.DM_REDRAW
Redaw==>
  far.SendDlgMessage @Dlg!,F.DM_REDRAW if @Redraw>0
Normalize==>
  if @Redraw>0
    NormalizePos @,true
    NormalizeTop @
    Redaw @
NormalizeItem=(item)->
  'table'==type(item) and item or {Item:item or '',Attribute:{Item:{},Selected:{},Disabled:{}}}
class ListBox
  new:(@Width=1,@Height=1)=>
    @_Dlg=false
    @Redraw=1
    @Wrap=true
    @HScroll=true
    @Top=0
    @Pos=0
    @Frozen=0
    @Col=1
    @Index=0
    @Items={}
    @Colors={}
    @Buffer=far.CreateUserControl @Width,@Height
  Dlg:=>
    if not @_Dlg
      error "Invalid dialog",2
    @_Dlg
  DlgProc:(dlg,msg,param1,param2)=>
    switch msg
      when F.DN_INITDIALOG
        Init @,dlg
      when F.DN_DRAWDLGITEM
        Draw @ if param1==@Index
      when F.DN_CONTROLINPUT
        return ControlInput @,param2 if param1==@Index
      when F.DN_INPUT
        Input @,param2
    nil
  Add:(param1,param2)=>
    params={@Items}
    item=if 'number'==type(param1)
      table.insert params,param1
      param2
    else
      param1
    table.insert params,NormalizeItem item
    table.insert table.unpack params
    Normalize @
  Delete:(index)=>
    table.remove @Items,index
    @Top-=1 if index<@Top
    @Pos-=1 if index<@Pos
    Normalize @
  Update:(index,item)=>
    @Items[index]=NormalizeItem item
    Normalize @
  SetColor:(index,type,pos,color)=>
    item=@Items[index]
    item.Attribute[type][pos]=color if item and pos>0 and pos<=item.Item\len! and item.Attribute[type]
    Redaw @
  SetHotkey:(index,key)=>
    item=@Items[index]
    item.Hotkey=UpperKey key if item
  Toggle:(index)=>
    item=@Items[index]
    item.Checked=not item.Checked if item
    Redaw @
  Hide:(index)=>
    if not @Items[index].Hidden
      @Items[index].Hidden=true
      Normalize @
  UnHide:(index)=>
    if @Items[index].Hidden
      @Items[index].Hidden=false
      Normalize @
  Enable:(index)=>
    if @Items[index].Disabled
      @Items[index].Disabled=false
      Normalize @
  Disable:(index)=>
    if not @Items[index].Disabled
      @Items[index].Disabled=true
      Normalize @
  Current:=>#@Items>0 and @Items[@Pos],@Pos or nil
  Begin:=>@Redraw-=1
  End:=>
    @Redraw+=1
    Normalize @
  MoveUp:(index)=>
    if index>1
      @Items[index-1],@Items[index]=@Items[index],@Items[index-1]
      @Pos-=1
      Normalize @
  MoveDown:(index)=>
    if index<#@Items
      @Items[index+1],@Items[index]=@Items[index],@Items[index+1]
      @Pos+=1
      Normalize @
