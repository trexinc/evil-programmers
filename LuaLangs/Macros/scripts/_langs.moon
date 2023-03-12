F=far.Flags
gmeta={__index:_G}
load=(compile)->
  (fname)->
    file,err=io.open fname
    if file
      text=assert file\read"*a"
      file\close!
      if (string.sub text,1,3)=="\239\187\191" then text=string.sub text,4
      compile text,fname
    else
      nil,err
languages={
  --https://yuescript.org/
  {
    module:"yue"
    mask:"*.yue"
    name:"YueMacro"
    init:(m)->m.insert_loader!
    load:(m)->m.loadfile
  }
  --https://moocscript.fun/
  {
    module:"moocscript.core"
    mask:"*.mooc"
    name:"MoonCakeMacro"
    load:(m)->load m.loadstring
  }
  --https://erde-lang.github.io/
  {
    module:"erde"
    mask:"*.erde"
    name:"ErdeMacro"
    init:(m)->m.load!
    load:(m)->load (text,fname)->
      compiled=m.compile text
      loadstring compiled,fname
  }
  --https://github.com/teal-language/tl
  {
    module:"tl"
    mask:"*.tl"
    name:"TealMacro"
    init:(m)->m.loader!
    load:(m)->load m.load
  }
}
loadonefile=(name,load)->
  (_,FullPath)->
    process=(f,...)->
      res,msg=f ...
      far.Message msg,name,nil,"wl" if not res
      res
    f=process load,FullPath
    if f
      FuncList={"Macro","Event","MenuItem","CommandLine","PanelModule","ContentColumns","NoMacro","NoEvent","NoMenuItem","NoCommandLine","NoPanelModule","NoContentColumns"}
      genv,lenv=getfenv!,{}
      setmetatable lenv,gmeta
      for name in *FuncList do lenv[name]=genv[name]
      setfenv f,lenv
      ok=process pcall,f,FullPath
      for name in *FuncList do lenv[name]=nil
old_loaders={}
for loader in *package.loaders
  table.insert old_loaders,loader
for lang in *languages
  ok,m=pcall require,lang.module
  if ok
    lang.init m if lang.init
    far.RecursiveSearch (win.GetEnv"farprofile").."\\Macros\\scripts",lang.mask,(loadonefile lang.name,lang.load m),(bit64.bor F.FRS_RECUR,F.FRS_SCANSYMLINK)
Event
  group:"ExitFAR"
  action:()->
    if erde=package.loaded['erde.init'] then erde.unload!
    package.loaders={}
    for loader in *old_loaders
      table.insert package.loaders,loader
    package.mooc_loaded=nil
