import P,R,S,V,C,Ct,Cg from lpeg
wordend=#-(R'az'+R'AZ'+R'09'+S'_')
{
  Keywords:{
    {
      (s,str,init)->
        nil
        Param=(name)->(V'Space'^0*P'`'*(Cg V'Value'^0,name)*P"'"*V'Space'^0)^-1
        cq=C Ct P{P'changequote'*V'Space'^0*P'('*V'Param1'*P','*V'Param2'*P')',Space:S' \t',Value:1-P"'",Param1:Param'param1',Param2:Param'param2'}
        res,quotes=cq\match str,init
        if res
          f=(v)->v~='' and v
          s.start=f quotes.param1
          s.end=f quotes.param2
        res
      Color:15
    }
    {P'm4_'^-1*(
      P'define'+P'defn'+P'undefine'+P'pushdef'+P'popdef'+P'indir'+P'builtin'+P'ifdef'+P'ifelse'+P'shift'+P'dumpdef'+P'traceon'+
      P'traceoff'+P'debugmode'+P'm4wrap'+P'len'+P'index'+P'regexp'+P'substr'+P'translit'+P'patsubst'+P'format'+P'incr'+P'decr'+
      P'eval'+P'syscmd'+P'esyscmd'+P'sysval'+P'maketemp'+P'errprint'+P'm4exit'
    )*wordend,Color:15}
    {'[,()]',Color:15}
    {P'm4_'^-1*(P'include'+P'sinclude'+P'divert'+P'undivert'+P'divnum')*wordend,Color:2}
    {P'm4_'^-1*(P'changecom'+P'changeword')*wordend,Color:12}
    {P'm4_'^-1*(P'dnl'+P'__file__'+P'__line__')*wordend,Color:13}
  }
  Pairs:{Color:12}
  Regions:{
    {
      Left:(s,str,init)->
        m=str\match '^'..(s.start or '`'),init
        if m then s.count=0
        m
      Right:(s,str,init)->
        local m
        if str\match '^'..(s.start or '`'),init
          s.count+=1
        else
          m=str\match '^'..(s.end or "'"),init
          if m
            if s.count>0
              s.count-=1
              m=nil
        m
      Color:14
    }
    {Left:'[%w_]+',Right:'',Pair:false}
  }
}
