import P,R,S,V,C,Cf,Cg,Ct from lpeg
sub,insert,get=string.sub,table.insert,editor.GetString
add=(t,k,v)->
  no='no'==sub k,1,2
  k=sub k,3 if no
  t[k]=v or not no
  t
parse=(line)->
  parser=P{
    Cf Ct''*(((1-V'Space')^1-V'Magic')*V'Space'^1)*V'Magic'*V'Space'^0*(V'Syntax'*-1+V'Syntax2'),add
    Space:S' \t'
    Delim:V'Space'+S':'
    Spaces:V'Space'^1
    Lower:R'az'
    Magic:P'vim:'+P'vi:'+P'ex:'
    Symbol:1-(V'Delim')
    Params:V'Param'*(V'Delims'*V'Param')^0
    Param:Cg V'Name'*(P'='*V'Value')^-1
    Name:C V'Lower'^1
    Value:C V'Symbol'^1
    Delims:V'Delim'^1
    Syntax:V'Params'*V'Delim'^0
    Symbol2:(1-V'Delim')+P'\\:'
    Value2:C V'Symbol2'^1
    Param2:Cg V'Name'*(P'='*V'Value2')^-1
    Params2:V'Param2'*(V'Spaces'*V'Param2')^0
    Syntax2:P'se'*P't'^-1*V'Spaces'*V'Params2'*P':'
  }
  parser\match line
(id)->
  lines,total={},(editor.GetInfo id).TotalLines
  for ii=1,total<=10 and total or 5
    insert lines,(get id,ii,3)
  if total>10
    for ii=total-4,total
      insert lines,(get id,ii,3)
  for line in *lines
    result=parse line
    if result return result
