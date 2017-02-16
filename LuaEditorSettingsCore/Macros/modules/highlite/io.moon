import P,S,locale from lpeg
{:alpha,:alnum,:digit,:xdigit}=locale!
digits=digit^1
word=(alpha+'_')*(alnum+'_')^0
{
  Keywords:{
    {P'0'*S'xX'*xdigit^1+digits^1+P'.'*digits^1+digits*P'.'*digits*(P'e'*P'-'^-1*digits)^-1,Color:10}
    {P'::='+P':=',Color:8}
    {S'`~@%$%%%^&%*%-%+/=\\<>%?%.,:',Color:15}
    {P';',Color:13}
    {'%(',Color:15,Open:1}
    {'%)',Color:15,Close:1}
    {'%[',Color:15,Open:2}
    {'%]',Color:15,Close:2}
    {'{',Color:15,Open:3}
    {'}',Color:15,Close:3}
    {P'block'+P'method'+P'while'+P'foreach'+P'if'+P'else'+P'do'+P'super'+P'self'+P'clone'+P'proto'+P'setSlot'+P'hasSlot'+P'type'+P'write'+P'print'+P'forward',Color:15}
    {word}
  }
  Pairs:{Color:12}
  Regions:{
    {Left:P'#'+P'//',Color:3}
    {Left:"/%*",Right:"%*/",Color:3}
    {Left:'"',Right:'"',Color:14,Keywords:{{P"\\"*(P 1),Color:6}}}
  }
}
