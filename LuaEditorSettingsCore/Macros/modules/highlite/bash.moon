import P,S,C,locale from lpeg
{:alpha,:alnum,:digit}=locale!
word=(alpha+'_')*(alnum+'_')^0
range=(s,e)->(P s)*(1-S s..e)^0*(P e)^-1
escape={P"\\"*(P 1),Color:6}
{
  Keywords:{
    {P'$'*(S'!#?*@$'+(range '(',')')+(range '[',']')+(range '{','}')+(range '`','`')+digit^1+word),Color:13}
    {P'\\'*P 1}
    {
--      (P'if'+P'then'+P'elif'+P'else'+P'fi'+P'case'+P'in'+P'esac'+P'while'+
--      P'for'+P'done'+P'do'+P'continue'+P'local'+P'return'+P'-nt'+P'-ot'+P'-ef'+
--      P'-eq'+P'-ne'+P'-lt'+P'-le'+P'-gt'+P'-ge'+P'-a'+P'-b'+P'-c'+P'-d'+P'-e'+
--      P'-f'+P'-g'+P'-h'+P'-k'+P'-p'+P'-r'+P'-s'+P'-t'+P'-u'+P'-w'+P'-x'+P'-O'+
--      P'-G'+P'-L'+P'-S'+P'-N'+P'-o'+P'-z'+P'-n')*#-word
      Color:15
    }
    {word}
  }
  Pairs:{Color:12}
  Regions:{
    {Left:'#',Color:3}
    {Left:"'",Right:"'",Color:14}
    {Left:'"',Right:'"',Color:14,Keywords:{escape}}
    {Left:'`',Right:'`',Color:14,Keywords:{escape}}
    {
      Left:(s,str,init)->
        r,s.heredoc=(C P'<<'*S'"\'\\-'^-1*(C (alpha+'_')*(alnum+'_')^0))\match str,init
        r
      Right:(s,str,init)->
        r=1==init and ((C P s.heredoc)*P -1)\match str,init
        if r then s.heredoc=nil
        r
      Color:6
    }
  }
}
