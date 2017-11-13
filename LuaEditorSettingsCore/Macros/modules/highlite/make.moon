import P,S,B from lpeg
variables={{Left:'$?${',Right:'}',Color:10},{Left:'$?$%(',Right:'%)',Color:14}}
mt_variables=__index:{Regions:variables}
syntax={
  Operators:{
    {'\t',Start:true,Color:{7,4}}
    {':',Color:14}
    {'@[%S]-@',Color:{13,0}}
  }
  Keywords:{
    {P'include'+P'ifdef'+P'ifndef'+P'endif'+P'ifeq'+P'ifneq'+P'if'+P'else',Start:true,Color:5}
    {P'.PHONY'+P'.NOEXPORT',Color:15}
    Word:'[.%w]+'
  }
  Pairs:{Color:12}
  Regions:{
    unpack variables
  }
}
for r in *syntax.Regions do setmetatable r,mt_variables
table.insert syntax.Regions,{Left:'#',Start:true,Color:6,Keywords:{{'@[%S]-@',Color:{13,0}}}}
syntax
