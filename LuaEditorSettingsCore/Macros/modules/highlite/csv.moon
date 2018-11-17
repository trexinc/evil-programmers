import P,S from lpeg
F=far.Flags
K=far.Colors
{Flags:flag,BackgroundColor:color}=far.AdvControl F.ACTL_GETCOLOR,K.COL_EDITORTEXT
flag=bit64.band flag,F.FCF_BG_4BIT
commacolor={Flags:flag,ForegroundColor:0xff404040,BackgroundColor:color}
colors={
  {Flags:flag,ForegroundColor:0xff4040f0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff40a0f0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff40f0c0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff40f040,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xfff0c040,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xfff04040,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xfff040c0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xffd0d0d0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff2b2ba0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff2b6aa0,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff2ba080,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff2ba02b,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xffa0802b,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xffa02b2b,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xffa02b80,BackgroundColor:color}
  {Flags:flag,ForegroundColor:0xff8a8a8a,BackgroundColor:color}
}

(line,addcolor)->
  field='"'*(((P 1)-'"')+P'""')^0*'"'+(1-S',"')^0
  comma=P','
  posB,posU,ii=1,1,0
  while true
    res=field\match line,posB
    if res
      word=string.sub line,posB,res-1
      addcolor posU,posU+word\len!-1,colors[ii%#colors+1],0
      res=comma\match line,res
      if res
        posB=res
        posU+=word\len!+1
        addcolor posU-1,posU-1,commacolor,0
      else
        break
    else
      break
    ii+=1
