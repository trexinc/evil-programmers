import P,S,V from lpeg
{
  Keywords:{{"%S+%(%)",Color:13}}
  Regions:{
    {Left:"\t%*%s",Start:true,Color:7}
    {Left:"        %*%s",Start:true,Color:7}
    {Left:(P{
      'D'
      Space:S' \t'
      NonSpace:1-V'Space'
      Y:(P"19"+P"20")*(V'NonSpace'-P'-')^1*P'-'*V'NonSpace'^1*V'Space'
      W:(P"Mon"+P"Tue"+P"Wed"+P"Thu"+P"Fri"+P"Sat"+P"Sun")*V'Space'
      D:V'Y'+V'W'}),Start:true,Color:14,Keywords:{{"<%S->",Color:12}}}
  }
}
