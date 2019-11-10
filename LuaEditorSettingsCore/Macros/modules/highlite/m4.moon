import P,R,S,V,C,Ct,Cg from lpeg
word=(R'az'+R'AZ'+R'09'+S'_')^1
{
  Operators:{
    {'[,()]',Color:15}
  }
  Keywords:{
    {P'm4_'^-1*(
      P'define'+P'defn'+P'undefine'+P'pushdef'+P'popdef'+P'indir'+P'builtin'+P'ifdef'+P'ifelse'+P'shift'+P'dumpdef'+P'traceon'+
      P'traceoff'+P'debugmode'+P'm4wrap'+P'len'+P'index'+P'regexp'+P'substr'+P'translit'+P'patsubst'+P'format'+P'incr'+P'decr'+
      P'eval'+P'syscmd'+P'esyscmd'+P'sysval'+P'maketemp'+P'errprint'+P'm4exit'
    ),Color:15}
    {P'm4_'^-1*(P'include'+P'sinclude'+P'divert'+P'undivert'+P'divnum'),Color:2}
    {P'm4_'^-1*(P'changecom'+P'changeword'),Color:12}
    {P'm4_'^-1*(P'dnl'+P'__file__'+P'__line__'),Color:13}
    {
      (s,str,init)->
        nil
        Param=(name)->(V'Space'^0*P'`'*(Cg V'Value'^0,name)*P"'"*V'Space'^0)^-1
        cq=C Ct P{P'changequote'*V'Space'^0*P'('*V'Param1'*P','*V'Param2'*P')',Space:S' \t',Value:1-P"'",Param1:Param'param1',Param2:Param'param2'}
        res,quotes=cq\match str,init.B
        if res
          f=(v)->v~='' and v
          s.start=f quotes.param1
          s.end=f quotes.param2
        res
      Color:{1,0}
      Skip:true
    }
    Word:word
  }
  Pairs:{Color:12}
  Regions:{
    {
      Left:(s,str,init)->
        m=str\match '^'..(s.start or '`'),init.U
        if m then s.count=0
        m
      Right:(s,str,init)->
        local m
        if str\match '^'..(s.start or '`'),init.U
          s.count+=1
        else
          m=str\match '^'..(s.end or "'"),init.U
          if m
            if s.count>0
              s.count-=1
              m=nil
        m
      Color:14
    }
  }
}
