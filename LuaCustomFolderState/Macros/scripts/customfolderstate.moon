res,init=pcall require,"customfolderstate_user"
if not res then init={}
F=far.Flags
insert=table.insert
expandrnv=(str)->str\gsub "%%(.-)%%",win.GetEnv
class Panel
  new: (panel)=>
    with panel
      @Name=.Name\lower!
      @Param=.Param
      @PluginId=.PluginId
      @File=.File\lower!
      @Sort=.Sort
      @Order=.Order
      @Action=.Action
    @Special=false
  __eq: (a,b)->
    cmp=(a,b)->('\\*'==a.Name\sub -2,-1) and (a.Name\sub 1,-3)==b.Name\sub 1,a.Name\len!-2
    (a.Name==b.Name or (cmp a,b) or (cmp b,a)) and a.Param==b.Param and a.PluginId==b.PluginId and a.File==b.File
active,passive=1,0
empty={Name:'',File:''}
panels=()->{[active]:(Panel panel.GetPanelDirectory nil,active),[passive]:Panel panel.GetPanelDirectory nil,passive}
last=
  [active]:Panel empty
  [passive]:Panel empty
folders={}
setpanelstate=(idx,sort,order)->
  info=panel.GetPanelInfo nil,idx
  top=info and info.TopPanelItem
  panel.SetSortMode nil,idx,sort
  panel.SetSortOrder nil,idx,order
  if top
    info=panel.GetPanelInfo nil,idx
    if info and info.CurrentItem
      panel.RedrawPanel nil,idx,{CurrentItem:info.CurrentItem,TopPanelItem:top}
process=(idx,current)->
  found=false
  for folder in *folders
    if current==folder
      found=folder
      break
  if last[idx].Special
    current.Sort=last[idx].Sort
    current.Order=last[idx].Order
  else
    info=panel.GetPanelInfo nil,idx
    if info
      current.Sort=info.SortMode
      current.Order=(bit64.band info.Flags,F.PFLAGS_REVERSESORTORDER)==F.PFLAGS_REVERSESORTORDER
  sort,order=current.Sort,current.Order
  if found and found.Sort
    with found
      sort=.Sort
      order=.Order
    current.Special=true
  setpanelstate idx,sort,order
  last[idx]=current
  if found and 'function'==type found.Action then found.Action idx,{Name:current.Name,Param:current.Param,PluginId:current.PluginId,File:current.File}
for folder in *init
  switch type folder
    when 'string'
      folder=
        Name:folder
        PluginId:string.rep '\0',16
    when 'table'
      nil
    else
      folder={}
  with folder
    if .Name and .Name!=''
      isset=(v,d)->if nil==v then d else v
      .Name=expandrnv .Name
      .Param=.Param or ''
      .PluginId=.PluginId and (36==.PluginId\len!) and (win.Uuid .PluginId) or (string.rep '\0',16)
      .File=.File or ''
      .Sort=isset .Sort,init.Sort
      .Order=isset .Order,init.Order
      insert folders,Panel folder
main=()->
  current=panels!
  if current[active]==last[passive] and current[passive]==last[active]
    last[active],last[passive]=last[passive],last[active]
  else
    for ii in *{active,passive}
      if last[ii]!=current[ii]
        process ii,current[ii]
main!
Event
  group:"FolderChanged"
  action:main
Event
  group:"ExitFAR"
  action:()->
    for ii in *{active,passive}
      if last[ii].Special
        setpanelstate ii,last[ii].Sort,last[ii].Order
