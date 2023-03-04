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
panels=()->(Panel panel.GetPanelDirectory nil,1),Panel panel.GetPanelDirectory nil,0
last,active,passive={},1,0
last[active],last[passive]=panels!
folders={}
for folder in *init
  switch type folder
    when 'string'
      folder=
        Name:folder
        PluginId:string.rep('\0',16)
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
Event
  group:"FolderChanged"
  action:()->
    current={}
    current[active],current[passive]=panels!
    process=(idx)->
      found=false
      for folder in *folders
        if current[idx]==folder
          found=folder
          break
      if last[idx].Special
        current[idx].Sort=last[idx].Sort
        current[idx].Order=last[idx].Order
      else
        info=panel.GetPanelInfo nil,idx
        if info
          current[idx].Sort=info.SortMode
          current[idx].Order=(bit64.band info.Flags,F.PFLAGS_REVERSESORTORDER)==F.PFLAGS_REVERSESORTORDER
      sort,order=current[idx].Sort,current[idx].Order
      if found and found.Sort
        with found
          sort=.Sort
          order=.Order
        current[idx].Special=true
      info=panel.GetPanelInfo nil,idx
      top=info and info.TopPanelItem
      panel.SetSortMode nil,idx,sort
      panel.SetSortOrder nil,idx,order
      if top
        info=panel.GetPanelInfo nil,idx
        if info and info.CurrentItem
          panel.RedrawPanel nil,idx,{CurrentItem:info.CurrentItem,TopPanelItem:top}
      last[idx]=current[idx]
      if found and 'function'==type found.Action then found.Action idx,{Name:current[idx].Name,Param:current[idx].Param,PluginId:current[idx].PluginId,File:current[idx].File}
    if current[active]==last[passive] and current[passive]==last[active]
      last[active],last[passive]=last[passive],last[active]
    else
      for ii in *{active,passive}
        if last[ii]!=current[ii]
          process ii
