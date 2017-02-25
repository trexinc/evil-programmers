F=far.Flags
K=far.Colors
ffi=require"ffi"
ec=ffi.cast("struct PluginStartupInfo*",far.CPluginStartupInfo!).EditorControl
egs=ffi.new "struct EditorGetString"
egs.StructSize=ffi.sizeof egs
ess=ffi.new "struct EditorSetString"
ess.StructSize=ffi.sizeof ess
editors={}
colorguid=win.Uuid "F018DA49-6EB9-49C3-84D8-0F5E7BA20EFB"
pcall ffi.cdef,[[
struct ABShared
{
  intptr_t EditorID;
  intptr_t Top;
  GUID Id;
};
]]
farguid=string.rep('\0',16)
IsMms=(str)->return str[0]==45 and str[1]==45 and str[2]==32 and str[3]==0
IsSpace=(char)->return char==32 or char==9
KillSpaces1=(id,lineno,mms=false,spaces=true,eol)->
  egs.StringNumber=lineno-1
  if (ec id,"ECTL_GETSTRING",0,egs)~=0
    ess.StringNumber=egs.StringNumber
    ess.StringLength=egs.StringLength
    ess.StringText=egs.StringText
    ess.StringEOL,neweol=if eol and egs.StringEOL[0]~=0
      eol=win.Utf8ToUtf16 eol..'\0'
      (ffi.cast "const wchar_t*",eol),true
    else
      egs.StringEOL,false
    collectgarbage!
    if spaces and (not mms or not IsMms ess.StringText)
      while ess.StringLength>0
        if not IsSpace ess.StringText[ess.StringLength-1] then break
        ess.StringLength-=1
    if ess.StringLength~=egs.StringLength or neweol then ec id,"ECTL_SETSTRING",0,ess

KillSpaces=(id,mms,spaces,eol)->
  total=(editor.GetInfo id).TotalLines
  for ii=1,total
    KillSpaces1 id,ii,mms,spaces,eol

KillEmptyLines=(id)->
  info=editor.GetInfo id
  total=info.TotalLines
  getlen=(row)->editor.GetString(id,row,1).StringLength
  if 0==getlen total then while total>1
    if 0==getlen total-1
      editor.DeleteString id
      total-=1
    else
      break
  editor.SetPosition id,info

SmartHome=->
  info=editor.GetInfo!
  pp1,pp2=info.CurPos,editor.GetString(-1,0,3)\find('%S') or 1
  editor.SetPosition -1,0,(pp1==1 or pp1>pp2) and pp2 or 1
  if 0==bit64.band info.Options,F.EOPT_PERSISTENTBLOCKS then editor.Select -1,'BTYPE_NONE'
  editor.Redraw!

FindIndent=(lineno,pos,lines,compare,brk)->
  for _=1,lines
    lineno-=1
    line=editor.GetString -1,lineno,3
    if line and line\len!>0
      pos2=line\find('%S') or line\len!+1
      if compare(pos,pos2) then return pos2,line\sub 1,pos2-1
      if brk then break
  nil

SmartTabBs=(lines,tab)->
  info=editor.GetInfo!
  ll,pp=info.CurLine,info.CurPos
  line,eol=editor.GetString -1,0,3
  if (line\sub 1,pp-1)\match "^%s*$"
    pp2,newprefix=FindIndent ll,pp,lines,(if tab then (x,y)->x<y else (x,y)->x>y),tab
    if newprefix
      suffix=line\sub pp
      editor.SetString -1,0,newprefix..suffix,eol
      editor.SetPosition -1,0,pp2
      editor.Redraw!
      return true
  false

dump=(o)->
  if type(o)=='table'
    s='{ '
    for k,v in pairs(o)
      if type(k)~='number' then k='"'..k..'"'
      s=s..'['..k..'] = '..dump(v)..','
    s .. '} '
  else
    tostring(o)

Schemes=require"editorsettings"

FixSchemes=(Sch)->
  default=far.AdvControl F.ACTL_GETCOLOR,K.COL_EDITORTEXT
  color=(fg,bg)->
    flag=(c,f)->c and (c<0x10 and f or 0) or bit64.bor default.Flags,f
    flags=bit64.bor (flag fg,F.FCF_FG_4BIT),(flag bg,F.FCF_BG_4BIT)
    (fg or bg) and {Flags:flags,ForegroundColor:fg or default.ForegroundColor,BackgroundColor:bg or default.BackgroundColor}
  invert=(c)->
    if c
      fix=(v1,v2)->(v1==bit64.band c.Flags,v1) and v2 or 0
      {ForegroundColor:c.BackgroundColor,BackgroundColor:c.ForegroundColor,Flags:bit64.bor (bit64.band c.Flags,bit64.bnot 3),(fix 1,2),(fix 2,1)}
  decodeK=(c)->color if 'table'==type c then c[1],c[2] else c
  decodeR=(c)->
    if 'table'==type c
      if c[2] then c[3] or=c[1] else c[2],c[3]=c[1],c[1]
      for ii=1,3 do c[ii]=decodeK c[ii]
    else
      c=color c
      c={c,c,c}
    c
  fix=(region)->
    regions=rawget region,'Regions'
    if regions
      for r in *regions
        fix r
    with region
      if .Keywords
        .Keywords.ColorFull or=decodeK .Keywords.Color
        for ii=1,#.Keywords
          .Keywords[ii]={.Keywords[ii]} if 'string'==type .Keywords[ii]
          .Keywords[ii].ColorFull or=(decodeK .Keywords[ii].Color) or .Keywords.ColorFull
      .ColorFull or=decodeR .Color
      if .Left and .Right and 'nil'==type .Pair then .Pair=true
  for s in *Sch
    s.First={s.First} if 'string'==type s.First
    if s.Highlite
      fix with s.Highlite
        if not .Pairs then .Pairs={}
        .Pairs.ColorFull or=decodeK .Pairs.Color
        .Pairs.ColorErrorFull or=invert .Pairs.ColorFull

FixSchemes Schemes

Highlite=(id,tt,top)->
  if tt.o.Highlite
    tocache=(v)->1+math.floor v/50
    fromcache=(v)->(v-1)*50+1
    clone=(t)->{k,('table'==type v) and (clone v) or v for k,v in pairs t}
    insert,remove=table.insert,table.remove
    ei=editor.GetInfo id
    start,finish=(math.min ei.TopScreenLine,tt.startline,top),math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines
    start=tocache start
    if start>#tt.cache
      start=#tt.cache
    else if start<#tt.cache
      for ii=start+1,#tt.cache
        tt.cache[ii]=nil
    left,right=ei.LeftPos,ei.LeftPos+ei.WindowSizeX
    margins=:left,top:ei.TopScreenLine,:right,bottom:math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines+1
    addcolor=(line,s,e,c,p=0)->
      if c and line>=margins.top and line<margins.bottom and not (s>=margins.right or e<margins.left)
        editor.AddColor id,line,s,e,F.ECF_AUTODELETE,c,p,colorguid
    state,state_data,pairs=(clone tt.cache[start].state),(clone tt.cache[start].data),(clone tt.cache[start].pairs)
    {CurPos:curpos,CurLine:curline}=editor.GetInfo id
    checkCursor=(line,pos,len)->(line==curline) and curpos>=pos and curpos<(pos+len)
    getRegion=->
      r=tt.o.Highlite
      for s in *state
        if 0==s then break
        r=r.Regions[s]
      r
    region,regionstart=getRegion!,1
    updateRegion=(s,line=false,pos=1,len)->
      add=(c,p=0)->addcolor line,pos,pos+len-1,c,p
      curpair=line and len and checkCursor line,pos,len
      if line
        addcolor line,regionstart,pos-1,region.ColorFull[2]
        if s==0 and len
          add region.ColorFull[3]
          if region.Pair
            pair=remove pairs
            if curpair or (pair and pair.cur)
              addcolor pair.line,pair.pos1,pair.pos2,tt.o.Highlite.Pairs.ColorFull,100 if pair and curpair
              add tt.o.Highlite.Pairs.ColorFull,100
          pos+=len
      if 0==state[#state]
        if 0==s
          if #state>1
            remove state
            state[#state]=0
        else
          state[#state]=s
      else
        if 0==s
          state[#state]=s
        else
          insert state,s
      region=getRegion!
      regionstart=pos
      if s>0 and len
        pos1,pos2=pos,pos+len-1
        insert pairs,{:line,:pos1,:pos2,cur:curpair}
        add region.ColorFull[1]
        if region.Pair
          if curpair
            add tt.o.Highlite.Pairs.ColorFull,100
        regionstart+=len
    match=(str,patt,init)->
      switch type patt
        when 'string'
          str\match '^'..patt,init
        when 'userdata'
          res=patt\match str,init
          if res then string.sub str,init,res-1 else false
        when 'function'
          res,next=patt state_data,str,init
          if next then match str,res,init else res
    start=fromcache start
    for ii=start,finish
      regionstart=1
      tt.cache[tocache ii]={state:(clone state),data:(clone state_data),pairs:(clone pairs)} if ii%50==1
      {StringText:line,StringLength:len}=editor.GetString id,ii,0
      if 0==bit64.band ei.Options,F.EOPT_EXPANDALLTABS
        margins.left,margins.right,pos,symb=0,len+1,0,0
        tab=ei.TabSize
        for fix,chars,tabs in line\gmatch"()[^\t]*()[\t]*()"
          tabs-=chars
          chars-=fix
          pos+=chars
          symb+=chars
          if margins.left==0 and pos>=left
            margins.left=symb-pos+left
          if right>symb and pos>=right
            margins.right=symb-pos+right
            break
          if tabs>0
            pos+=tab-pos%tab+tab*(tabs-1)
            symb+=tabs
            if margins.left==0 and pos>=left
              margins.left=symb-math.floor (pos-left)/tab
            if right>symb and pos>=right
              margins.right=symb-math.floor (pos-right)/tab
              break
        if 0==margins.left then margins.right=0
      posU,posB=1,1
      match2=(patt)->match line,patt,posB
      while posU<=len
        stepU,stepB=1,string.len line\match '.',posB
        match3=(patt)->
          m=match2 patt
          if m
            stepU,stepB=m\len!,string.len m
            true
        match4=(patt,s)->
          if match3 patt
            updateRegion s,ii,posU,stepU
            true
        skip=false
        if region.Keywords
          for keyword in *region.Keywords
            if not (keyword.Start and posU>1)
              if match3 keyword[1]
                add=(c,p)->addcolor ii,posU,posU+stepU-1,c,p
                addcolor ii,regionstart,posU-1,region.ColorFull[2]
                add keyword.ColorFull
                regionstart=posU+stepU
                skip=true
                if keyword.Open or keyword.Close
                  curpair=checkCursor ii,posU,stepU
                  if keyword.Open
                    insert pairs,{line:ii,pos1:posU,pos2:posU+stepU-1,cur:curpair,type:keyword.Open}
                    if curpair
                      add tt.o.Highlite.Pairs.ColorFull,100
                  else
                    pair=remove pairs
                    if curpair or (pair and pair.cur)
                      addcolor pair.line,pair.pos1,pair.pos2,pair.type==keyword.Close and tt.o.Highlite.Pairs.ColorFull or tt.o.Highlite.Pairs.ColorErrorFull,100 if pair and curpair
                      add (pair and pair.cur and pair.type~=keyword.Close) and tt.o.Highlite.Pairs.ColorErrorFull or tt.o.Highlite.Pairs.ColorFull,100
                break
        if not skip and region.Regions
          for kk=1,#region.Regions
            if not (region.Regions[kk].Start and posU>1)
              if match4 region.Regions[kk].Left,kk
                skip=true
                break
        if not skip
          if region.Right
            match4 region.Right,0
        posU+=stepU
        posB+=stepB
      if not region.Right or match2 region.Right then updateRegion 0,ii,len+1
      else addcolor ii,regionstart,len,region.ColorFull[2]
    tt.startline=fromcache #tt.cache

InitType=(obj)->{o:obj,cache:{{state:{0},data:{},pairs:{}}},startline:1}

GetType1=(FileName,FirstLine)->
  for scheme in *Schemes
    tt=type scheme.Type
    cmp=(mask,fn)->far.ProcessName F.PN_CMPNAMELIST,mask,fn,F.PN_SKIPPATH
    if (tt=='string' and cmp scheme.Type,FileName) or (tt=='function' and scheme.Type cmp,FileName)
      return scheme
    if 'table'==type scheme.First
      for first in *scheme.First
        if FirstLine\match first
          return scheme
  nil

GetType=(id,FileName)->
  tt=editors[id]
  if not tt
    tt=InitType GetType1 FileName,editor.GetString id,1,3
    editors[id]=tt
  tt

ApplyType=(id,tt,startup,fn)->
  params={
    {F.ESPT_TABSIZE         ,"TabSize"}
    {F.ESPT_EXPANDTABS      ,"ExpandTabs"}
    {F.ESPT_AUTOINDENT      ,"AutoIndent"}
    {F.ESPT_CURSORBEYONDEOL ,"CursorBeyondEol"}
    {F.ESPT_CHARCODEBASE    ,"CharCodeBase"}
    {F.ESPT_CODEPAGE        ,"CodePage",true}
    {F.ESPT_SAVEFILEPOSITION,"SaveFilePosition"}
    {F.ESPT_LOCKMODE        ,"LockMode"}
    {F.ESPT_SETWORDDIV      ,"WordDiv"}
    {F.ESPT_SHOWWHITESPACE  ,"ShowWhiteSpace"}
    {F.ESPT_SETBOM          ,"SetBOM",true}
  }
  for param in *params do if type(tt.o[param[2]])~='nil' and (not param[3] or (startup and not mf.fexist fn)) then editor.SetParam id,param[1],tt.o[param[2]]

redraw=0
Event
  group:"EditorEvent"
  action:(id,Event)->
    if Event==F.EE_CLOSE
      editors[id]=nil
    elseif Event==F.EE_SAVE or Event==F.EE_READ or Event==F.EE_REDRAW or Event==F.EE_GOTFOCUS
      fn=editor.GetFileName id
      tt=GetType id,fn
      if tt
        if Event==F.EE_SAVE
          if tt.o.KillSpaces or tt.o.Eol then KillSpaces id,tt.o.MinusMinusSpace,tt.o.KillSpaces,tt.o.Eol
          if tt.o.KillEmptyLines then KillEmptyLines id
        elseif Event==F.EE_READ
          ApplyType id,tt,true,fn
        elseif Event==F.EE_REDRAW and redraw==0
          redraw+=1
          ab=_G.airbrush and ffi.cast "struct ABShared*",_G.airbrush
          top,guid=if ab and ab.EditorID==id then (tonumber ab.Top),ffi.string ab.Id,16 else math.huge,farguid
          Highlite id,tt,top if guid==farguid
          if tt.o.WhiteSpaceColor
            ei=editor.GetInfo id
            if 0~=bit64.band ei.Options,F.EOPT_SHOWWHITESPACE
              start,finish=ei.TopScreenLine,math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines
              for ii=start,finish
                line,pos=editor.GetString(id,ii),1
                while true
                  jj,kk=line.StringText\cfind("([%s]+)",pos)
                  if not jj then break
                  editor.AddColor id,ii,jj,kk,F.ECF_AUTODELETE,tt.o.WhiteSpaceColor,100,colorguid
                  pos=kk+1
                if 0~=bit64.band ei.Options,F.EOPT_SHOWLINEBREAK then editor.AddColor id,ii,line.StringLength+1,line.StringLength+line.StringEOL\len!,F.ECF_AUTODELETE,tt.o.WhiteSpaceColor,100,colorguid
          redraw-=1


KeyData={
  {
    Key:0x23 --VK_END
    Shift:0x13 --shift|lalt|ralt
    Option:"KillSpaces"
    Action:(_,mms)->
      KillSpaces1 -1,0,mms
      false
  }
  {
    Key:0x24 --VK_HOME
    Shift:0
    Option:"SmartHome"
    Action:->
      SmartHome!
      true
  }
  {
    Key:9 --VK_TAB
    Shift:0
    Option:"SmartTab"
    Action:(lines)->SmartTabBs lines,true
  }
  {
    Key:8 --VK_BACK
    Shift:0
    Option:"SmartBs"
    Action:(lines)->SmartTabBs lines
  }
}

Event
  group:"EditorInput"
  action:(rec)->
    if rec.EventType==F.KEY_EVENT and rec.KeyDown
      cc=bit.band(rec.ControlKeyState,0x1f)
      for key in *KeyData
        if (cc==0 or (bit.band(cc,key.Shift)~=0 and bit.band(cc,bit.bnot(key.Shift))==0)) and rec.VirtualKeyCode==key.Key
          tt=GetType editor.GetInfo!.EditorID,editor.GetFileName!
          if tt and tt.o[key.Option] then return key.Action tt.o.Lines or 10,tt.o.MinusMinusSpace
    false

MenuItem
  menu: "Plugins"
  area: "Editor"
  guid: "4DDEE94D-F1B4-440E-982F-26AAA826CEE9"
  text: "Editor Settings"
  action: ->
    id=editor.GetInfo!.EditorID
    tt=editors[id]
    if tt
      check=(c)->c.__name==tt.o.__name
      hotkeys="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      gethotkey=->(if hotkeys\len!>0 then "&"..(hotkeys\sub 1,1)..". " else "   "),do hotkeys=hotkeys\sub 2
      result=far.Menu{Id:win.Uuid"34BB1EE6-E7E1-44F4-A8DC-D51CF1B85E4C"},[{text:gethotkey!..scheme.Title,checked:check(scheme),selected:check(scheme),value:InitType scheme} for scheme in *Schemes]
      if result
        editors[id]=result.value
        ApplyType id,result.value
        editor.Redraw id
