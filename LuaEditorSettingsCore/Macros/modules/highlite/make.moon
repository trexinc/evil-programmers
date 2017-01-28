import P,S,B from lpeg
variables={{Left:'$?${',Right:'}',Color:10},{Left:'$?$%(',Right:'%)',Color:14}}
mt_variables=__index:{Regions:variables}
syntax={
  Keywords:{
    {(P'include'+P'ifdef'+P'ifndef'+P'endif'+P'ifeq'+P'ifneq'+P'if'+P'else')*#(S' \t'+P -1),Start:true,Color:5}
    {'\t',Start:true,Color:{7,4}}
    {(P'.PHONY'+P'.NOEXPORT')*#(S' \t:'+P -1),Color:15}
    {':',Color:14}
    {'@[%S]-@',Color:{13,0}}
    {'[%w]+'}}
  Pairs:{Color:12}
  Regions:{
    unpack variables
  }
}
for r in *syntax.Regions do setmetatable r,mt_variables
table.insert syntax.Regions,{Left:'#',Start:true,Color:6,Keywords:{{'@[%S]-@',Color:{13,0}}}}
syntax
